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

#ifndef _FGE_C_COMPRESSOR_HPP_INCLUDED
#define _FGE_C_COMPRESSOR_HPP_INCLUDED

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace fge
{

class Compressor
{
public:
    using ErrorString = char const*;

    Compressor() = default;
    Compressor(Compressor const& compressor) = default;
    Compressor(Compressor&& compressor) noexcept = default;
    virtual ~Compressor() = default;

    [[nodiscard]] inline std::size_t getLastCompressionSize() const { return this->_g_lastCompressionSize; }

    [[nodiscard]] virtual std::optional<ErrorString> compress(std::span<uint8_t const> rawData) = 0;
    [[nodiscard]] virtual std::optional<ErrorString> uncompress(std::span<uint8_t const> data) = 0;

    [[nodiscard]] inline std::vector<uint8_t> const& getBuffer() const { return this->_g_buffer; }

protected:
    std::vector<uint8_t> _g_buffer;
    std::size_t _g_lastCompressionSize{0};
};

} // namespace fge

#endif // _FGE_C_COMPRESSOR_HPP_INCLUDED
