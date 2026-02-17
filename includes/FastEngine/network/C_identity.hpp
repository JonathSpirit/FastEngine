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

#ifndef _FGE_C_IDENTITY_HPP_INCLUDED
#define _FGE_C_IDENTITY_HPP_INCLUDED

#include "C_ipAddress.hpp"

namespace fge::net
{

/**
 * \struct Identity
 * \ingroup network
 * \brief A class to represent a client or server identity with an IP address and a port
 */
struct Identity
{
    IpAddress _ip{};
    Port _port{FGE_ANYPORT};

    [[nodiscard]] inline bool operator==(Identity const& right) const
    {
        return (this->_ip == right._ip) && (this->_port == right._port);
    }

    [[nodiscard]] inline std::string toString() const
    {
        return this->_ip.toString().value_or("UNDEFINED") + ":" + std::to_string(this->_port);
    }
};

/**
 * \struct IdentityHash
 * \ingroup network
 * \brief A class to hash an Identity (useful for std::unordered_map or other containers)
 */
struct IdentityHash
{
    inline std::size_t operator()(Identity const& id) const
    {
        auto const h1 = std::hash<IpAddress>{}(id._ip);
        auto const h2 = std::hash<Port>{}(id._port);
        return h1 ^ (h2 << 1);
    }
};

} // namespace fge::net

#endif // _FGE_C_IDENTITY_HPP_INCLUDED
