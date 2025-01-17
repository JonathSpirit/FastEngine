/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/network/C_socket.hpp"
#include "FastEngine/fge_endian.hpp"
#include "FastEngine/network/C_packet.hpp"

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

    #define _WIN32_WINDOWS _WIN32_WINNT_VISTA
    #define WINVER _WIN32_WINNT_VISTA
    #define _WIN32_WINNT _WIN32_WINNT_VISTA

    #include <winsock2.h>
    #include <iphlpapi.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <netinet/tcp.h>
    #include <sys/socket.h>
    #include <unistd.h>

    #include <ifaddrs.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
#endif // _WIN32

#ifdef _WIN32
    #define _FGE_SOCKET_INVALID INVALID_SOCKET
    #define _FGE_SOCKET_ERROR SOCKET_ERROR
    #define _FGE_SEND_RECV_FLAG 0
#else
    #define _FGE_SOCKET_INVALID -1
    #define _FGE_SOCKET_ERROR -1
    #define _FGE_SEND_RECV_FLAG MSG_NOSIGNAL
#endif

#if defined(__APPLE__) && defined(__MACH__)
    // Apple platform
    #include "TargetConditionals.h"

    #if TARGET_OS_MAC
        #define _FGE_MACOS
    #endif
#endif

#ifndef _WIN32
    #undef None
#endif

namespace fge::net
{

#ifdef _WIN32
using SocketLength = int;
#else
using SocketLength = socklen_t;
#endif //_WIN32

/*
Can be found on windows :
    WSANOTINITIALISED
    ENETDOWN
    ENETRESET
    ESHUTDOWN
    EHOSTUNREACH
    ECONNABORTED
    EINVALIDPROVIDER
    EINVALIDPROCTABLE
    EPROVIDERFAILEDINIT
    ESOCKTNOSUPPORT

Can be found on unix :
    ENOMEM
    EPIPE
    EBADF
    EAGAIN
    ELOOP
    ENAMETOOLONG
    ENOENT
    ENOTDIR
    EROFS
    EDOM
    ENFILE
    EPERM

Can be found on unix and windows :
    EACCES
    EWOULDBLOCK
    ECONNRESET
    EDESTADDRREQ
    EFAULT
    EINTR
    EINVAL
    EMSGSIZE
    ENOBUFS
    ENOTCONN
    ENOTSOCK
    EOPNOTSUPP
    EISCONN
    EADDRNOTAVAIL
    EADDRINUSE
    ENOPROTOOPT
    EAFNOSUPPORT
    EMFILE
    EPROTONOSUPPORT
    EINPROGRESS
    ENETUNREACH
    EPROTOTYPE
    ETIMEDOUT
    EALREADY
    ECONNREFUSED
*/

namespace
{

Socket::Errors NormalizeError(int err)
{
#ifdef _WIN32
    switch (err)
    {
    case WSANOTINITIALISED:
        return Socket::Errors::ERR_NOTINIT;

    case WSAEWOULDBLOCK:
        return Socket::Errors::ERR_NOTREADY;
    case WSAEALREADY:
        return Socket::Errors::ERR_NOTREADY;
    case WSAEINPROGRESS:
        return Socket::Errors::ERR_NOTREADY;

    case WSAETIMEDOUT:
        return Socket::Errors::ERR_DISCONNECTED;
    case WSAECONNABORTED:
        return Socket::Errors::ERR_DISCONNECTED;
    case WSAECONNRESET:
        return Socket::Errors::ERR_DISCONNECTED;
    case WSAENETRESET:
        return Socket::Errors::ERR_DISCONNECTED;
    case WSAENOTCONN:
        return Socket::Errors::ERR_DISCONNECTED;
    case WSAENETUNREACH:
        return Socket::Errors::ERR_DISCONNECTED;

    case WSAECONNREFUSED:
        return Socket::Errors::ERR_REFUSED;

    case WSAEADDRINUSE:
        return Socket::Errors::ERR_ALREADYUSED;
    case WSAEISCONN:
        return Socket::Errors::ERR_ALREADYCONNECTED;

    case WSAEMFILE:
        return Socket::Errors::ERR_TOOMANYSOCKET;

    default:
        return Socket::Errors::ERR_UNSUCCESS;
    }
#else
    if ((err == EAGAIN) || (err == EINPROGRESS))
    {
        return Socket::Errors::ERR_NOTREADY;
    }

    switch (err)
    {
    case EWOULDBLOCK:
        return Socket::Errors::ERR_NOTREADY;
    case EALREADY:
        return Socket::Errors::ERR_NOTREADY;
    case EINPROGRESS:
        return Socket::Errors::ERR_NOTREADY;

    case ETIMEDOUT:
        return Socket::Errors::ERR_DISCONNECTED;
    case ECONNABORTED:
        return Socket::Errors::ERR_DISCONNECTED;
    case ECONNRESET:
        return Socket::Errors::ERR_DISCONNECTED;
    case ENETRESET:
        return Socket::Errors::ERR_DISCONNECTED;
    case ENOTCONN:
        return Socket::Errors::ERR_DISCONNECTED;
    case ENETUNREACH:
        return Socket::Errors::ERR_DISCONNECTED;
    case EPIPE:
        return Socket::Errors::ERR_DISCONNECTED;

    case ECONNREFUSED:
        return Socket::Errors::ERR_REFUSED;

    case EADDRINUSE:
        return Socket::Errors::ERR_ALREADYUSED;
    case EISCONN:
        return Socket::Errors::ERR_ALREADYCONNECTED;

    case EMFILE:
        return Socket::Errors::ERR_TOOMANYSOCKET;

    default:
        return Socket::Errors::ERR_UNSUCCESS;
    }
#endif // _WIN32
}
Socket::Errors NormalizeError()
{
#ifdef _WIN32
    return NormalizeError(WSAGetLastError());
#else
    return NormalizeError(errno);
#endif // _WIN32
}

sockaddr* CreateAddress(sockaddr_in& addr4, sockaddr_in6& addr6, int& size, IpAddress const& address, Port port)
{
    sockaddr* addr = nullptr;
    size = 0;
    if (address.getType() == IpAddress::Types::Ipv4)
    {
        addr4.sin_family = AF_INET;
        addr4.sin_addr.s_addr = std::get<IpAddress::Ipv4Data>(address.getNetworkByteOrder().value());
        addr4.sin_port = fge::SwapHostNetEndian_16(port);
        addr = reinterpret_cast<sockaddr*>(&addr4);
        size = sizeof(sockaddr_in);
#ifdef _FGE_MACOS
        addr4.sin_len = size;
#endif
    }
    else
    {
        addr6.sin6_family = AF_INET6;
        auto data = std::get<IpAddress::Ipv6Data>(address.getNetworkByteOrder().value());
        std::memcpy(&addr6.sin6_addr.s6_addr, data.data(), data.size() * 2);
        addr6.sin6_port = fge::SwapHostNetEndian_16(port);
        addr = reinterpret_cast<sockaddr*>(&addr6);
        size = sizeof(sockaddr_in6);
#ifdef _FGE_MACOS
        addr6.sin6_len = size;
#endif
    }

    return addr;
}

} // namespace

///Socket

Socket::Socket(Types type, IpAddress::Types addressType) :
        g_type(type),
        g_addressType(addressType),
        g_socket(_FGE_SOCKET_INVALID)
{}
Socket::~Socket()
{
    this->close();
}

void Socket::setAddressType(IpAddress::Types type)
{
    if (this->g_addressType == type || type == IpAddress::Types::None)
    {
        return;
    }

    this->g_addressType = type;
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        this->close();
        this->create();
    }
}

void Socket::close()
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
#ifdef _WIN32
        closesocket(this->g_socket);
#else
        ::close(this->g_socket);
#endif //_WIN32
        this->g_socket = _FGE_SOCKET_INVALID;
    }
}

bool Socket::isValid() const
{
    return this->g_socket != _FGE_SOCKET_INVALID;
}

Port Socket::getLocalPort() const
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        sockaddr_in address{};
        SocketLength addressSize = sizeof(address);
        if (getsockname(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
        {
            return fge::SwapHostNetEndian_16(address.sin_port);
        }
    }
    return 0;
}
IpAddress Socket::getLocalAddress() const
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        if (this->g_addressType == IpAddress::Types::Ipv4)
        {
            sockaddr_in address{};
            SocketLength addressSize = sizeof(address);
            if (getsockname(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
            {
                return fge::SwapHostNetEndian_32(address.sin_addr.s_addr);
            }
        }
        else
        {
            sockaddr_in6 address{};
            SocketLength addressSize = sizeof(address);
            if (getsockname(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
            {
                IpAddress ip;
                ip.setNetworkByteOrdered(address.sin6_addr.s6_addr);
                return ip;
            }
        }
    }
    return IpAddress::None;
}
Port Socket::getRemotePort() const
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        sockaddr_in address{};
        SocketLength addressSize = sizeof(address);
        if (getpeername(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
        {
            return fge::SwapHostNetEndian_16(address.sin_port);
        }
    }
    return 0;
}
IpAddress Socket::getRemoteAddress() const
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        if (this->g_addressType == IpAddress::Types::Ipv4)
        {
            sockaddr_in address{};
            SocketLength addressSize = sizeof(address);
            if (getpeername(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
            {
                return fge::SwapHostNetEndian_32(address.sin_addr.s_addr);
            }
        }
        else
        {
            sockaddr_in6 address{};
            SocketLength addressSize = sizeof(address);
            if (getpeername(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
            {
                IpAddress ip;
                ip.setNetworkByteOrdered(address.sin6_addr.s6_addr);
                return ip;
            }
        }
    }
    return IpAddress::None;
}

bool Socket::isBlocking() const
{
    return this->g_isBlocking;
}

Socket::Errors Socket::setBlocking(bool mode)
{
#ifdef _WIN32
    unsigned long iMode = mode ? 0 : 1;
    if (ioctlsocket(this->g_socket, FIONBIO, &iMode) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }
    this->g_isBlocking = mode;
    return Errors::ERR_NOERROR;
#else
    int status = fcntl(this->g_socket, F_GETFL);
    if (mode)
    {
        if (fcntl(this->g_socket, F_SETFL, status & ~O_NONBLOCK) == _FGE_SOCKET_ERROR)
        {
            return NormalizeError();
        }
    }
    else
    {
        if (fcntl(this->g_socket, F_SETFL, status | O_NONBLOCK) == _FGE_SOCKET_ERROR)
        {
            return NormalizeError();
        }
    }
    this->g_isBlocking = mode;
    return Errors::ERR_NOERROR;
#endif //_WIN32
}
Socket::Errors Socket::setReuseAddress(bool mode)
{
    char const optval = mode ? 1 : 0;
    if (setsockopt(this->g_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }
    return Errors::ERR_NOERROR;
}
Socket::Errors Socket::setBroadcastOption(bool mode)
{
    char const optval = mode ? 1 : 0;
    if (setsockopt(this->g_socket, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }
    return Errors::ERR_NOERROR;
}
Socket::Errors Socket::setIpv6Only(bool mode)
{
    char const optval = mode ? 1 : 0;
    if (setsockopt(this->g_socket, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }
    return Errors::ERR_NOERROR;
}
Socket::Errors Socket::setDontFragment(bool mode)
{
#ifdef _WIN32
    char const optval = mode ? 1 : 0;
    if (setsockopt(this->g_socket, IPPROTO_IP, IP_DONTFRAGMENT, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }
    return Errors::ERR_NOERROR;
    #elseifdef _FGE_MACOS
    int const optval = mode ? 1 : 0;
    if (setsockopt(this->g_socket, IPPROTO_IP, IP_DF, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }
    return Errors::ERR_NOERROR;
#else
    char const optval = mode ? IP_PMTUDISC_DO : IP_PMTUDISC_DONT;
    if (setsockopt(this->g_socket, IPPROTO_IP, IP_MTU_DISCOVER, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }
    return Errors::ERR_NOERROR;
#endif // _WIN32
}

Socket::Errors Socket::select(bool read, uint32_t timeoutms)
{
    // Setup the selector
    fd_set selector;
#ifdef _WIN32
    selector.fd_count = 1;
    *selector.fd_array = this->g_socket;
#else
    FD_ZERO(&selector);
    FD_SET(this->g_socket, &selector);
#endif // _WIN32

    // Setup the timeout
    timeval time{};
    time.tv_sec = static_cast<long>(timeoutms / 1000);
    time.tv_usec = static_cast<long>((timeoutms % 1000) * 1000);

    // Wait for something
    if (::select(static_cast<int>(this->g_socket + 1), (read ? &selector : nullptr), (!read ? &selector : nullptr),
                 nullptr, &time) == 1)
    {
        // Socket selected for write, Check for errors
        int32_t optval = 0;
        SocketLength optlen = sizeof(optval);

        if (getsockopt(this->g_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen) ==
            _FGE_SOCKET_ERROR)
        {
            return NormalizeError();
        }
        // Check the value returned...
        if (optval != 0)
        {
            return NormalizeError(optval);
        }
        return Errors::ERR_NOERROR;
    }
    // Failed to connect before timeout is over
    return NormalizeError();
}

bool Socket::initSocket()
{
#ifdef _WIN32
    WSAData wsaData{};
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true;
#endif // _WIN32
}
void Socket::uninitSocket()
{
#ifdef _WIN32
    WSACleanup();
#endif // _WIN32
}

std::optional<uint16_t> Socket::retrieveCurrentAdapterMTU() const
{
#ifdef _WIN32
    ULONG bufferSize = 0;

    auto const family = this->g_addressType == IpAddress::Types::Ipv4 ? AF_INET : AF_INET6;
    GetAdaptersAddresses(family, 0, nullptr, nullptr, &bufferSize);

    std::unique_ptr<PIP_ADAPTER_ADDRESSES[]> const adapterAddresses =
            std::make_unique<PIP_ADAPTER_ADDRESSES[]>(bufferSize);
    PIP_ADAPTER_ADDRESSES adapterAddress = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapterAddresses.get());

    auto const currentAddress = this->getLocalAddress();

    if (GetAdaptersAddresses(family, 0, nullptr, adapterAddress, &bufferSize) == NO_ERROR)
    {
        for (PIP_ADAPTER_ADDRESSES adapter = adapterAddress; adapter != nullptr; adapter = adapter->Next)
        {
            IpAddress ip;
            if (family == AF_INET)
            {
                sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(adapter->FirstUnicastAddress->Address.lpSockaddr);
                ip.setNetworkByteOrdered(addr4->sin_addr.s_addr);
            }
            else
            {
                sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(adapter->FirstUnicastAddress->Address.lpSockaddr);
                IpAddress::Ipv6Data data;
                std::memcpy(data.data(), addr6->sin6_addr.s6_addr, 16);
                ip.setNetworkByteOrdered(data);
            }

            if (ip == currentAddress)
            {
                return std::clamp<uint32_t>(adapter->Mtu,
                                            family == AF_INET ? FGE_SOCKET_IPV4_MIN_MTU : FGE_SOCKET_IPV6_MIN_MTU,
                                            FGE_SOCKET_FULL_DATAGRAM_SIZE);
            }
        }
    }

    return std::nullopt;
#else
    auto const currentAddress = this->getLocalAddress();
    std::string interfaceName;

    // Get the interface name associated with the current address
    ifaddrs* ifaddr{nullptr};
    if (getifaddrs(&ifaddr) != 0 || ifaddr == nullptr)
    {
        return std::nullopt;
    }

    auto const family = this->g_addressType == IpAddress::Types::Ipv4 ? AF_INET : AF_INET6;

    for (ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
        {
            continue;
        }

        if (family == ifa->ifa_addr->sa_family)
        {
            IpAddress ip;
            if (family == AF_INET)
            {
                sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
                ip.setNetworkByteOrdered(addr4->sin_addr.s_addr);
                if (currentAddress == ip)
                {
                    interfaceName = ifa->ifa_name;
                    break;
                }
            }
            else
            {
                sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(ifa->ifa_addr);
                IpAddress::Ipv6Data data;
                std::memcpy(data.data(), addr6->sin6_addr.s6_addr, 16);
                ip.setNetworkByteOrdered(data);
                if (currentAddress == ip)
                {
                    interfaceName = ifa->ifa_name;
                    break;
                }
            }
        }
    }

    freeifaddrs(ifaddr);

    if (interfaceName.empty())
    {
        return std::nullopt;
    }

    // Get the MTU for the interface
    ifreq ifr;
    std::strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
    if (ioctl(this->g_socket, SIOCGIFMTU, &ifr) != 0)
    {
        return std::nullopt;
    }

    return std::clamp<uint32_t>(ifr.ifr_mtu,
                                this->g_addressType == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_MIN_MTU
                                                                              : FGE_SOCKET_IPV6_MIN_MTU,
                                FGE_SOCKET_FULL_DATAGRAM_SIZE);
#endif // _WIN32
}
std::vector<Socket::AdapterInfo> Socket::getAdaptersInfo(IpAddress::Types type)
{
#ifdef _WIN32
    std::vector<AdapterInfo> adapters;

    auto const family =
            type == IpAddress::Types::Ipv4 ? AF_INET : (type == IpAddress::Types::Ipv6 ? AF_INET6 : AF_UNSPEC);

    ULONG bufferSize = 0;
    GetAdaptersAddresses(family, 0, nullptr, nullptr, &bufferSize);

    std::unique_ptr<PIP_ADAPTER_ADDRESSES[]> const adapterAddresses =
            std::make_unique<PIP_ADAPTER_ADDRESSES[]>(bufferSize);
    PIP_ADAPTER_ADDRESSES adapterAddress = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapterAddresses.get());

    if (GetAdaptersAddresses(family, 0, nullptr, adapterAddress, &bufferSize) == NO_ERROR)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

        for (PIP_ADAPTER_ADDRESSES adapter = adapterAddress; adapter != nullptr; adapter = adapter->Next)
        {
            AdapterInfo info;
            info._name = converter.to_bytes(adapter->FriendlyName);
            info._description = converter.to_bytes(adapter->Description);

            for (auto* unicast = adapter->FirstUnicastAddress; unicast != nullptr; unicast = unicast->Next)
            {
                auto& data = info._data.emplace_back();
                auto const addrFamily = unicast->Address.lpSockaddr->sa_family;

                if (addrFamily == AF_INET)
                {
                    sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(unicast->Address.lpSockaddr);
                    data._unicast.setNetworkByteOrdered(addr4->sin_addr.s_addr);
                }
                else
                {
                    sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(unicast->Address.lpSockaddr);
                    IpAddress::Ipv6Data ipv6Data;
                    std::memcpy(ipv6Data.data(), addr6->sin6_addr.s6_addr, 16);
                    data._unicast.setNetworkByteOrdered(ipv6Data);
                }
            }

            info._mtu = std::clamp<uint32_t>(adapter->Mtu,
                                             (adapter->Flags & IP_ADAPTER_IPV6_ENABLED) > 0 ? FGE_SOCKET_IPV6_MIN_MTU
                                                                                            : FGE_SOCKET_IPV4_MIN_MTU,
                                             FGE_SOCKET_FULL_DATAGRAM_SIZE);

            adapters.push_back(info);
        }
    }

    return adapters;
#else
    std::vector<AdapterInfo> adapters;

    auto const family =
            type == IpAddress::Types::Ipv4 ? AF_INET : (type == IpAddress::Types::Ipv6 ? AF_INET6 : AF_UNSPEC);

    ifaddrs* ifaddr{nullptr};
    if (getifaddrs(&ifaddr) != 0 || ifaddr == nullptr)
    {
        return adapters;
    }

    auto socket = ::socket(family == AF_UNSPEC ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    for (ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
        {
            continue;
        }

        if (family == ifa->ifa_addr->sa_family || family == AF_UNSPEC)
        {
            IpAddress ip;
            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
                ip.setNetworkByteOrdered(addr4->sin_addr.s_addr);
            }
            else
            {
                sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(ifa->ifa_addr);
                IpAddress::Ipv6Data data;
                std::memcpy(data.data(), addr6->sin6_addr.s6_addr, 16);
                ip.setNetworkByteOrdered(data);
            }

            bool found = false;
            for (auto& adapter: adapters)
            {
                if (adapter._name == ifa->ifa_name)
                {
                    adapter._data.emplace_back()._unicast = ip;
                    found = true;
                }
            }

            if (found)
            {
                continue;
            }

            auto& data = adapters.emplace_back();
            data._data.emplace_back()._unicast = ip;
            data._name = ifa->ifa_name;
            data._description = ifa->ifa_name; //TODO: Get the description

            ifreq ifr;
            std::strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
            if (ioctl(socket, SIOCGIFMTU, &ifr) != 0)
            {
                data._mtu = 0;
                continue;
            }

            data._mtu = std::clamp<uint32_t>(ifr.ifr_mtu,
                                             family == AF_INET ? FGE_SOCKET_IPV4_MIN_MTU : FGE_SOCKET_IPV6_MIN_MTU,
                                             FGE_SOCKET_FULL_DATAGRAM_SIZE);
        }
    }

    freeifaddrs(ifaddr);
    ::close(socket);

    return adapters;
#endif // _WIN32
}

int Socket::getPlatformSpecifiedError()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif // _WIN32
}

///SocketUdp

SocketUdp::SocketUdp(IpAddress::Types addressType) :
        Socket(Types::UDP, addressType),
        g_buffer(FGE_SOCKET_FULL_DATAGRAM_SIZE)
{
    //Create UDP socket
    this->create();

    //Set the blocking state to false by default
    this->setBlocking(false);

    //Enable broadcast by default
    this->setBroadcastOption(true);
}
SocketUdp::SocketUdp(IpAddress::Types addressType, bool blocking, bool broadcast) :
        Socket(Types::UDP, addressType),
        g_buffer(FGE_SOCKET_FULL_DATAGRAM_SIZE)
{
    //Create UDP socket
    this->create();

    //Set the blocking state
    this->setBlocking(blocking);

    //Enable/Disable broadcast
    this->setBroadcastOption(broadcast);
}

Socket::Errors SocketUdp::create()
{
    if (this->g_addressType == IpAddress::Types::None)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    if (this->g_socket == _FGE_SOCKET_INVALID)
    {
        //Create UDP socket
        this->g_socket =
                socket(this->g_addressType == IpAddress::Types::Ipv4 ? AF_INET : AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

        if (this->g_socket == _FGE_SOCKET_INVALID)
        { //Check if valid
            return NormalizeError();
        }
    }
    return Errors::ERR_NOERROR;
}

Socket::Errors SocketUdp::connect(IpAddress const& remoteAddress, Port remotePort)
{
    if (remoteAddress.getType() == IpAddress::Types::None)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Create the remote address
    sockaddr_in addr4{};
    sockaddr_in6 addr6{};
    int size = 0;
    auto* addr = CreateAddress(addr4, addr6, size, remoteAddress, remotePort);

    if (::connect(this->g_socket, addr, size) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    return Errors::ERR_NOERROR;
}
Socket::Errors SocketUdp::disconnect()
{
    //MSDN: If the address member of the structure specified by name is filled with zeros, the socket will be disconnected.

    sockaddr_in addr4{};
    sockaddr_in6 addr6{};
    sockaddr* addr = nullptr;
    int size = 0;

    if (this->g_addressType == IpAddress::Types::Ipv4)
    {
        addr = reinterpret_cast<sockaddr*>(&addr4);
        size = sizeof(addr4);
    }
    else
    {
        addr = reinterpret_cast<sockaddr*>(&addr6);
        size = sizeof(addr6);
    }

    std::memset(addr, 0, size);

    if (::connect(this->g_socket, addr, size) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    return Errors::ERR_NOERROR;
}
Socket::Errors SocketUdp::bind(Port port, IpAddress const& address)
{
    // Close the socket if it is already bound
    close();

    // Create the internal socket if it doesn't exist
    create();

    // Check if the address is valid
    if (address == IpAddress::None || address == IpAddress::Ipv4Broadcast)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Bind the socket
    sockaddr_in addr4{};
    sockaddr_in6 addr6{};
    int size = 0;
    auto* addr = CreateAddress(addr4, addr6, size, address, port);

    if (::bind(this->g_socket, addr, size) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    return Errors::ERR_NOERROR;
}

Socket::Errors SocketUdp::sendTo(void const* data, std::size_t size, IpAddress const& remoteAddress, Port remotePort)
{
    // Create the internal socket if it doesn't exist
    create();

    // Make sure that all the data will fit in one datagram
    if (size > FGE_SOCKET_FULL_DATAGRAM_SIZE)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Build the target address
    sockaddr_in addr4{};
    sockaddr_in6 addr6{};
    int addrSize = 0;
    auto* addr = CreateAddress(addr4, addr6, addrSize, remoteAddress, remotePort);

    // Send the data (unlike TCP, all the data is always sent in one call)
    int sent = sendto(this->g_socket, static_cast<char const*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG, addr,
                      addrSize);

    // Check for errors
    if (sent == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    return Errors::ERR_NOERROR;
}
Socket::Errors SocketUdp::send(void const* data, std::size_t size)
{
    if ((data == nullptr) || (size == 0))
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    int sent = ::send(this->g_socket, static_cast<char const*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG);

    // Check for errors
    if (sent == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    return Errors::ERR_NOERROR;
}
Socket::Errors
SocketUdp::receiveFrom(void* data, std::size_t size, std::size_t& received, IpAddress& remoteAddress, Port& remotePort)
{
    // First clear the variables to fill
    received = 0;
    remoteAddress = IpAddress();
    remotePort = 0;

    // Check the destination buffer
    if (data == nullptr)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Data that will be filled with the other computer's address
    sockaddr_in addr4{};
    sockaddr_in6 addr6{};
    sockaddr* addr = nullptr;
    int addrSize = 0;
    if (this->g_addressType == IpAddress::Types::Ipv4)
    {
        addr4.sin_addr.s_addr = INADDR_ANY;
        addr4.sin_port = 0;
        addr = reinterpret_cast<sockaddr*>(&addr4);
        addrSize = sizeof(sockaddr_in);
#ifdef _FGE_MACOS
        addr4.sin_len = addrSize;
#endif
    }
    else
    {
        addr6.sin6_addr = in6addr_any;
        addr6.sin6_port = 0;
        addr = reinterpret_cast<sockaddr*>(&addr6);
        addrSize = sizeof(sockaddr_in6);
#ifdef _FGE_MACOS
        addr6.sin6_len = addrSize;
#endif
    }

    // Receive a chunk of bytes
    SocketLength addressSize = addrSize;
    int sizeReceived = recvfrom(this->g_socket, static_cast<char*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG,
                                addr, &addressSize);

    // Check for errors
    if (sizeReceived == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    // Fill the sender informations
    received = static_cast<std::size_t>(sizeReceived);
    if (this->g_addressType == IpAddress::Types::Ipv4)
    {
        remoteAddress.setNetworkByteOrdered(addr4.sin_addr.s_addr);
        remotePort = fge::SwapHostNetEndian_16(addr4.sin_port);
    }
    else
    {
        remoteAddress.setNetworkByteOrdered(addr6.sin6_addr.s6_addr);
        remotePort = fge::SwapHostNetEndian_16(addr6.sin6_port);
    }
    return Errors::ERR_NOERROR;
}
Socket::Errors SocketUdp::receive(void* data, std::size_t size, std::size_t& received)
{
    // First clear the variables to fill
    received = 0;

    // Check the destination buffer
    if (data == nullptr)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Receive a chunk of bytes
    int sizeReceived = recv(this->g_socket, static_cast<char*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG);

    // Check for errors
    if (sizeReceived == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    // Fill the sender informations
    received = static_cast<std::size_t>(sizeReceived);

    return Errors::ERR_NOERROR;
}

Socket::Errors SocketUdp::send(Packet& packet)
{
    if (packet.getDataSize() == 0)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    if (!packet._g_lastDataValidity)
    {
        packet.onSend(packet._g_lastData, 0);
        packet._g_sendPos = 0;
    }

    int sent = ::send(this->g_socket, reinterpret_cast<char const*>(packet._g_lastData.data()),
                      static_cast<int>(packet._g_lastData.size()), _FGE_SEND_RECV_FLAG);

    // Check for errors
    if (sent == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    return Errors::ERR_NOERROR;
}
Socket::Errors SocketUdp::sendTo(Packet& packet, IpAddress const& remoteAddress, Port remotePort)
{
    // Create the internal socket if it doesn't exist
    create();

    // Make sure that all the data will fit in one datagram
    if (packet.getDataSize() > FGE_SOCKET_FULL_DATAGRAM_SIZE)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Build the target address
    sockaddr_in addr4{};
    sockaddr_in6 addr6{};
    int addrSize = 0;
    auto* addr = CreateAddress(addr4, addr6, addrSize, remoteAddress, remotePort);

    if (!packet._g_lastDataValidity)
    {
        packet.onSend(packet._g_lastData, 0);
        packet._g_sendPos = 0;
    }

    // Send the data (unlike TCP, all the data is always sent in one call)
    int sent = sendto(this->g_socket, reinterpret_cast<char const*>(packet._g_lastData.data()),
                      static_cast<int>(packet._g_lastData.size()), _FGE_SEND_RECV_FLAG, addr, addrSize);

    // Check for errors
    if (sent == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    return Errors::ERR_NOERROR;
}
Socket::Errors SocketUdp::receiveFrom(Packet& packet, IpAddress& remoteAddress, Port& remotePort)
{
    size_t received = 0;
    Socket::Errors status =
            this->receiveFrom(this->g_buffer.data(), this->g_buffer.size(), received, remoteAddress, remotePort);

    packet.clear();
    if ((status == Errors::ERR_NOERROR) && (received > 0))
    {
        packet.onReceive(this->g_buffer.data(), received);
    }
    return status;
}
Socket::Errors SocketUdp::receive(Packet& packet)
{
    size_t received = 0;
    Errors status = this->receive(this->g_buffer.data(), this->g_buffer.size(), received);

    packet.clear();
    if ((status == Errors::ERR_NOERROR) && (received > 0))
    {
        packet.onReceive(this->g_buffer.data(), received);
    }
    return status;
}

std::optional<uint16_t> SocketUdp::retrieveAdapterMTUForDestination(IpAddress const& destination)
{
    //Create a temporary socket
    SocketUdp tempSocket;
    tempSocket.bind(FGE_ANYPORT,
                    destination.getType() == IpAddress::Types::Ipv4 ? IpAddress::Ipv4Any : IpAddress::Ipv6Any);
    tempSocket.connect(destination, FGE_ANYPORT);
    return tempSocket.retrieveCurrentAdapterMTU();
}

SocketUdp& SocketUdp::operator=(SocketUdp&& r) noexcept
{
    this->g_isBlocking = r.g_isBlocking;
    this->g_type = r.g_type;
    this->g_socket = r.g_socket;
    r.g_socket = _FGE_SOCKET_INVALID;
    return *this;
}

///SocketTcp

SocketTcp::SocketTcp(IpAddress::Types addressType) :
        Socket(Types::TCP, addressType),
        g_receivedSize(0),
        g_wantedSize(0),
        g_buffer(FGE_SOCKET_TCP_DEFAULT_BUFFERSIZE)
{
    //Create TCP socket
    this->create();

    //Set the blocking state to false by default
    this->setBlocking(false);
}
SocketTcp::SocketTcp(IpAddress::Types addressType, bool blocking) :
        Socket(Types::TCP, addressType),
        g_receivedSize(0),
        g_wantedSize(0),
        g_buffer(FGE_SOCKET_TCP_DEFAULT_BUFFERSIZE)
{
    //Create TCP socket
    this->create();

    //Set the blocking state
    this->setBlocking(blocking);
}

void SocketTcp::flush()
{
    this->g_receivedSize = 0;
    this->g_wantedSize = 0;
    this->g_buffer.clear();
    this->g_buffer.resize(FGE_SOCKET_TCP_DEFAULT_BUFFERSIZE);
}

Socket::Errors SocketTcp::create(SocketDescriptor sck)
{
    if (sck == _FGE_SOCKET_INVALID)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    this->close();

    this->g_receivedSize = 0;
    this->g_wantedSize = 0;

    //Create TCP socket
    this->g_socket = sck;

    //Disable the Nagle algorithm
    char const optval = 1;
    if (setsockopt(this->g_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

// On Mac OS X, disable the SIGPIPE signal on disconnection
#ifdef _FGE_MACOS
    if (setsockopt(this->g_socket, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }
#endif

    return this->setBlocking(this->g_isBlocking);
}
Socket::Errors SocketTcp::create()
{
    if (this->g_addressType == IpAddress::Types::None)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    if (this->g_socket == _FGE_SOCKET_INVALID)
    {
        //Create TCP socket
        this->g_socket =
                socket(this->g_addressType == IpAddress::Types::Ipv4 ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);

        if (this->g_socket == _FGE_SOCKET_INVALID)
        { //Check if valid
            return NormalizeError();
        }

        //Disable the Nagle algorithm
        char const optval = 1;
        if (setsockopt(this->g_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
        {
            return NormalizeError();
        }

// On Mac OS X, disable the SIGPIPE signal on disconnection
#ifdef _FGE_MACOS
        if (setsockopt(this->g_socket, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
        {
            return NormalizeError();
        }
#endif
    }
    return Errors::ERR_NOERROR;
}

Socket::Errors SocketTcp::connect(IpAddress const& remoteAddress, Port remotePort, uint32_t timeoutms)
{
    // Disconnect the socket if it is already connected
    this->close();

    this->g_receivedSize = 0;
    this->g_wantedSize = 0;

    // Create the internal socket if it doesn't exist
    this->create();

    // Create the remote address
    sockaddr_in addr4{};
    sockaddr_in6 addr6{};
    int addrSize = 0;
    auto* addr = CreateAddress(addr4, addr6, addrSize, remoteAddress, remotePort);

    if (timeoutms == 0)
    {
        // Connect the socket
        if (::connect(this->g_socket, addr, addrSize) == _FGE_SOCKET_ERROR)
        {
            return NormalizeError();
        }

        // Connection succeeded
        return Errors::ERR_NOERROR;
    }
    // Save the previous blocking state
    bool blocking = this->isBlocking();

    // Switch to non-blocking to enable our connection timeout
    if (blocking)
    {
        this->setBlocking(false);
    }

    // Try to connect to the remote address
    if (::connect(this->g_socket, addr, addrSize) != _FGE_SOCKET_ERROR)
    {
        // We got instantly connected! (it may no happen a lot...)
        this->setBlocking(blocking);
        return Errors::ERR_NOERROR;
    }

    // Get the error status
    Errors status = NormalizeError();

    // If we were in non-blocking mode, return immediately
    if (!blocking)
    {
        return status;
    }

    // Otherwise, wait until something happens to our socket (success, timeout or error)
    if (status == Errors::ERR_NOTREADY)
    {
        // Setup the selector
        fd_set selector;
#ifdef _WIN32
        selector.fd_count = 1;
        *selector.fd_array = this->g_socket;
#else
        FD_ZERO(&selector);
        FD_SET(this->g_socket, &selector);
#endif // _WIN32

        // Setup the timeout
        timeval time{};
        time.tv_sec = static_cast<long>(timeoutms / 1000);
        time.tv_usec = static_cast<long>((timeoutms % 1000) * 1000);

        // Wait for something to write on our socket (which means that the connection request has returned)
        if (::select(static_cast<int>(this->g_socket + 1), nullptr, &selector, nullptr, &time) == 1)
        {
            // Socket selected for write, Check for errors
            int32_t optval;
            SocketLength optlen = sizeof(optval);

            if (getsockopt(this->g_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen) ==
                _FGE_SOCKET_ERROR)
            {
                return NormalizeError();
            }
            // Check the value returned...
            if (optval != 0)
            {
                return NormalizeError(optval);
            }
            status = Errors::ERR_NOERROR;
        }
        else
        {
            // Failed to connect before timeout is over
            status = NormalizeError();
        }
    }

    // Switch back to blocking mode
    this->setBlocking(true);

    return status;
}

Socket::Errors SocketTcp::send(void const* data, std::size_t size)
{
    std::size_t sent;
    return this->send(data, size, sent);
}
Socket::Errors SocketTcp::send(void const* data, std::size_t size, std::size_t& sent)
{
    // Check the parameters
    if ((data == nullptr) || (size == 0))
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Loop until every byte has been sent
    int result = 0;
    for (sent = 0; sent < size; sent += result)
    {
        // Send a chunk of data
        result = ::send(this->g_socket, static_cast<char const*>(data) + sent, static_cast<int>(size - sent),
                        _FGE_SEND_RECV_FLAG);

        // Check for errors
        if (result == _FGE_SOCKET_ERROR)
        {
            Errors status = NormalizeError();

            if ((status == Errors::ERR_NOTREADY) && (sent > 0))
            {
                return Errors::ERR_PARTIAL;
            }
            return status;
        }
    }

    return Errors::ERR_NOERROR;
}
Socket::Errors SocketTcp::receive(void* data, std::size_t size, std::size_t& received)
{
    // First clear the variables to fill
    received = 0;

    // Check the destination buffer
    if (data == nullptr)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Receive a chunk of bytes
    int sizeReceived = recv(this->g_socket, static_cast<char*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG);

    // Check the number of bytes received
    if (sizeReceived > 0)
    {
        received = static_cast<std::size_t>(sizeReceived);
        return Errors::ERR_NOERROR;
    }
    else if (sizeReceived == 0)
    {
        return Errors::ERR_DISCONNECTED;
    }
    else
    {
        return NormalizeError();
    }
}
Socket::Errors SocketTcp::receive(void* data, std::size_t size, std::size_t& received, uint32_t timeoutms)
{
    Errors error = this->select(true, timeoutms);
    if (error == Errors::ERR_NOERROR)
    {
        return this->receive(data, size, received);
    }
    return error;
}

Socket::Errors SocketTcp::send(Packet& packet)
{
    if (!packet._g_lastDataValidity)
    { // New packet that gonna be sent
        packet.onSend(packet._g_lastData, sizeof(uint32_t));
        *reinterpret_cast<uint32_t*>(packet._g_lastData.data()) = fge::SwapHostNetEndian_32(packet._g_lastData.size());
        packet._g_sendPos = 0;
    }

    // Send the data block
    std::size_t sent;
    Errors status = this->send(packet._g_lastData.data() + packet._g_sendPos,
                               packet._g_lastData.size() - packet._g_sendPos, sent);

    // In the case of a partial send, record the location to resume from
    if (status == Errors::ERR_PARTIAL)
    {
        packet._g_sendPos += sent;
    }
    else if (status == Errors::ERR_NOERROR)
    {
        packet._g_sendPos = 0;
    }

    return status;
}
Socket::Errors SocketTcp::receive(Packet& packet)
{
    if (this->g_receivedSize == 0)
    { // New packet is here
        std::size_t received;

        this->g_buffer.resize(sizeof(uint32_t));
        Socket::Errors status = this->receive(this->g_buffer.data(), sizeof(uint32_t), received);

        if (received == 0)
        {
            return status;
        }

        this->g_receivedSize += received;

        if (this->g_receivedSize >= sizeof(uint32_t))
        {
            this->g_wantedSize = fge::SwapHostNetEndian_32(*reinterpret_cast<uint32_t*>(this->g_buffer.data()));
            if (this->g_wantedSize == 0)
            { // Received a bad size
                this->g_receivedSize = 0;
                this->g_wantedSize = 0;
                return Errors::ERR_UNSUCCESS;
            }
            this->g_buffer.resize(this->g_wantedSize + sizeof(uint32_t));
        }

        return Errors::ERR_PARTIAL;
    }
    else
    { // Already on a pending packet
        if (this->g_wantedSize > 0)
        { // We have already the wanted size
            std::size_t received;
            Errors status = this->receive(this->g_buffer.data() + this->g_receivedSize,
                                          this->g_buffer.size() - this->g_receivedSize, received);

            if (received == 0)
            {
                return status;
            }

            this->g_receivedSize += received;

            if (this->g_wantedSize == this->g_receivedSize)
            { // Well we finished this pending packet
                packet.clear();
                packet.onReceive(this->g_buffer.data() + sizeof(uint32_t), this->g_wantedSize - sizeof(uint32_t));
                this->g_receivedSize = 0;
                this->g_wantedSize = 0;
                return Errors::ERR_DONE;
            }

            return Errors::ERR_PARTIAL;
        }
        else
        { // We don't have the wanted size
            std::size_t received;
            Errors status = this->receive(this->g_buffer.data() + this->g_receivedSize,
                                          sizeof(uint32_t) - this->g_receivedSize, received);

            if (received == 0)
            {
                return status;
            }

            this->g_receivedSize += received;

            if (this->g_receivedSize >= sizeof(uint32_t))
            {
                this->g_wantedSize = fge::SwapHostNetEndian_32(*reinterpret_cast<uint32_t*>(this->g_buffer.data()));
                if (this->g_wantedSize == 0)
                { // Received a bad size
                    this->g_receivedSize = 0;
                    this->g_wantedSize = 0;
                    return Errors::ERR_UNSUCCESS;
                }
                this->g_buffer.resize(this->g_wantedSize + sizeof(uint32_t));
            }

            return Errors::ERR_PARTIAL;
        }
    }
}

Socket::Errors SocketTcp::sendAndReceive(Packet& sendPacket, Packet& receivePacket, uint32_t timeoutms)
{
    Socket::Errors error = this->send(sendPacket);
    if (error == Errors::ERR_NOERROR)
    {
        error = this->select(true, timeoutms);
        if (error == Errors::ERR_NOERROR)
        {
            return this->receive(receivePacket);
        }
    }
    return error;
}
Socket::Errors SocketTcp::receive(fge::net::Packet& packet, uint32_t timeoutms)
{
    Errors error = this->select(true, timeoutms);
    if (error == Errors::ERR_NOERROR)
    {
        return this->receive(packet);
    }
    return error;
}

SocketTcp& SocketTcp::operator=(SocketTcp&& r) noexcept
{
    this->g_isBlocking = r.g_isBlocking;
    this->g_type = r.g_type;
    this->g_socket = r.g_socket;
    this->g_receivedSize = r.g_receivedSize;
    this->g_wantedSize = r.g_wantedSize;
    this->g_buffer.swap(r.g_buffer);

    r.g_receivedSize = 0;
    r.g_wantedSize = 0;
    r.g_socket = _FGE_SOCKET_INVALID;
    return *this;
}

///SocketListenerTcp

SocketListenerTcp::SocketListenerTcp(IpAddress::Types addressType) :
        Socket(Types::TCP_LISTENER, addressType)
{
    //Create TCP socket
    this->create();

    //Set the blocking state to false by default
    this->setBlocking(false);
}
SocketListenerTcp::SocketListenerTcp(IpAddress::Types addressType, bool blocking) :
        Socket(Types::TCP_LISTENER, addressType)
{
    //Create TCP socket
    this->create();

    //Set the blocking state
    this->setBlocking(blocking);
}

Socket::Errors SocketListenerTcp::create()
{
    if (this->g_addressType == IpAddress::Types::None)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    if (this->g_socket == _FGE_SOCKET_INVALID)
    {
        //Create TCP socket
        this->g_socket =
                socket(this->g_addressType == IpAddress::Types::Ipv4 ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);

        if (this->g_socket == _FGE_SOCKET_INVALID)
        { //Check if valid
            return NormalizeError();
        }

        //Disable the Nagle algorithm
        char const optval = 1;
        if (setsockopt(this->g_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
        {
            return NormalizeError();
        }

// On Mac OS X, disable the SIGPIPE signal on disconnection
#ifdef _FGE_MACOS
        if (setsockopt(this->g_socket, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
        {
            return NormalizeError();
        }
#endif
    }
    return Errors::ERR_NOERROR;
}

Socket::Errors SocketListenerTcp::listen(Port port, IpAddress const& address)
{
    // Close the socket if it is already bound
    this->close();

    // Create the internal socket if it doesn't exist
    this->create();

    // Check if the address is valid
    if (address == IpAddress::None || address == IpAddress::Ipv4Broadcast)
    {
        return Errors::ERR_INVALIDARGUMENT;
    }

    // Bind the socket to the specified port
    sockaddr_in addr4{};
    sockaddr_in6 addr6{};
    int addrSize = 0;
    auto* addr = CreateAddress(addr4, addr6, addrSize, address, port);

    if (bind(this->g_socket, addr, addrSize) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    // Listen to the bound port
    if (::listen(this->g_socket, SOMAXCONN) == _FGE_SOCKET_ERROR)
    {
        return NormalizeError();
    }

    return Errors::ERR_NOERROR;
}
Socket::Errors SocketListenerTcp::accept(SocketTcp& socket)
{
    // Make sure that we're listening
    if (this->g_socket == _FGE_SOCKET_INVALID)
    {
        return Errors::ERR_DISCONNECTED;
    }

    // Accept a new connection
    sockaddr_in address{};
    SocketLength length = sizeof(address);
    SocketDescriptor remote = ::accept(this->g_socket, reinterpret_cast<sockaddr*>(&address), &length);

    // Check for errors
    if (remote == _FGE_SOCKET_INVALID)
    {
        return NormalizeError();
    }

    // Initialize the new connected socket
    socket.close();
    socket.create(remote);

    return Errors::ERR_NOERROR;
}

SocketListenerTcp& SocketListenerTcp::operator=(SocketListenerTcp&& r) noexcept
{
    this->g_isBlocking = r.g_isBlocking;
    this->g_type = r.g_type;
    this->g_socket = r.g_socket;
    r.g_socket = _FGE_SOCKET_INVALID;
    return *this;
}

} // namespace fge::net
