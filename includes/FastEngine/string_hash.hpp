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

#ifndef _FGE_STRING_HASH_HPP_INCLUDED
#define _FGE_STRING_HASH_HPP_INCLUDED

#include <string>

namespace fge
{

struct StringHash
{
    using is_transparent = void;

    [[nodiscard]] inline std::size_t operator()(char const* str) const { return std::hash<std::string_view>{}(str); }

    [[nodiscard]] inline std::size_t operator()(std::string_view str) const
    {
        return std::hash<std::string_view>{}(str);
    }

    [[nodiscard]] inline std::size_t operator()(std::string const& str) const { return std::hash<std::string>{}(str); }
};

} // namespace fge

#endif //_FGE_STRING_HASH_HPP_INCLUDED