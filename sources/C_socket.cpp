/*
 * Copyright 2022 Guillaume Guillet
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

#include "FastEngine/C_socket.hpp"
#include "FastEngine/C_packet.hpp"
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
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
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
    fge::net::Socket::Error NormalizeError()
    {
        #ifdef _WIN32
        int err = WSAGetLastError();

        switch (err)
        {
            case WSANOTINITIALISED: return fge::net::Socket::Error::ERR_NOTINIT;

            case WSAEWOULDBLOCK:    return fge::net::Socket::Error::ERR_NOTREADY;
            case WSAEALREADY:       return fge::net::Socket::Error::ERR_NOTREADY;
            case WSAEINPROGRESS:    return fge::net::Socket::Error::ERR_NOTREADY;

            case WSAETIMEDOUT:      return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAECONNABORTED:   return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAECONNRESET:     return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAENETRESET:      return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAENOTCONN:       return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAENETUNREACH:    return fge::net::Socket::Error::ERR_DISCONNECTED;

            case WSAECONNREFUSED:   return fge::net::Socket::Error::ERR_REFUSED;

            case WSAEADDRINUSE:     return fge::net::Socket::Error::ERR_ALREADYUSED;
            case WSAEISCONN:        return fge::net::Socket::Error::ERR_ALREADYCONNECTED;

            case WSAEMFILE:         return fge::net::Socket::Error::ERR_TOOMANYSOCKET;

            default:                return fge::net::Socket::Error::ERR_UNSUCCESS;
        }
        #else
        int err = errno;

        if ((err == EAGAIN) || (err == EINPROGRESS))
        {
            return fge::net::Socket::Error::ERR_NOTREADY;
        }

        switch (err)
        {
            case EWOULDBLOCK:       return fge::net::Socket::Error::ERR_NOTREADY;
            case EALREADY:          return fge::net::Socket::Error::ERR_NOTREADY;
            case EINPROGRESS:       return fge::net::Socket::Error::ERR_NOTREADY;

            case ETIMEDOUT:         return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ECONNABORTED:      return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ECONNRESET:        return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ENETRESET:         return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ENOTCONN:          return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ENETUNREACH:       return fge::net::Socket::Error::ERR_DISCONNECTED;
            case EPIPE:             return fge::net::Socket::Error::ERR_DISCONNECTED;

            case ECONNREFUSED:      return fge::net::Socket::Error::ERR_REFUSED;

            case EADDRINUSE:        return fge::net::Socket::Error::ERR_ALREADYUSED;
            case EISCONN:           return fge::net::Socket::Error::ERR_ALREADYCONNECTED;

            case EMFILE:            return fge::net::Socket::Error::ERR_TOOMANYSOCKET;

            default:                return fge::net::Socket::Error::ERR_UNSUCCESS;
        }
        #endif // _WIN32
    }
    fge::net::Socket::Error NormalizeError(int err)
    {
        #ifdef _WIN32
        switch (err)
        {
            case WSANOTINITIALISED: return fge::net::Socket::Error::ERR_NOTINIT;

            case WSAEWOULDBLOCK:    return fge::net::Socket::Error::ERR_NOTREADY;
            case WSAEALREADY:       return fge::net::Socket::Error::ERR_NOTREADY;
            case WSAEINPROGRESS:    return fge::net::Socket::Error::ERR_NOTREADY;

            case WSAETIMEDOUT:      return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAECONNABORTED:   return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAECONNRESET:     return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAENETRESET:      return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAENOTCONN:       return fge::net::Socket::Error::ERR_DISCONNECTED;
            case WSAENETUNREACH:    return fge::net::Socket::Error::ERR_DISCONNECTED;

            case WSAECONNREFUSED:   return fge::net::Socket::Error::ERR_REFUSED;

            case WSAEADDRINUSE:     return fge::net::Socket::Error::ERR_ALREADYUSED;
            case WSAEISCONN:        return fge::net::Socket::Error::ERR_ALREADYCONNECTED;

            case WSAEMFILE:         return fge::net::Socket::Error::ERR_TOOMANYSOCKET;

            default:                return fge::net::Socket::Error::ERR_UNSUCCESS;
        }
        #else
        if ((err == EAGAIN) || (err == EINPROGRESS))
        {
            return fge::net::Socket::Error::ERR_NOTREADY;
        }

        switch (err)
        {
            case EWOULDBLOCK:       return fge::net::Socket::Error::ERR_NOTREADY;
            case EALREADY:          return fge::net::Socket::Error::ERR_NOTREADY;
            case EINPROGRESS:       return fge::net::Socket::Error::ERR_NOTREADY;

            case ETIMEDOUT:         return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ECONNABORTED:      return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ECONNRESET:        return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ENETRESET:         return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ENOTCONN:          return fge::net::Socket::Error::ERR_DISCONNECTED;
            case ENETUNREACH:       return fge::net::Socket::Error::ERR_DISCONNECTED;
            case EPIPE:             return fge::net::Socket::Error::ERR_DISCONNECTED;

            case ECONNREFUSED:      return fge::net::Socket::Error::ERR_REFUSED;

            case EADDRINUSE:        return fge::net::Socket::Error::ERR_ALREADYUSED;
            case EISCONN:           return fge::net::Socket::Error::ERR_ALREADYCONNECTED;

            case EMFILE:            return fge::net::Socket::Error::ERR_TOOMANYSOCKET;

            default:                return fge::net::Socket::Error::ERR_UNSUCCESS;
        }
        #endif // _WIN32
    }
}

///Socket

Socket::Socket(fge::net::Socket::Type type) :
    g_type(type),
    g_socket(_FGE_SOCKET_INVALID)
{
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

fge::net::Port Socket::getLocalPort() const
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        sockaddr_in address{};
        fge::net::SocketLength addressSize = sizeof(address);
        if (getsockname(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
        {
            return fge::SwapHostNetEndian_16(address.sin_port);
        }
    }
    return 0;
}
fge::net::IpAddress Socket::getLocalAddress() const
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        sockaddr_in address{};
        fge::net::SocketLength addressSize = sizeof(address);
        if (getsockname(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
        {
            return { fge::SwapHostNetEndian_32(address.sin_addr.s_addr) };
        }
    }
    return fge::net::IpAddress::None;
}
fge::net::Port Socket::getRemotePort() const
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        sockaddr_in address{};
        fge::net::SocketLength addressSize = sizeof(address);
        if (getpeername(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
        {
            return fge::SwapHostNetEndian_16(address.sin_port);
        }
    }
    return 0;
}
fge::net::IpAddress Socket::getRemoteAddress() const
{
    if (this->g_socket != _FGE_SOCKET_INVALID)
    {
        sockaddr_in address{};
        fge::net::SocketLength addressSize = sizeof(address);
        if (getpeername(this->g_socket, reinterpret_cast<sockaddr*>(&address), &addressSize) != _FGE_SOCKET_ERROR)
        {
            return { fge::SwapHostNetEndian_32(address.sin_addr.s_addr) };
        }
    }
    return fge::net::IpAddress::None;
}

bool Socket::isBlocking() const
{
    return this->g_isBlocking;
}

fge::net::Socket::Error Socket::setBlocking(bool mode)
{
    #ifdef _WIN32
        unsigned long iMode = mode ? 0 : 1;
        if ( ioctlsocket(this->g_socket, FIONBIO, &iMode) == _FGE_SOCKET_ERROR )
        {
            return fge::net::NormalizeError();
        }
        this->g_isBlocking = mode;
        return fge::net::Socket::ERR_NOERROR;
    #else
        int status = fcntl(this->g_socket, F_GETFL);
        if (mode)
        {
            if ( fcntl(this->g_socket, F_SETFL, status & ~O_NONBLOCK) == _FGE_SOCKET_ERROR )
            {
                return fge::net::NormalizeError();
            }
        }
        else
        {
            if ( fcntl(this->g_socket, F_SETFL, status | O_NONBLOCK) == _FGE_SOCKET_ERROR )
            {
                return fge::net::NormalizeError();
            }
        }
        this->g_isBlocking = mode;
        return fge::net::Socket::ERR_NOERROR;
    #endif //_WIN32
}
fge::net::Socket::Error Socket::setReuseAddress(bool mode)
{
    const char optval = mode ? 1 : 0;
    if ( setsockopt(this->g_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR )
    {
        return fge::net::NormalizeError();
    }
    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error Socket::setBroadcastOption(bool mode)
{
    const char optval = mode ? 1 : 0;
    if ( setsockopt(this->g_socket, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR )
    {
        return fge::net::NormalizeError();
    }
    return fge::net::Socket::ERR_NOERROR;
}

fge::net::Socket::Error Socket::select(bool read, uint32_t timeoutms)
{
    // Setup the selector
    fd_set selector;
    #ifdef _win32
    selector.fd_count = 1;
    *selector.fd_array = this->g_socket;
    #else
    FD_ZERO(&selector);
    FD_SET(this->g_socket, &selector);
    #endif // _win32

    // Setup the timeout
    timeval time{};
    time.tv_sec  = static_cast<long>(timeoutms / 1000);
    time.tv_usec = static_cast<long>((timeoutms%1000) * 1000);

    // Wait for something
    if (::select(static_cast<int>(this->g_socket + 1), (read ? &selector : nullptr), (!read ? &selector : nullptr), nullptr, &time) == 1)
    {
        // Socket selected for write, Check for errors
        int32_t optval = 0;
        fge::net::SocketLength optlen = sizeof(optval);

        if (getsockopt(this->g_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen) == _FGE_SOCKET_ERROR)
        {
            return fge::net::NormalizeError();
        }
        // Check the value returned...
        if (optval != 0)
        {
            return fge::net::NormalizeError(optval);
        }
        return fge::net::Socket::ERR_NOERROR;
    }
    // Failed to connect before timeout is over
    return fge::net::NormalizeError();
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

int Socket::getPlatformSpecifiedError()
{
    #ifdef _WIN32
    return WSAGetLastError();
    #else
    return errno;
    #endif // _WIN32
}

///SocketUdp

SocketUdp::SocketUdp() :
    Socket(fge::net::Socket::TYPE_UDP),
    g_buffer(FGE_SOCKET_MAXDATAGRAMSIZE)
{
    //Create UDP socket
    this->create();

    //Set the blocking state to false by default
    this->setBlocking(false);

    //Enable broadcast by default
    this->setBroadcastOption(true);
}
SocketUdp::SocketUdp(bool blocking, bool broadcast) :
    Socket(fge::net::Socket::TYPE_UDP),
    g_buffer(FGE_SOCKET_MAXDATAGRAMSIZE)
{
    //Create UDP socket
    this->create();

    //Set the blocking state
    this->setBlocking(blocking);

    //Enable/Disable broadcast
    this->setBroadcastOption(broadcast);
}
SocketUdp::~SocketUdp()
{
    this->close();
}

fge::net::Socket::Error SocketUdp::create()
{
    if (this->g_socket == _FGE_SOCKET_INVALID)
    {
        //Create UDP socket
        this->g_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (this->g_socket == _FGE_SOCKET_INVALID)
        {//Check if valid
            return fge::net::NormalizeError();
        }
    }
    return fge::net::Socket::ERR_NOERROR;
}

fge::net::Socket::Error SocketUdp::connect(const fge::net::IpAddress& remoteAddress, fge::net::Port remotePort)
{
    // Create the remote address
    sockaddr_in addr{};
    addr.sin_addr.s_addr = remoteAddress.getNetworkByteOrder();
    addr.sin_family      = AF_INET;
    addr.sin_port        = fge::SwapHostNetEndian_16(remotePort);
    #ifdef _FGE_MACOS
        addr.sin_len = sizeof(addr);
    #endif

    if (::connect(this->g_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error SocketUdp::bind(fge::net::Port port, const IpAddress& address)
{
    // Close the socket if it is already bound
    close();

    // Create the internal socket if it doesn't exist
    create();

    // Check if the address is valid
    if ((address == fge::net::IpAddress::None) || (address == fge::net::IpAddress::Broadcast))
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    // Bind the socket
    sockaddr_in addr{};
    addr.sin_addr.s_addr = address.getNetworkByteOrder();
    addr.sin_family      = AF_INET;
    addr.sin_port        = fge::SwapHostNetEndian_16(port);
    #ifdef _FGE_MACOS
        addr.sin_len = sizeof(addr);
    #endif

    if (::bind(this->g_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    return fge::net::Socket::ERR_NOERROR;
}

fge::net::Socket::Error SocketUdp::sendTo(const void* data, std::size_t size, const IpAddress& remoteAddress, fge::net::Port remotePort)
{
    // Create the internal socket if it doesn't exist
    create();

    // Make sure that all the data will fit in one datagram
    if (size > FGE_SOCKET_MAXDATAGRAMSIZE)
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    // Build the target address
    sockaddr_in address{};
    address.sin_addr.s_addr = remoteAddress.getNetworkByteOrder();
    address.sin_family      = AF_INET;
    address.sin_port        = fge::SwapHostNetEndian_16(remotePort);
    #ifdef _FGE_MACOS
        address.sin_len = sizeof(addr);
    #endif

    // Send the data (unlike TCP, all the data is always sent in one call)
    int sent = sendto(this->g_socket, static_cast<const char*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG, reinterpret_cast<sockaddr*>(&address), sizeof(address));

    // Check for errors
    if (sent == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error SocketUdp::send(const void* data, std::size_t size)
{
    if ((data == nullptr) || (size == 0))
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    int sent = ::send(this->g_socket, static_cast<const char*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG);

    // Check for errors
    if (sent == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error SocketUdp::receiveFrom(void* data, std::size_t size, std::size_t& received, fge::net::IpAddress& remoteAddress, fge::net::Port& remotePort)
{
    // First clear the variables to fill
    received      = 0;
    remoteAddress = IpAddress();
    remotePort    = 0;

    // Check the destination buffer
    if (data == nullptr)
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    // Data that will be filled with the other computer's address
    sockaddr_in address{};
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = 0;
    #ifdef _FGE_MACOS
        address.sin_len = sizeof(addr);
    #endif

    // Receive a chunk of bytes
    fge::net::SocketLength addressSize = sizeof(address);
    int sizeReceived = recvfrom(this->g_socket, static_cast<char*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG, reinterpret_cast<sockaddr*>(&address), &addressSize);

    // Check for errors
    if (sizeReceived == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    // Fill the sender informations
    received      = static_cast<std::size_t>(sizeReceived);
    remoteAddress.setNetworkByteOrdered(address.sin_addr.s_addr);
    remotePort    = fge::SwapHostNetEndian_16(address.sin_port);

    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error SocketUdp::receive(void* data, std::size_t size, std::size_t& received)
{
    // First clear the variables to fill
    received = 0;

    // Check the destination buffer
    if (data == nullptr)
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    // Receive a chunk of bytes
    int sizeReceived = recv(this->g_socket, static_cast<char*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG);

    // Check for errors
    if (sizeReceived == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    // Fill the sender informations
    received = static_cast<std::size_t>(sizeReceived);

    return fge::net::Socket::ERR_NOERROR;
}

fge::net::Socket::Error SocketUdp::send(fge::net::Packet& packet)
{
    if (packet.getDataSize() == 0)
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    if (!packet._g_lastDataValidity)
    {
        packet.onSend(packet._g_lastData, 0);
        packet._g_sendPos = 0;
    }

    int sent = ::send(this->g_socket, reinterpret_cast<const char*>(packet._g_lastData.data()), static_cast<int>(packet._g_lastData.size()), _FGE_SEND_RECV_FLAG);

    // Check for errors
    if (sent == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error SocketUdp::sendTo(fge::net::Packet& packet, const IpAddress& remoteAddress, fge::net::Port remotePort)
{
    // Create the internal socket if it doesn't exist
    create();

    // Make sure that all the data will fit in one datagram
    if (packet.getDataSize() > FGE_SOCKET_MAXDATAGRAMSIZE)
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    // Build the target address
    sockaddr_in address{};
    address.sin_addr.s_addr = remoteAddress.getNetworkByteOrder();
    address.sin_family      = AF_INET;
    address.sin_port        = fge::SwapHostNetEndian_16(remotePort);
    #ifdef _FGE_MACOS
        address.sin_len = sizeof(addr);
    #endif

    if (!packet._g_lastDataValidity)
    {
        packet.onSend(packet._g_lastData, 0);
        packet._g_sendPos = 0;
    }

    // Send the data (unlike TCP, all the data is always sent in one call)
    int sent = sendto(this->g_socket, reinterpret_cast<const char*>(packet._g_lastData.data()), static_cast<int>(packet._g_lastData.size()), _FGE_SEND_RECV_FLAG, reinterpret_cast<sockaddr*>(&address), sizeof(address));

    // Check for errors
    if (sent == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error SocketUdp::receiveFrom(fge::net::Packet& packet, fge::net::IpAddress& remoteAddress, fge::net::Port& remotePort)
{
    size_t received = 0;
    fge::net::Socket::Error status = this->receiveFrom(this->g_buffer.data(), this->g_buffer.size(), received, remoteAddress, remotePort);

    packet.clear();
    if ((status == fge::net::Socket::ERR_NOERROR) && (received > 0))
    {
        packet.onReceive(this->g_buffer.data(), received);
    }
    return status;
}
fge::net::Socket::Error SocketUdp::receive(fge::net::Packet& packet)
{
    size_t received = 0;
    fge::net::Socket::Error status = this->receive(this->g_buffer.data(), this->g_buffer.size(), received);

    packet.clear();
    if ((status == fge::net::Socket::ERR_NOERROR) && (received > 0))
    {
        packet.onReceive(this->g_buffer.data(), received);
    }
    return status;
}

fge::net::SocketUdp& SocketUdp::operator=(fge::net::SocketUdp&& r) noexcept
{
    this->g_isBlocking = r.g_isBlocking;
    this->g_type = r.g_type;
    this->g_socket = r.g_socket;
    r.g_socket = _FGE_SOCKET_INVALID;
    return *this;
}

///SocketTcp

SocketTcp::SocketTcp() :
    Socket(fge::net::Socket::TYPE_TCP),
    g_receivedSize(0),
    g_wantedSize(0),
    g_buffer(FGE_SOCKET_TCP_DEFAULT_BUFFERSIZE)
{
    //Create TCP socket
    this->create();

    //Set the blocking state to false by default
    this->setBlocking(false);
}
SocketTcp::SocketTcp(bool blocking) :
    Socket(fge::net::Socket::TYPE_TCP),
    g_receivedSize(0),
    g_wantedSize(0),
    g_buffer(FGE_SOCKET_TCP_DEFAULT_BUFFERSIZE)
{
    //Create TCP socket
    this->create();

    //Set the blocking state
    this->setBlocking(blocking);
}
SocketTcp::~SocketTcp()
{
    this->close();
}

void SocketTcp::flush()
{
    this->g_receivedSize = 0;
    this->g_wantedSize = 0;
    this->g_buffer.clear();
    this->g_buffer.resize(FGE_SOCKET_TCP_DEFAULT_BUFFERSIZE);
}

fge::net::Socket::Error SocketTcp::create(fge::net::Socket::SocketDescriptor sck)
{
    if (sck == _FGE_SOCKET_INVALID)
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    this->close();

    this->g_receivedSize = 0;
    this->g_wantedSize = 0;

    //Create TCP socket
    this->g_socket = sck;

    //Disable the Nagle algorithm
    const char optval = 1;
    if (setsockopt(this->g_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    // On Mac OS X, disable the SIGPIPE signal on disconnection
    #ifdef _FGE_MACOS
        if (setsockopt(this->g_socket, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
        {
            return fge::net::NormalizeError();
        }
    #endif

    return this->setBlocking(this->g_isBlocking);
}
fge::net::Socket::Error SocketTcp::create()
{
    if (this->g_socket == _FGE_SOCKET_INVALID)
    {
        //Create TCP socket
        this->g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (this->g_socket == _FGE_SOCKET_INVALID)
        {//Check if valid
            return fge::net::NormalizeError();
        }

        //Disable the Nagle algorithm
        const char optval = 1;
        if (setsockopt(this->g_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
        {
            return fge::net::NormalizeError();
        }

        // On Mac OS X, disable the SIGPIPE signal on disconnection
        #ifdef _FGE_MACOS
            if (setsockopt(this->g_socket, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
            {
                return fge::net::NormalizeError();
            }
        #endif
    }
    return fge::net::Socket::ERR_NOERROR;
}

fge::net::Socket::Error SocketTcp::connect(const fge::net::IpAddress& remoteAddress, fge::net::Port remotePort, uint32_t timeoutms)
{
    // Disconnect the socket if it is already connected
    this->close();

    this->g_receivedSize = 0;
    this->g_wantedSize = 0;

    // Create the internal socket if it doesn't exist
    this->create();

    // Create the remote address
    sockaddr_in address{};
    address.sin_addr.s_addr = remoteAddress.getNetworkByteOrder();
    address.sin_family      = AF_INET;
    address.sin_port        = fge::SwapHostNetEndian_16(remotePort);
    #ifdef _FGE_MACOS
        address.sin_len = sizeof(addr);
    #endif

    if (timeoutms == 0)
    {
        // Connect the socket
        if (::connect(this->g_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == _FGE_SOCKET_ERROR)
        {
            return fge::net::NormalizeError();
        }

        // Connection succeeded
        return fge::net::Socket::ERR_NOERROR;
    }
    // Save the previous blocking state
    bool blocking = this->isBlocking();

    // Switch to non-blocking to enable our connection timeout
    if (blocking)
    {
        this->setBlocking(false);
    }

    // Try to connect to the remote address
    if (::connect(this->g_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != _FGE_SOCKET_ERROR)
    {
        // We got instantly connected! (it may no happen a lot...)
        this->setBlocking(blocking);
        return fge::net::Socket::ERR_NOERROR;
    }

    // Get the error status
    fge::net::Socket::Error status = fge::net::NormalizeError();

    // If we were in non-blocking mode, return immediately
    if (!blocking)
    {
        return status;
    }

    // Otherwise, wait until something happens to our socket (success, timeout or error)
    if (status == fge::net::Socket::ERR_NOTREADY)
    {
        // Setup the selector
        fd_set selector;
        #ifdef _win32
        selector.fd_count = 1;
        *selector.fd_array = this->g_socket;
        #else
        FD_ZERO(&selector);
        FD_SET(this->g_socket, &selector);
        #endif // _win32

        // Setup the timeout
        timeval time{};
        time.tv_sec  = static_cast<long>(timeoutms / 1000);
        time.tv_usec = static_cast<long>((timeoutms%1000) * 1000);

        // Wait for something to write on our socket (which means that the connection request has returned)
        if (::select(static_cast<int>(this->g_socket + 1), nullptr, &selector, nullptr, &time) == 1)
        {
            // Socket selected for write, Check for errors
            int32_t optval;
            fge::net::SocketLength optlen = sizeof(optval);

            if (getsockopt(this->g_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen) == _FGE_SOCKET_ERROR)
            {
                return fge::net::NormalizeError();
            }
            // Check the value returned...
            if (optval != 0)
            {
                return fge::net::NormalizeError(optval);
            }
            status = fge::net::Socket::ERR_NOERROR;
        }
        else
        {
            // Failed to connect before timeout is over
            status = fge::net::NormalizeError();
        }
    }

    // Switch back to blocking mode
    this->setBlocking(true);

    return status;
}

fge::net::Socket::Error SocketTcp::send(const void* data, std::size_t size)
{
    std::size_t sent;
    return this->send(data, size, sent);
}
fge::net::Socket::Error SocketTcp::send(const void* data, std::size_t size, std::size_t& sent)
{
    // Check the parameters
    if ((data == nullptr) || (size == 0))
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    // Loop until every byte has been sent
    int result = 0;
    for (sent = 0; sent < size; sent += result)
    {
        // Send a chunk of data
        result = ::send(this->g_socket, static_cast<const char*>(data) + sent, static_cast<int>(size - sent), _FGE_SEND_RECV_FLAG);

        // Check for errors
        if (result == _FGE_SOCKET_ERROR)
        {
            fge::net::Socket::Error status = fge::net::NormalizeError();

            if ((status == fge::net::Socket::ERR_NOTREADY) && (sent > 0))
            {
                return fge::net::Socket::ERR_PARTIAL;
            }
            return status;
        }
    }

    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error SocketTcp::receive(void* data, std::size_t size, std::size_t& received)
{
    // First clear the variables to fill
    received = 0;

    // Check the destination buffer
    if (data == nullptr)
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    // Receive a chunk of bytes
    int sizeReceived = recv(this->g_socket, static_cast<char*>(data), static_cast<int>(size), _FGE_SEND_RECV_FLAG);

    // Check the number of bytes received
    if (sizeReceived > 0)
    {
        received = static_cast<std::size_t>(sizeReceived);
        return fge::net::Socket::ERR_NOERROR;
    }
    else if (sizeReceived == 0)
    {
        return fge::net::Socket::ERR_DISCONNECTED;
    }
    else
    {
        return fge::net::NormalizeError();
    }
}

fge::net::Socket::Error SocketTcp::send(fge::net::Packet& packet)
{
    if ( !packet._g_lastDataValidity )
    {// New packet that gonna be sent
        packet.onSend(packet._g_lastData, sizeof(uint32_t));
        *reinterpret_cast<uint32_t*>(packet._g_lastData.data()) = fge::SwapHostNetEndian_32(packet._g_lastData.size());
        packet._g_sendPos = 0;
    }

    // Send the data block
    std::size_t sent;
    fge::net::Socket::Error status = this->send(packet._g_lastData.data() + packet._g_sendPos, packet._g_lastData.size() - packet._g_sendPos, sent);

    // In the case of a partial send, record the location to resume from
    if (status == fge::net::Socket::ERR_PARTIAL)
    {
        packet._g_sendPos += sent;
    }
    else if (status == fge::net::Socket::ERR_NOERROR)
    {
        packet._g_sendPos = 0;
    }

    return status;
}
fge::net::Socket::Error SocketTcp::receive(fge::net::Packet& packet)
{
    if (this->g_receivedSize == 0)
    {// New packet is here
        std::size_t received;

        this->g_buffer.resize(sizeof(uint32_t));
        fge::net::Socket::Error status = this->receive(this->g_buffer.data(), sizeof(uint32_t), received);

        if (received == 0)
        {
            return status;
        }

        this->g_receivedSize += received;

        if (this->g_receivedSize >= sizeof(uint32_t))
        {
            this->g_wantedSize = fge::SwapHostNetEndian_32( *reinterpret_cast<uint32_t*>(this->g_buffer.data()) );
            if (this->g_wantedSize == 0)
            {// Received a bad size
                this->g_receivedSize = 0;
                this->g_wantedSize = 0;
                return fge::net::Socket::ERR_UNSUCCESS;
            }
            this->g_buffer.resize(this->g_wantedSize + sizeof(uint32_t));
        }

        return fge::net::Socket::ERR_PARTIAL;
    }
    else
    {// Already on a pending packet
        if (this->g_wantedSize > 0)
        {// We have already the wanted size
            std::size_t received;
            fge::net::Socket::Error status = this->receive(this->g_buffer.data() + this->g_receivedSize, this->g_buffer.size() - this->g_receivedSize, received);

            if (received == 0)
            {
                return status;
            }

            this->g_receivedSize += received;

            if (this->g_wantedSize == this->g_receivedSize)
            {// Well we finished this pending packet
                packet.clear();
                packet.onReceive(this->g_buffer.data()+sizeof(uint32_t), this->g_wantedSize-sizeof(uint32_t));
                this->g_receivedSize = 0;
                this->g_wantedSize = 0;
                return fge::net::Socket::ERR_DONE;
            }

            return fge::net::Socket::ERR_PARTIAL;
        }
        else
        {// We don't have the wanted size
            std::size_t received;
            fge::net::Socket::Error status = this->receive(this->g_buffer.data() + this->g_receivedSize, sizeof(uint32_t) - this->g_receivedSize, received);

            if (received == 0)
            {
                return status;
            }

            this->g_receivedSize += received;

            if (this->g_receivedSize >= sizeof(uint32_t))
            {
                this->g_wantedSize = fge::SwapHostNetEndian_32( *reinterpret_cast<uint32_t*>(this->g_buffer.data()) );
                if (this->g_wantedSize == 0)
                {// Received a bad size
                    this->g_receivedSize = 0;
                    this->g_wantedSize = 0;
                    return fge::net::Socket::ERR_UNSUCCESS;
                }
                this->g_buffer.resize(this->g_wantedSize + sizeof(uint32_t));
            }

            return fge::net::Socket::ERR_PARTIAL;
        }
    }
}

fge::net::Socket::Error SocketTcp::sendAndReceive(fge::net::Packet& sendPacket, fge::net::Packet& receivePacket, uint32_t timeoutms)
{
    fge::net::Socket::Error error = this->send(sendPacket);
    if (error == fge::net::Socket::ERR_NOERROR)
    {
        error = this->select(true, timeoutms);
        if (error == fge::net::Socket::ERR_NOERROR)
        {
            return this->receive(receivePacket);
        }

    }
    return error;
}
fge::net::Socket::Error SocketTcp::receive(fge::net::Packet& packet, uint32_t timeoutms)
{
    fge::net::Socket::Error error = this->select(true, timeoutms);
    if (error == fge::net::Socket::ERR_NOERROR)
    {
        return this->receive(packet);
    }
    return error;
}

fge::net::SocketTcp& SocketTcp::operator=(fge::net::SocketTcp&& r) noexcept
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

SocketListenerTcp::SocketListenerTcp() :
    Socket(fge::net::Socket::TYPE_LISTENER_TCP)
{
    //Create TCP socket
    this->create();

    //Set the blocking state to false by default
    this->setBlocking(false);
}
SocketListenerTcp::SocketListenerTcp(bool blocking) :
    Socket(fge::net::Socket::TYPE_LISTENER_TCP)
{
    //Create TCP socket
    this->create();

    //Set the blocking state
    this->setBlocking(blocking);
}
SocketListenerTcp::~SocketListenerTcp()
{
    this->close();
}

fge::net::Socket::Error SocketListenerTcp::create()
{
    if (this->g_socket == _FGE_SOCKET_INVALID)
    {
        //Create TCP socket
        this->g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (this->g_socket == _FGE_SOCKET_INVALID)
        {//Check if valid
            return fge::net::NormalizeError();
        }

        //Disable the Nagle algorithm
        const char optval = 1;
        if (setsockopt(this->g_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
        {
            return fge::net::NormalizeError();
        }

        // On Mac OS X, disable the SIGPIPE signal on disconnection
        #ifdef _FGE_MACOS
            if (setsockopt(this->g_socket, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) == _FGE_SOCKET_ERROR)
            {
                return fge::net::NormalizeError();
            }
        #endif
    }
    return fge::net::Socket::ERR_NOERROR;
}

fge::net::Socket::Error SocketListenerTcp::listen(fge::net::Port port, const fge::net::IpAddress& address)
{
    // Close the socket if it is already bound
    this->close();

    // Create the internal socket if it doesn't exist
    this->create();

    // Check if the address is valid
    if ((address == fge::net::IpAddress::None) || (address == fge::net::IpAddress::Broadcast))
    {
        return fge::net::Socket::ERR_INVALIDARGUMENT;
    }

    // Bind the socket to the specified port
    sockaddr_in addr{};
    addr.sin_addr.s_addr = address.getNetworkByteOrder();
    addr.sin_family      = AF_INET;
    addr.sin_port        = fge::SwapHostNetEndian_16(port);
    #ifdef _FGE_MACOS
        addr.sin_len = sizeof(addr);
    #endif

    if (bind(this->g_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    // Listen to the bound port
    if (::listen(this->g_socket, SOMAXCONN) == _FGE_SOCKET_ERROR)
    {
        return fge::net::NormalizeError();
    }

    return fge::net::Socket::ERR_NOERROR;
}
fge::net::Socket::Error SocketListenerTcp::accept(fge::net::SocketTcp& socket)
{
    // Make sure that we're listening
    if (this->g_socket == _FGE_SOCKET_INVALID)
    {
        return fge::net::Socket::ERR_DISCONNECTED;
    }

    // Accept a new connection
    sockaddr_in address{};
    fge::net::SocketLength length = sizeof(address);
    fge::net::Socket::SocketDescriptor remote = ::accept(this->g_socket, reinterpret_cast<sockaddr*>(&address), &length);

    // Check for errors
    if (remote == _FGE_SOCKET_INVALID)
    {
        return fge::net::NormalizeError();
    }

    // Initialize the new connected socket
    socket.close();
    socket.create(remote);

    return fge::net::Socket::ERR_NOERROR;
}

fge::net::SocketListenerTcp& SocketListenerTcp::operator=(fge::net::SocketListenerTcp&& r) noexcept
{
    this->g_isBlocking = r.g_isBlocking;
    this->g_type = r.g_type;
    this->g_socket = r.g_socket;
    r.g_socket = _FGE_SOCKET_INVALID;
    return *this;
}

}//end fge::net
