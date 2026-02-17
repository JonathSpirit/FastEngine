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

#ifndef _FGE_C_ERROR_HPP_INCLUDED_
#define _FGE_C_ERROR_HPP_INCLUDED_

#include "FastEngine/fge_extern.hpp"
#include <cstdint>
#include <ostream>

namespace fge::net
{

struct FGE_API Error
{
    enum class Types
    {
        ERR_ALREADY_INVALID,
        ERR_EXTRACT,
        ERR_RULE,
        ERR_TRANSMIT,
        ERR_DATA,

        ERR_SCENE_OLD_PACKET
    };

    constexpr Error(Types type) :
            _type(type)
    {}
    constexpr Error(Types type, std::size_t readPos, char const* error, char const* function) :
            _type(type),
            _readPos(readPos),
            _error(error),
            _function(function)
    {}
    constexpr Error(Types type, char const* error, char const* function) :
            _type(type),
            _error(error),
            _function(function)
    {}

    Types _type;
    std::size_t _readPos{0};
    char const* _error{nullptr};
    char const* _function{nullptr};

    void dump(std::ostream& os) const;
};

} // namespace fge::net

#endif // _FGE_C_ERROR_HPP_INCLUDED_
