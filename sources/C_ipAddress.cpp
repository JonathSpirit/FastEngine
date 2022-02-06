#include "FastEngine/C_ipAddress.hpp"
#include "FastEngine/fge_endian.hpp"

#ifdef _WIN32
    #ifdef _WIN32_WINDOWS
        #undef _WIN32_WINDOWS
    #endif // _WIN32_WINDOWS
    #ifdef WINVER
        #undef WINVER
    #endif // WINVER
    #ifdef _WIN32_WINNT
        #undef _WIN32_WINNT
    #endif // _WIN32_WINNT

    #define _WIN32_WINDOWS  _WIN32_WINNT_WINXP
    #define WINVER          _WIN32_WINNT_WINXP
    #define _WIN32_WINNT    _WIN32_WINNT_WINXP

    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <iphlpapi.h>
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netdb.h>
    #include <unistd.h>
#endif // _WIN32

#ifdef _WIN32
    #define _FGE_SOCKET_INVALID INVALID_SOCKET
    #define _FGE_SOCKET_ERROR SOCKET_ERROR
#else
    #define _FGE_SOCKET_INVALID -1
    #define _FGE_SOCKET_ERROR -1
#endif

namespace fge
{
namespace net
{

const fge::net::IpAddress IpAddress::None;
const fge::net::IpAddress IpAddress::Any(0, 0, 0, 0);
const fge::net::IpAddress IpAddress::LocalHost(127, 0, 0, 1);
const fge::net::IpAddress IpAddress::Broadcast(255, 255, 255, 255);

FGE_API IpAddress::IpAddress() :
    g_address(0),
    g_valid(false)
{
}
FGE_API IpAddress::IpAddress(const std::string& address) :
    g_address(0),
    g_valid(false)
{
    this->set(address);
}
FGE_API IpAddress::IpAddress(const char* address) :
    g_address(0),
    g_valid(false)
{
    this->set(std::string(address));
}
FGE_API IpAddress::IpAddress(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0) :
    g_address( fge::SwapHostNetEndian_32((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0) ),
    g_valid(true)
{
}
FGE_API IpAddress::IpAddress(uint32_t address) :
    g_address( fge::SwapHostNetEndian_32(address) ),
    g_valid(true)
{
}

bool FGE_API IpAddress::set(const std::string& address)
{
    if (address == "255.255.255.255")
    {//Broadcast
        this->g_address = INADDR_BROADCAST;
        this->g_valid = true;
        return true;
    }
    else if (address == "0.0.0.0")
    {//Any
        this->g_address = INADDR_ANY;
        this->g_valid = true;
        return true;
    }
    else
    {//IP as string xxx.xxx.xxx.xxx
        uint32_t ip = inet_addr(address.c_str());

        if (ip != INADDR_NONE)
        {
            this->g_address = ip;
            this->g_valid = true;
            return true;
        }
        else
        {//Maybe host name
            addrinfo hints{};
            hints.ai_family = AF_INET;
            addrinfo* result = nullptr;

            if ( getaddrinfo(address.c_str(), nullptr, &hints, &result) == 0 )
            {
                if (result)
                {
                    ip = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
                    freeaddrinfo(result);

                    this->g_address = ip;
                    this->g_valid = true;
                    return true;
                }
            }
        }
    }

    this->g_address = 0;
    this->g_valid = false;
    return false;
}
bool FGE_API IpAddress::set(const char* address)
{
    return this->set(std::string(address));
}
bool FGE_API IpAddress::set(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0)
{
    this->g_address = fge::SwapHostNetEndian_32((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
    this->g_valid = true;
    return true;
}
bool FGE_API IpAddress::set(uint32_t address)
{
    this->g_address = fge::SwapHostNetEndian_32(address);
    this->g_valid = true;
    return true;
}
bool FGE_API IpAddress::setNetworkByteOrdered(uint32_t address)
{
    this->g_address = address;
    this->g_valid = true;
    return true;
}

bool FGE_API IpAddress::operator==(const fge::net::IpAddress& r) const
{
    return (this->g_address == r.g_address) && (this->g_valid == r.g_valid);
}

std::string FGE_API IpAddress::toString() const
{
    in_addr address{};
    address.s_addr = this->g_address;

    return {inet_ntoa(address)};
}

uint32_t FGE_API IpAddress::getNetworkByteOrder() const
{
    return this->g_address;
}
uint32_t FGE_API IpAddress::getHostByteOrder() const
{
    return fge::SwapHostNetEndian_32(this->g_address);
}

std::string FGE_API IpAddress::getHostName()
{
    char name[80];
    if ( gethostname(name, sizeof(name)) == _FGE_SOCKET_ERROR )
    {
        return {};
    }
    return {name};
}

void FGE_API IpAddress::getLocalAddresses(std::vector<fge::net::IpAddress>& buff)
{
    buff.clear();

    addrinfo hints{};
    hints.ai_family = AF_INET;
    addrinfo* result = nullptr;

    if ( getaddrinfo("", nullptr, &hints, &result) == 0 )
    {
        if (result)
        {
            addrinfo *ptr = result;

            do
            {
                fge::net::IpAddress ip;
                ip.g_address = reinterpret_cast<sockaddr_in*>(ptr->ai_addr)->sin_addr.s_addr;
                ip.g_valid = true;

                buff.push_back(ip);

                ptr=ptr->ai_next;
            }
            while (ptr != nullptr);

            freeaddrinfo(result);
        }
    }
}

}//end net
}//end fge
