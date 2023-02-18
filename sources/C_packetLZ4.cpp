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

#include "FastEngine/C_packetLZ4.hpp"
#include "FastEngine/fge_endian.hpp"
#include "lz4.h"
#include "lz4hc.h"
#include <stdexcept>

namespace fge
{
namespace net
{

///Class PacketLZ4

uint32_t PacketLZ4::_maxUncompressedReceivedSize = FGE_PACKETLZ4_DEFAULT_MAXUNCOMPRESSEDRECEIVEDSIZE;

PacketLZ4::PacketLZ4() :
        fge::net::Packet(),
        g_lastCompressionSize(0)
{}
PacketLZ4::PacketLZ4(fge::net::PacketLZ4&& pck) noexcept :
        fge::net::Packet(std::move(pck)),
        g_buffer(std::move(pck.g_buffer)),
        g_lastCompressionSize(pck.g_lastCompressionSize)
{}

std::size_t PacketLZ4::getLastCompressionSize() const
{
    return this->g_lastCompressionSize;
}

void PacketLZ4::onSend(std::vector<uint8_t>& buffer, std::size_t offset)
{
    std::size_t dataSrcSize = this->getDataSize();
    int dataDstSize = LZ4_compressBound(dataSrcSize);
    const char* dataSrc = reinterpret_cast<const char*>(this->_g_data.data());

    if (dataDstSize <= 0)
    { //input size is incorrect (too large or negative)
        throw std::invalid_argument("input size is too large or negative !");
    }

    buffer.resize(dataDstSize + sizeof(uint32_t) + offset);

    int dataCompressedSize = LZ4_compress_default(
            dataSrc, reinterpret_cast<char*>(buffer.data()) + sizeof(uint32_t) + offset, dataSrcSize, dataDstSize);
    if (dataCompressedSize <= 0)
    {
        throw std::overflow_error("no enough buffer size or compression error !");
    }

    *reinterpret_cast<uint32_t*>(buffer.data() + offset) = fge::SwapHostNetEndian_32(dataSrcSize);

    buffer.resize(dataCompressedSize + sizeof(uint32_t) + offset);
    this->g_lastCompressionSize = buffer.size();
    this->_g_lastDataValidity = true;
}

void PacketLZ4::onReceive(void* data, std::size_t dsize)
{
    if (dsize < 4)
    {
        throw std::invalid_argument("received a bad packet !");
    }

    uint32_t dataUncompressedSize = 0;
    const char* dataBuff = static_cast<const char*>(data);

    dataUncompressedSize = fge::SwapHostNetEndian_32(*reinterpret_cast<const uint32_t*>(&dataBuff[0]));

    if ((dataUncompressedSize > LZ4_MAX_INPUT_SIZE) ||
        (dataUncompressedSize > fge::net::PacketLZ4::_maxUncompressedReceivedSize))
    {
        throw std::range_error("received packet is too big !");
    }

    this->g_buffer.resize(dataUncompressedSize + 10);

    int dataUncompressedFinalSize = LZ4_decompress_safe(dataBuff + sizeof(uint32_t), this->g_buffer.data(),
                                                        dsize - sizeof(uint32_t), this->g_buffer.size());

    if (dataUncompressedFinalSize <= 0)
    {
        throw std::invalid_argument("received a bad packet !");
    }

    this->append(this->g_buffer.data(), dataUncompressedFinalSize);
}

///Class PacketLZ4HC

uint32_t PacketLZ4HC::_maxUncompressedReceivedSize = FGE_PACKETLZ4HC_DEFAULT_MAXUNCOMPRESSEDRECEIVEDSIZE;

PacketLZ4HC::PacketLZ4HC() :
        fge::net::Packet(),
        g_compressionLevel(LZ4HC_CLEVEL_DEFAULT),
        g_lastCompressionSize(0)
{}
PacketLZ4HC::PacketLZ4HC(fge::net::PacketLZ4HC&& pck) noexcept :
        fge::net::Packet(std::move(pck)),
        g_buffer(std::move(pck.g_buffer)),
        g_compressionLevel(pck.g_compressionLevel),
        g_lastCompressionSize(pck.g_lastCompressionSize)
{}

std::size_t PacketLZ4HC::getLastCompressionSize() const
{
    return this->g_lastCompressionSize;
}

void PacketLZ4HC::onSend(std::vector<uint8_t>& buffer, std::size_t offset)
{
    std::size_t dataSrcSize = this->getDataSize();
    int dataDstSize = LZ4_compressBound(dataSrcSize);
    const char* dataSrc = reinterpret_cast<const char*>(this->_g_data.data());

    if (dataDstSize <= 0)
    { //input size is incorrect (too large or negative)
        throw std::invalid_argument("input size is too large or negative !");
    }

    buffer.resize(dataDstSize + sizeof(uint32_t) + offset);

    int dataCompressedSize =
            LZ4_compress_HC(dataSrc, reinterpret_cast<char*>(buffer.data()) + sizeof(uint32_t) + offset, dataSrcSize,
                            dataDstSize, this->g_compressionLevel);
    if (dataCompressedSize <= 0)
    {
        throw std::overflow_error("no enough buffer size or compression error !");
    }

    *reinterpret_cast<uint32_t*>(buffer.data() + offset) = fge::SwapHostNetEndian_32(dataSrcSize);

    buffer.resize(dataCompressedSize + sizeof(uint32_t) + offset);
    this->g_lastCompressionSize = buffer.size();
    this->_g_lastDataValidity = true;
}

void PacketLZ4HC::onReceive(void* data, std::size_t dsize)
{
    if (dsize < 4)
    {
        throw std::invalid_argument("received a bad packet !");
    }

    uint32_t dataUncompressedSize = 0;
    const char* dataBuff = static_cast<const char*>(data);

    dataUncompressedSize = fge::SwapHostNetEndian_32(*reinterpret_cast<const uint32_t*>(&dataBuff[0]));

    if ((dataUncompressedSize > LZ4_MAX_INPUT_SIZE) ||
        (dataUncompressedSize > fge::net::PacketLZ4HC::_maxUncompressedReceivedSize))
    {
        throw std::range_error("received packet is too big !");
    }

    this->g_buffer.resize(dataUncompressedSize + 10);

    int dataUncompressedFinalSize = LZ4_decompress_safe(dataBuff + sizeof(uint32_t), this->g_buffer.data(),
                                                        dsize - sizeof(uint32_t), this->g_buffer.size());

    if (dataUncompressedFinalSize <= 0)
    {
        throw std::invalid_argument("received a bad packet !");
    }

    this->append(this->g_buffer.data(), dataUncompressedFinalSize);
}

void PacketLZ4HC::setCompressionLevel(int value)
{
    this->g_compressionLevel = value;
}
int PacketLZ4HC::getCompressionLevel() const
{
    return this->g_compressionLevel;
}

} // namespace net
} // namespace fge
