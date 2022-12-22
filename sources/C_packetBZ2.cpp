/*
 * Copyright 2022 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FastEngine/C_packetBZ2.hpp"
#include "FastEngine/fge_endian.hpp"
#include <bzlib.h>
#include <stdexcept>

namespace fge
{
namespace net
{

uint32_t PacketBZ2::_maxUncompressedReceivedSize = FGE_PACKETBZ2_DEFAULT_MAXUNCOMPRESSEDRECEIVEDSIZE;

PacketBZ2::PacketBZ2() :
        fge::net::Packet(),
        g_blockSize(FGE_PACKETBZ2_DEFAULT_BLOCKSIZE),
        g_workfactor(FGE_PACKETBZ2_DEFAULT_WORKFACTOR),
        g_lastCompressionSize(0)
{}
PacketBZ2::PacketBZ2(fge::net::PacketBZ2&& pck) noexcept :
        fge::net::Packet(std::move(pck)),
        g_blockSize(pck.g_blockSize),
        g_workfactor(pck.g_workfactor),
        g_buffer(std::move(pck.g_buffer)),
        g_lastCompressionSize(pck.g_lastCompressionSize)
{}

void PacketBZ2::onSend(std::vector<uint8_t>& buffer, std::size_t offset)
{
    uint32_t dataSrcSize = this->getDataSize();
    uint32_t dataDstSize = ((dataSrcSize + (dataSrcSize / 100)) + 608);

    buffer.resize(dataDstSize + sizeof(uint32_t) + offset);

    int result = BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(buffer.data()) + sizeof(uint32_t) + offset,
                                          &dataDstSize, reinterpret_cast<char*>(this->_g_data.data()), dataSrcSize,
                                          this->g_blockSize, 0, this->g_workfactor);

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

void PacketBZ2::onReceive(void* data, std::size_t dsize)
{
    if (dsize < 4)
    {
        throw std::invalid_argument("Received a bad packet !");
    }

    uint32_t dataUncompressedSize = 0;
    char* dataBuff = static_cast<char*>(data);

    dataUncompressedSize = fge::SwapHostNetEndian_32(*reinterpret_cast<const uint32_t*>(&dataBuff[0]));

    if (dataUncompressedSize > fge::net::PacketBZ2::_maxUncompressedReceivedSize)
    {
        throw std::range_error("received packet is too big !");
    }

    dataUncompressedSize += 10;

    this->g_buffer.resize(dataUncompressedSize);

    int result = BZ2_bzBuffToBuffDecompress(this->g_buffer.data(), &dataUncompressedSize, dataBuff + sizeof(uint32_t),
                                            dsize - sizeof(uint32_t), 0, 0);

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

void PacketBZ2::setBlockSize(int blockSize)
{
    this->g_blockSize = (blockSize < 1) ? 1 : ((blockSize > 9) ? 9 : blockSize);
}
int PacketBZ2::getBlockSize() const
{
    return this->g_blockSize;
}
void PacketBZ2::setWorkFactor(int factor)
{
    this->g_workfactor = (factor < 0) ? 0 : ((factor > 250) ? 250 : factor);
}
int PacketBZ2::getWorkFactor() const
{
    return this->g_workfactor;
}

std::size_t PacketBZ2::getLastCompressionSize() const
{
    return this->g_lastCompressionSize;
}

} // namespace net
} // namespace fge
