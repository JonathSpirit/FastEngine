#include "FastEngine/C_packetBZ2.hpp"
#include "FastEngine/fge_endian.hpp"
#include <bzlib.h>
#include <stdexcept>

namespace fge
{
namespace net
{

uint32_t FGE_API PacketBZ2::_MaxUncompressedReceivedSize = FGE_PACKETBZ2_DEFAULT_MAXUNCOMPRESSEDRECEIVEDSIZE;

FGE_API PacketBZ2::PacketBZ2() : fge::net::Packet(),
    g_blockSize(FGE_PACKETBZ2_DEFAULT_BLOCKSIZE),
    g_workfactor(FGE_PACKETBZ2_DEFAULT_WORKFACTOR),
    g_lastCompressionSize(0)
{
}

void FGE_API PacketBZ2::onSend(std::vector<uint8_t>& buffer, std::size_t offset)
{
    uint32_t dataSrcSize = this->getDataSize();
    uint32_t dataDstSize = ((dataSrcSize + (dataSrcSize/100)) + 608);

    buffer.resize(dataDstSize + sizeof(uint32_t) + offset);

    int result = BZ2_bzBuffToBuffCompress( reinterpret_cast<char*>(buffer.data()) + sizeof(uint32_t) + offset,
                                          &dataDstSize,
                                          reinterpret_cast<char*>( this->_g_data.data() ),
                                          dataSrcSize,
                                          this->g_blockSize,
                                          0,
                                          this->g_workfactor );

    switch (result)
    {
        case BZ_CONFIG_ERROR:
            throw std::invalid_argument("Config error !");
            break;
        case BZ_PARAM_ERROR:
            throw std::invalid_argument("Parameter error !");
            break;
        case BZ_MEM_ERROR:
            throw std::out_of_range("No enough memory !");
            break;
        case BZ_OUTBUFF_FULL:
            throw std::out_of_range("Data > Buffer");
            break;
    }

    *reinterpret_cast<uint32_t*>(buffer.data() + offset) = fge::SwapHostNetEndian_32(dataSrcSize);

    buffer.resize(dataDstSize + sizeof(uint32_t) + offset);
    this->g_lastCompressionSize = buffer.size();
    this->_g_lastDataValidity = true;
}

void FGE_API PacketBZ2::onReceive(void* data, std::size_t dsize)
{
    if ( dsize < 4 )
    {
        throw std::invalid_argument("Received a bad packet !");
    }

    uint32_t dataUncompressedSize=0;
    char* dataBuff = static_cast<char*>(data);

    dataUncompressedSize = fge::SwapHostNetEndian_32( *reinterpret_cast<const uint32_t*>(&dataBuff[0]) );

    if (dataUncompressedSize > fge::net::PacketBZ2::_MaxUncompressedReceivedSize)
    {
        throw std::range_error("received packet is too big !");
    }

    dataUncompressedSize += 10;

    this->g_buffer.resize(dataUncompressedSize);

    int result = BZ2_bzBuffToBuffDecompress( this->g_buffer.data(),
                                             &dataUncompressedSize,
                                             dataBuff+sizeof(uint32_t),
                                             dsize-sizeof(uint32_t),
                                             0,
                                             0);

    switch (result)
    {
        case BZ_CONFIG_ERROR:
            throw std::invalid_argument("PacketBZ2 : Config error !");
            break;
        case BZ_PARAM_ERROR:
            throw std::invalid_argument("PacketBZ2 : Parameter error !");
            break;
        case BZ_MEM_ERROR:
            throw std::out_of_range("PacketBZ2 : No enough memory !");
            break;
        case BZ_OUTBUFF_FULL:
            throw std::out_of_range("PacketBZ2 : Data > Buffer");
            break;
    }

    this->append(this->g_buffer.data(), dataUncompressedSize);
}

void FGE_API PacketBZ2::setBlockSize(int blockSize)
{
    this->g_blockSize = (blockSize < 1) ? 1 : ((blockSize > 9) ? 9 : blockSize);
}
int FGE_API PacketBZ2::getBlockSize() const
{
    return this->g_blockSize;
}
void FGE_API PacketBZ2::setWorkFactor(int factor)
{
    this->g_workfactor = (factor < 0) ? 0 : ((factor > 250) ? 250 : factor);
}
int FGE_API PacketBZ2::getWorkFactor() const
{
    return this->g_workfactor;
}

std::size_t FGE_API PacketBZ2::getLastCompressionSize() const
{
    return this->g_lastCompressionSize;
}

}//end net
}//end fge
