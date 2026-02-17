/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/C_compressorBZ2.hpp"
#include "FastEngine/fge_endian.hpp"
#include "bzlib.h"
#include <algorithm>

namespace fge
{

std::optional<Compressor::ErrorString> CompressorBZ2::compress(std::span<uint8_t const> const& rawData)
{
    if (rawData.empty())
    {
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();
        return ErrorString{"input size is zero"};
    }

    uint32_t dataDstSize = ((rawData.size() + (rawData.size() / 100)) + 600);

    this->_g_buffer.resize(dataDstSize + sizeof(uint32_t));

    auto const result =
            BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(this->_g_buffer.data()) + sizeof(uint32_t), &dataDstSize,
                                     const_cast<char*>(reinterpret_cast<char const*>(rawData.data())), rawData.size(),
                                     this->g_blockSize, 0, this->g_workFactor);

    if (result != BZ_OK)
    {
        this->_g_lastCompressionSize = 0;
        this->_g_buffer.clear();

        switch (result)
        {
        case BZ_CONFIG_ERROR:
            return ErrorString{"Config error"};
        case BZ_PARAM_ERROR:
            return ErrorString{"Parameter error"};
        case BZ_MEM_ERROR:
            return ErrorString{"No enough memory"};
        case BZ_OUTBUFF_FULL:
            return ErrorString{"Data > Buffer"};
        default:
            return ErrorString{"compression error"};
        }
    }

    this->_g_buffer.resize(dataDstSize + sizeof(uint32_t));
    this->_g_lastCompressionSize = this->_g_buffer.size();
    *reinterpret_cast<uint32_t*>(this->_g_buffer.data()) = fge::SwapHostNetEndian_32(rawData.size());
    return std::nullopt;
}
std::optional<Compressor::ErrorString> CompressorBZ2::uncompress(std::span<uint8_t const> const& data)
{
    if (data.size() < 4)
    {
        this->_g_buffer.clear();
        return ErrorString{"bad data size"};
    }

    uint32_t dataUncompressedSize = SwapHostNetEndian_32(*reinterpret_cast<uint32_t const*>(data.data()));

    if (dataUncompressedSize > this->g_maxUncompressedSize)
    {
        this->_g_buffer.clear();
        return ErrorString{"data uncompressed size is too big"};
    }

    char const* dataSrc = reinterpret_cast<char const*>(data.data());

    dataUncompressedSize += FGE_COMPRESSOR_BZ2_EXTRA_BYTES;
    this->_g_buffer.resize(dataUncompressedSize);

    auto const result = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(this->_g_buffer.data()),
                                                   &dataUncompressedSize, const_cast<char*>(dataSrc + sizeof(uint32_t)),
                                                   data.size() - sizeof(uint32_t), 0, 0);

    if (result != BZ_OK)
    {
        this->_g_buffer.clear();

        switch (result)
        {
        case BZ_CONFIG_ERROR:
            return ErrorString{"Config error"};
        case BZ_PARAM_ERROR:
            return ErrorString{"Parameter error"};
        case BZ_MEM_ERROR:
            return ErrorString{"No enough memory"};
        case BZ_OUTBUFF_FULL:
            return ErrorString{"Data > Buffer"};
        default:
            return ErrorString{"decompression error"};
        }
    }

    this->_g_buffer.resize(dataUncompressedSize);
    return std::nullopt;
}

void CompressorBZ2::setMaxUncompressedSize(uint32_t value)
{
    this->g_maxUncompressedSize = value;
}

uint32_t CompressorBZ2::getMaxUncompressedSize() const
{
    return this->g_maxUncompressedSize;
}

void CompressorBZ2::setBlockSize(int blockSize)
{
    this->g_blockSize = std::clamp(blockSize, 1, 9);
}
int CompressorBZ2::getBlockSize() const
{
    return this->g_blockSize;
}

void CompressorBZ2::setWorkFactor(int factor)
{
    this->g_workFactor = std::clamp(factor, 0, 250);
}
int CompressorBZ2::getWorkFactor() const
{
    return this->g_workFactor;
}

} // namespace fge
