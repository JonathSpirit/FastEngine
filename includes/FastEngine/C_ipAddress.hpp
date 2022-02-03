#ifndef _FGE_C_IPADDRESS_HPP_INCLUDED_
#define _FGE_C_IPADDRESS_HPP_INCLUDED_

#include <FastEngine/fastengine_extern.hpp>
#include <string>
#include <vector>
#include <cstdint>

#define FGE_ANYPORT 0

namespace fge
{
namespace net
{

using Port = uint16_t;

class FGE_API IpAddress
{
public:
    IpAddress();
    IpAddress(const std::string& address);
    IpAddress(const char* address);
    IpAddress(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0);
    IpAddress(uint32_t address);
    ~IpAddress() = default;

    bool set(const std::string& address);
    bool set(const char* address);
    bool set(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0);
    bool set(uint32_t address);
    bool setNetworkByteOrdered(uint32_t address);

    bool operator==(const fge::net::IpAddress& r) const;

    std::string toString() const;

    uint32_t getNetworkByteOrder() const;
    uint32_t getHostByteOrder() const;

    static std::string getHostName();
    static void getLocalAddresses(std::vector<fge::net::IpAddress>& buff);

    static const fge::net::IpAddress None;
    static const fge::net::IpAddress Any;
    static const fge::net::IpAddress LocalHost;
    static const fge::net::IpAddress Broadcast;

private:
    uint32_t g_address; //Network byte order address
    bool g_valid;
};

}//end net
}//end fge

#endif // _FGE_C_IPADDRESS_HPP_INCLUDED_
