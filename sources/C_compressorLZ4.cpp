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

#include "FastEngine/C_compressorLZ4.hpp"
#include "FastEngine/fge_endian.hpp"
#include "lz4.h"
#include "lz4hc.h"
#include <algorithm>

namespace fge
{

//CompressorLZ4

std::optional<CompressorLZ4::ErrorString> CompressorLZ4::compress(std::span<uint8_t const> rawData)
{
    if (rawData.empty())
    {
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return ErrorString{"input size is zero"};
    }

    auto const dataDstSize = LZ4_compressBound(static_cast<int>(rawData.size()));

    if (dataDstSize <= 0)
    { //input size is incorrect (too large or negative)
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return ErrorString{"input size is too large, or bound error"};
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
        return ErrorString{"no enough buffer size or compression error"};
    }

    this->_g_buffer.resize(dataCompressedSize + sizeof(uint32_t));
    this->_g_lastCompressionSize = this->_g_buffer.size();
    *reinterpret_cast<uint32_t*>(this->_g_buffer.data()) = SwapHostNetEndian_32(rawData.size());
    return std::nullopt;
}
std::optional<CompressorLZ4::ErrorString> CompressorLZ4::uncompress(std::span<uint8_t const> data)
{
    if (data.size() < 4)
    {
        this->_g_buffer.clear();
        return ErrorString{"bad date size"};
    }

    uint32_t const dataUncompressedSize = SwapHostNetEndian_32(*reinterpret_cast<uint32_t const*>(data.data()));

    if (dataUncompressedSize > LZ4_MAX_INPUT_SIZE || dataUncompressedSize > this->g_maxUncompressedSize)
    {
        this->_g_buffer.clear();
        return ErrorString{"data uncompressed size is too big"};
    }

    char const* dataSrc = reinterpret_cast<char const*>(data.data());

    this->_g_buffer.resize(dataUncompressedSize + FGE_COMPRESSOR_LZ4_EXTRA_BYTES);

    auto const dataUncompressedFinalSize = LZ4_decompress_safe(
            dataSrc + sizeof(uint32_t), reinterpret_cast<char*>(this->_g_buffer.data()),
            static_cast<int>(data.size() - sizeof(uint32_t)), static_cast<int>(this->_g_buffer.size()));

    if (dataUncompressedFinalSize <= 0)
    {
        this->_g_buffer.clear();
        return ErrorString{"error during uncompress"};
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

std::optional<CompressorLZ4HC::ErrorString> CompressorLZ4HC::compress(std::span<uint8_t const> rawData)
{
    if (rawData.empty())
    {
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return ErrorString{"input size is zero"};
    }

    auto const dataDstSize = LZ4_compressBound(static_cast<int>(rawData.size()));

    if (dataDstSize <= 0)
    { //input size is incorrect (too large or negative)
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return ErrorString{"input size is too large, or bound error"};
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
        return ErrorString{"no enough buffer size or compression error"};
    }

    this->_g_buffer.resize(dataCompressedSize + sizeof(uint32_t));
    this->_g_lastCompressionSize = this->_g_buffer.size();
    *reinterpret_cast<uint32_t*>(this->_g_buffer.data()) = SwapHostNetEndian_32(rawData.size());
    return std::nullopt;
}
std::optional<CompressorLZ4HC::ErrorString> CompressorLZ4HC::uncompress(std::span<uint8_t const> data)
{
    if (data.size() < 4)
    {
        this->_g_buffer.clear();
        return ErrorString{"bad data size"};
    }

    uint32_t const dataUncompressedSize = SwapHostNetEndian_32(*reinterpret_cast<uint32_t const*>(data.data()));

    if (dataUncompressedSize > LZ4_MAX_INPUT_SIZE || dataUncompressedSize > this->g_maxUncompressedSize)
    {
        this->_g_buffer.clear();
        return ErrorString{"data uncompressed size is too big"};
    }

    char const* dataSrc = reinterpret_cast<char const*>(data.data());

    this->_g_buffer.resize(dataUncompressedSize + FGE_COMPRESSOR_LZ4_EXTRA_BYTES);

    auto const dataUncompressedFinalSize = LZ4_decompress_safe(
            dataSrc + sizeof(uint32_t), reinterpret_cast<char*>(this->_g_buffer.data()),
            static_cast<int>(data.size() - sizeof(uint32_t)), static_cast<int>(this->_g_buffer.size()));

    if (dataUncompressedFinalSize <= 0)
    {
        this->_g_buffer.clear();
        return ErrorString{"error during uncompress"};
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

} // namespace fge
