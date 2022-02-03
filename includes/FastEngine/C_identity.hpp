#ifndef _FGE_C_IDENTITY_HPP_INCLUDED
#define _FGE_C_IDENTITY_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_ipAddress.hpp>

namespace fge
{
namespace net
{

struct Identity
{
    fge::net::IpAddress _ip;
    fge::net::Port _port;

    inline bool operator==(const fge::net::Identity& right) const
    {
        return (this->_ip==right._ip) && (this->_port==right._port);
    }
};

struct IdentityHash
{
    std::size_t operator() (const fge::net::Identity& id) const
    {
        return std::hash<uint64_t>()(static_cast<uint64_t>(id._ip.getNetworkByteOrder()) | (static_cast<uint64_t>(id._port)<<32));
    }
};

}//end net
}//end fge

#endif // _FGE_C_IDENTITY_HPP_INCLUDED
