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

#ifndef _FGE_C_COMPRESSORBZ2_HPP_INCLUDED
#define _FGE_C_COMPRESSORBZ2_HPP_INCLUDED

#include "fge_extern.hpp"
#include "FastEngine/C_compressor.hpp"
#include <limits>

/*
 * This file is using the library :
 * libbzip2
 * copyright (C) 1996-2019 Julian R Seward.
 */

#define FGE_COMPRESSOR_BZ2_DEFAULT_WORKFACTOR 0
#define FGE_COMPRESSOR_BZ2_DEFAULT_BLOCKSIZE 4
#define FGE_COMPRESSOR_BZ2_EXTRA_BYTES 10
#define FGE_COMPRESSOR_BZ2_DEFAULT_MAX_SIZE std::numeric_limits<uint16_t>::max()
#define FGE_COMPRESSOR_BZ2_VERSION "1.1.0"

namespace fge
{

class FGE_API CompressorBZ2 : public Compressor
{
public:
    using Compressor::Compressor;

    [[nodiscard]] std::optional<ErrorString> compress(std::span<uint8_t const> const& rawData) override;
    [[nodiscard]] std::optional<ErrorString> uncompress(std::span<uint8_t const> const& data) override;

    void setMaxUncompressedSize(uint32_t value);
    [[nodiscard]] uint32_t getMaxUncompressedSize() const;

    void setBlockSize(int blockSize);
    [[nodiscard]] int getBlockSize() const;
    void setWorkFactor(int factor);
    [[nodiscard]] int getWorkFactor() const;

private:
    uint32_t g_maxUncompressedSize{FGE_COMPRESSOR_BZ2_DEFAULT_MAX_SIZE};
    int g_blockSize{FGE_COMPRESSOR_BZ2_DEFAULT_BLOCKSIZE};
    int g_workFactor{FGE_COMPRESSOR_BZ2_DEFAULT_WORKFACTOR};
};

} // namespace fge

#endif // _FGE_C_COMPRESSORBZ2_HPP_INCLUDED
