/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_ipAddress.hpp"

namespace fge::net
{

/**
 * \struct Identity
 * \ingroup network
 * \brief A class to represent a client or server identity with an IP address and a port
 */
struct Identity
{
    fge::net::IpAddress _ip;
    fge::net::Port _port;

    inline bool operator==(const fge::net::Identity& right) const
    {
        return (this->_ip == right._ip) && (this->_port == right._port);
    }
};

/**
 * \struct IdentityHash
 * \ingroup network
 * \brief A class to hash an Identity (useful for std::unordered_map or other containers)
 */
struct IdentityHash
{
    inline std::size_t operator()(const fge::net::Identity& id) const
    {
        return std::hash<uint64_t>()(static_cast<uint64_t>(id._ip.getNetworkByteOrder()) |
                                     (static_cast<uint64_t>(id._port) << 32));
    }
};

} // namespace fge::net

#endif // _FGE_C_IDENTITY_HPP_INCLUDED
