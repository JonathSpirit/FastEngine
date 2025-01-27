/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/network/C_packetLZ4.hpp"
#include "FastEngine/fge_endian.hpp"
#include "FastEngine/fge_except.hpp"
#include "lz4.h"
#include "lz4hc.h"

namespace fge::net
{

//CompressorLZ4

std::optional<Error> CompressorLZ4::compress(std::span<uint8_t const> rawData)
{
    if (rawData.empty())
    {
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "input size is zero", __func__};
    }

    auto const dataDstSize = LZ4_compressBound(static_cast<int>(rawData.size()));

    if (dataDstSize <= 0)
    { //input size is incorrect (too large or negative)
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "input size is too large, or bound error", __func__};
    }

    char const* dataSrc = reinterpret_cast<char const*>(rawData.data());

    this->_g_buffer.resize(dataDstSize + sizeof(uint32_t));

    auto const dataCompressedSize =
            LZ4_compress_default(dataSrc, reinterpret_cast<char*>(this->_g_buffer.data()) + sizeof(uint32_t),
                                 static_cast<int>(rawData.size()), dataDstSize);
    if (dataCompressedSize <= 0)
    {
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "no enough buffer size or compression error", __func__};
    }

    this->_g_buffer.resize(dataCompressedSize + sizeof(uint32_t));
    this->_g_lastCompressionSize = this->_g_buffer.size();
    *reinterpret_cast<uint32_t*>(this->_g_buffer.data()) = SwapHostNetEndian_32(rawData.size());
    return std::nullopt;
}
std::optional<Error> CompressorLZ4::uncompress(std::span<uint8_t const> data)
{
    if (data.size() < 4)
    {
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "bad date size", __func__};
    }

    uint32_t const dataUncompressedSize = SwapHostNetEndian_32(*reinterpret_cast<uint32_t const*>(data.data()));

    if (dataUncompressedSize > LZ4_MAX_INPUT_SIZE || dataUncompressedSize > this->g_maxUncompressedSize)
    {
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "data uncompressed size is too big", __func__};
    }

    char const* dataSrc = reinterpret_cast<char const*>(data.data());

    this->_g_buffer.resize(dataUncompressedSize + FGE_NET_LZ4_EXTRA_BYTES);

    auto const dataUncompressedFinalSize = LZ4_decompress_safe(
            dataSrc + sizeof(uint32_t), reinterpret_cast<char*>(this->_g_buffer.data()),
            static_cast<int>(data.size() - sizeof(uint32_t)), static_cast<int>(this->_g_buffer.size()));

    if (dataUncompressedFinalSize <= 0)
    {
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "error during uncompress", __func__};
    }

    this->_g_buffer.resize(dataUncompressedFinalSize);
    return std::nullopt;
}

void CompressorLZ4::setMaxUncompressedSize(uint32_t value)
{
    this->g_maxUncompressedSize = value;
}
uint32_t CompressorLZ4::getMaxUncompressedSize() const
{
    return this->g_maxUncompressedSize;
}

//CompressorLZ4HC

std::optional<Error> CompressorLZ4HC::compress(std::span<uint8_t const> rawData)
{
    if (rawData.empty())
    {
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "input size is zero", __func__};
    }

    auto const dataDstSize = LZ4_compressBound(static_cast<int>(rawData.size()));

    if (dataDstSize <= 0)
    { //input size is incorrect (too large or negative)
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "input size is too large, or bound error", __func__};
    }

    char const* dataSrc = reinterpret_cast<char const*>(rawData.data());

    this->_g_buffer.resize(dataDstSize + sizeof(uint32_t));

    auto const dataCompressedSize =
            LZ4_compress_HC(dataSrc, reinterpret_cast<char*>(this->_g_buffer.data()) + sizeof(uint32_t),
                            static_cast<int>(rawData.size()), dataDstSize, this->g_compressionLevel);
    if (dataCompressedSize <= 0)
    {
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "no enough buffer size or compression error", __func__};
    }

    this->_g_buffer.resize(dataCompressedSize + sizeof(uint32_t));
    this->_g_lastCompressionSize = this->_g_buffer.size();
    *reinterpret_cast<uint32_t*>(this->_g_buffer.data()) = SwapHostNetEndian_32(rawData.size());
    return std::nullopt;
}
std::optional<Error> CompressorLZ4HC::uncompress(std::span<uint8_t const> data)
{
    if (data.size() < 4)
    {
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "bad date size", __func__};
    }

    uint32_t const dataUncompressedSize = SwapHostNetEndian_32(*reinterpret_cast<uint32_t const*>(data.data()));

    if (dataUncompressedSize > LZ4_MAX_INPUT_SIZE || dataUncompressedSize > this->g_maxUncompressedSize)
    {
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "data uncompressed size is too big", __func__};
    }

    char const* dataSrc = reinterpret_cast<char const*>(data.data());

    this->_g_buffer.resize(dataUncompressedSize + FGE_NET_LZ4_EXTRA_BYTES);

    auto const dataUncompressedFinalSize = LZ4_decompress_safe(
            dataSrc + sizeof(uint32_t), reinterpret_cast<char*>(this->_g_buffer.data()),
            static_cast<int>(data.size() - sizeof(uint32_t)), static_cast<int>(this->_g_buffer.size()));

    if (dataUncompressedFinalSize <= 0)
    {
        this->_g_buffer.clear();
        return Error{Error::Types::ERR_DATA, 0, "error during uncompress", __func__};
    }

    this->_g_buffer.resize(dataUncompressedFinalSize);
    return std::nullopt;
}

void CompressorLZ4HC::setMaxUncompressedSize(uint32_t value)
{
    this->g_maxUncompressedSize = value;
}
uint32_t CompressorLZ4HC::getMaxUncompressedSize() const
{
    return this->g_maxUncompressedSize;
}

void CompressorLZ4HC::setCompressionLevel(int value)
{
    this->g_compressionLevel = std::clamp(value, LZ4HC_CLEVEL_MIN, LZ4HC_CLEVEL_MAX);
}
int CompressorLZ4HC::getCompressionLevel() const
{
    return this->g_compressionLevel;
}

//PacketLZ4

std::atomic_uint32_t PacketLZ4::gMaxUncompressedSize{FGE_NET_LZ4_DEFAULT_PACKET_MAX_SIZE};

PacketLZ4::PacketLZ4(PacketLZ4&& pck) noexcept :
        Packet(std::move(static_cast<Packet&&>(pck))),
        g_compressor(std::move(pck.g_compressor))
{}
PacketLZ4::PacketLZ4(Packet&& pck) noexcept :
        Packet(std::move(pck))
{}
PacketLZ4::PacketLZ4(PacketLZ4 const& pck) :
        Packet(pck),
        g_compressor(pck.g_compressor)
{}
PacketLZ4::PacketLZ4(Packet const& pck) :
        Packet(pck)
{}

CompressorLZ4 const& PacketLZ4::getCompressor() const
{
    return this->g_compressor;
}

void PacketLZ4::onSend(std::vector<uint8_t>& buffer, std::size_t offset)
{
    auto const err = this->g_compressor.compress(this->_g_data);
    if (err)
    {
        this->_g_lastDataValidity = false;
        throw Exception(err->_error);
    }

    buffer.resize(this->g_compressor.getLastCompressionSize() + offset);
    std::memcpy(buffer.data() + offset, this->g_compressor.getBuffer().data(),
                this->g_compressor.getLastCompressionSize());
    this->_g_lastDataValidity = true;
}

void PacketLZ4::onReceive(void* data, std::size_t dsize)
{
    this->clear();

    if (data == nullptr || dsize == 0)
    {
        this->invalidate();
        return;
    }

    this->g_compressor.setMaxUncompressedSize(gMaxUncompressedSize);
    auto const err = this->g_compressor.uncompress(std::span(static_cast<uint8_t const*>(data), dsize));
    if (err)
    {
        this->invalidate();
        throw Exception(err->_error);
    }

    this->append(this->g_compressor.getBuffer().data(), this->g_compressor.getBuffer().size());
}

//PacketLZ4HC

std::atomic_uint32_t PacketLZ4HC::gMaxUncompressedSize{FGE_NET_LZ4HC_DEFAULT_PACKET_MAX_SIZE};
std::atomic_int PacketLZ4HC::gCompressionLevel{FGE_NET_LZ4HC_DEFAULT_COMPRESSION_LEVEL};

PacketLZ4HC::PacketLZ4HC(PacketLZ4HC&& pck) noexcept :
        Packet(std::move(static_cast<Packet&&>(pck))),
        g_compressor(std::move(pck.g_compressor))
{}
PacketLZ4HC::PacketLZ4HC(Packet&& pck) noexcept :
        Packet(std::move(pck))
{}
PacketLZ4HC::PacketLZ4HC(PacketLZ4HC const& pck) :
        Packet(pck),
        g_compressor(pck.g_compressor)
{}
PacketLZ4HC::PacketLZ4HC(Packet const& pck) :
        Packet(pck)
{}

CompressorLZ4HC const& PacketLZ4HC::getCompressor() const
{
    return this->g_compressor;
}

void PacketLZ4HC::onSend(std::vector<uint8_t>& buffer, std::size_t offset)
{
    this->g_compressor.setCompressionLevel(gCompressionLevel);
    auto const err = this->g_compressor.compress(this->_g_data);
    if (err)
    {
        this->_g_lastDataValidity = false;
        throw Exception(err->_error);
    }

    buffer.resize(this->g_compressor.getLastCompressionSize() + offset);
    std::memcpy(buffer.data() + offset, this->g_compressor.getBuffer().data(),
                this->g_compressor.getLastCompressionSize());
    this->_g_lastDataValidity = true;
}

void PacketLZ4HC::onReceive(void* data, std::size_t dsize)
{
    this->clear();

    if (data == nullptr || dsize == 0)
    {
        this->invalidate();
        return;
    }

    this->g_compressor.setMaxUncompressedSize(gMaxUncompressedSize);
    auto const err = this->g_compressor.uncompress(std::span(static_cast<uint8_t const*>(data), dsize));
    if (err)
    {
        this->invalidate();
        throw Exception(err->_error);
    }

    this->append(this->g_compressor.getBuffer().data(), this->g_compressor.getBuffer().size());
}

} // namespace fge::net
