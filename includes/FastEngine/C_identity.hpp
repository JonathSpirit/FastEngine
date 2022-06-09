#ifndef _FGE_C_IDENTITY_HPP_INCLUDED
#define _FGE_C_IDENTITY_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_ipAddress.hpp>

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
        return (this->_ip==right._ip) && (this->_port==right._port);
    }
};

/**
 * \struct IdentityHash
 * \ingroup network
 * \brief A class to hash an Identity (useful for std::unordered_map or other containers)
 */
struct IdentityHash
{
    inline std::size_t operator() (const fge::net::Identity& id) const
    {
        return std::hash<uint64_t>()(static_cast<uint64_t>(id._ip.getNetworkByteOrder()) | (static_cast<uint64_t>(id._port)<<32));
    }
};

}//end fge::net

#endif // _FGE_C_IDENTITY_HPP_INCLUDED
