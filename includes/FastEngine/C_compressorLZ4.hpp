/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_C_COMPRESSORLZ4_HPP_INCLUDED
#define _FGE_C_COMPRESSORLZ4_HPP_INCLUDED

#include "fge_extern.hpp"
#include "FastEngine/C_compressor.hpp"
#include <limits>

/*
 * This file is using the library :
 * LZ4 - Fast LZ compression algorithm
 * Copyright (C) 2011-present, Yann Collet.
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 */

#define FGE_COMPRESSOR_LZ4_EXTRA_BYTES 10
#define FGE_COMPRESSOR_LZ4_DEFAULT_MAX_SIZE std::numeric_limits<uint16_t>::max()
#define FGE_COMPRESSOR_LZ4HC_DEFAULT_MAX_SIZE std::numeric_limits<uint16_t>::max()
#define FGE_COMPRESSOR_LZ4HC_DEFAULT_COMPRESSION_LEVEL 9
#define FGE_COMPRESSOR_LZ4_VERSION "1.10.0"

namespace fge
{

class FGE_API CompressorLZ4 : public Compressor
{
public:
    using Compressor::Compressor;

    [[nodiscard]] std::optional<ErrorString> compress(std::span<uint8_t const> const& rawData) override;
    [[nodiscard]] std::optional<ErrorString> uncompress(std::span<uint8_t const> const& data) override;

    void setMaxUncompressedSize(uint32_t value);
    [[nodiscard]] uint32_t getMaxUncompressedSize() const;

private:
    uint32_t g_maxUncompressedSize{FGE_COMPRESSOR_LZ4_DEFAULT_MAX_SIZE};
};

class FGE_API CompressorLZ4HC : public Compressor
{
public:
    using Compressor::Compressor;

    [[nodiscard]] std::optional<ErrorString> compress(std::span<uint8_t const> const& rawData) override;
    [[nodiscard]] std::optional<ErrorString> uncompress(std::span<uint8_t const> const& data) override;

    void setMaxUncompressedSize(uint32_t value);
    [[nodiscard]] uint32_t getMaxUncompressedSize() const;

    void setCompressionLevel(int value);
    [[nodiscard]] int getCompressionLevel() const;

private:
    uint32_t g_maxUncompressedSize{FGE_COMPRESSOR_LZ4HC_DEFAULT_MAX_SIZE};
    int g_compressionLevel{FGE_COMPRESSOR_LZ4HC_DEFAULT_COMPRESSION_LEVEL};
};

} // namespace fge

#endif // _FGE_C_COMPRESSORLZ4_HPP_INCLUDED
