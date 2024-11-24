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

#ifndef _FGE_C_SOCKET_HPP_INCLUDED_
#define _FGE_C_SOCKET_HPP_INCLUDED_

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/network/C_ipAddress.hpp"
#include <cstdint>
#include <vector>

#define FGE_SOCKET_MAXDATAGRAMSIZE 65507
#define FGE_SOCKET_TCP_DEFAULT_BUFFERSIZE 2048

namespace fge::net
{

class Packet;

/**
 * \class Socket
 * \ingroup network
 * \brief A base class wrapper for low-level network functions
 */
class FGE_API Socket
{
public:
#ifdef _WIN32
    #if defined(_WIN64)
    using SocketDescriptor = uint64_t;
    #else
    using SocketDescriptor = unsigned int;
    #endif
#else
    using SocketDescriptor = int;
#endif

    /**
     * \enum Type
     * \brief The possible types of sockets
     */
    enum Type
    {
        TYPE_UDP,
        TYPE_TCP,
        TYPE_LISTENER_TCP
    };

    /**
     * \enum Error
     * \brief The error codes
     */
    enum Error
    {
        ERR_NOERROR = 0,
        ERR_SUCCESS = ERR_NOERROR,
        ERR_DONE = ERR_NOERROR,

        ERR_PARTIAL,
        ERR_NOTREADY,
        ERR_DISCONNECTED,
        ERR_REFUSED,

        ERR_ALREADYCONNECTED,
        ERR_ALREADYUSED,
        ERR_TOOMANYSOCKET,

        ERR_NOTINIT,

        ERR_INVALIDARGUMENT,

        ERR_UNSUCCESS,
        ERR_UNKNOWN = ERR_UNSUCCESS
    };

    /**
     * \brief Get the type of the socket
     *
     * Type can be one of the type defined in the Socket::Type enum
     *
     * \return The type of the socket
     */
    [[nodiscard]] inline Type getType() const { return this->g_type; }
    /**
     * \brief Get the address type of the socket
     *
     * By default, the address type is Ipv4.
     *
     * \return The address type of the socket (Ipv4 or Ipv6)
     */
    [[nodiscard]] inline IpAddress::Types getAddressType() const { return this->g_addressType; }
    /**
     * \brief Set the address type of the socket
     *
     * When the address type is changed, the socket is closed and recreated.
     *
     * \param type The address type of the socket (Ipv4 or Ipv6)
     */
    void setAddressType(IpAddress::Types type);

    /**
     * \brief Create a new socket
     *
     * \return If successful, Error::ERR_NOERROR is returned, otherwise an error code is returned
     */
    virtual Error create() = 0;
    /**
     * \brief Close the socket
     */
    void close();
    /**
     * \brief Check if the socket is valid
     *
     * The socket is valid if it has been created and not closed
     *
     * \return \b true if the socket is valid, \b false otherwise
     */
    [[nodiscard]] bool isValid() const;

    /**
     * \brief Get the local port of the socket
     *
     * From MSDN:
     * This function provide the only way to determine the local association that has been set by the system.
     *
     * \return The local port of the socket or 0 if there was an error
     */
    [[nodiscard]] fge::net::Port getLocalPort() const;
    /**
     * \brief Get the local address of the socket
     *
     * From MSDN:
     * This function provide the only way to determine the local association that has been set by the system.
     *
     * \return The local address of the socket or fge::net::IpAddress::None if there was an error
     */
    [[nodiscard]] fge::net::IpAddress getLocalAddress() const;
    /**
     * \brief Get the remote port of the socket
     *
     * From MSDN:
     * For datagram sockets, only the address of a peer specified in a previous connect call will be returned.
     * Any address specified by a previous sendto call will not be returned.
     *
     * \return The remote port of the socket or 0 if there was an error
     */
    [[nodiscard]] fge::net::Port getRemotePort() const;
    /**
     * \brief Get the remote address of the socket
     *
     * From MSDN:
     * For datagram sockets, only the address of a peer specified in a previous connect call will be returned.
     * Any address specified by a previous sendto call will not be returned.
     *
     * \return The remote address of the socket or fge::net::IpAddress::None if there was an error
     */
    [[nodiscard]] fge::net::IpAddress getRemoteAddress() const;

    /**
     * \brief Check if the socket is in blocking mode
     *
     * \return \b true if the socket is in blocking mode, \b false otherwise
     */
    [[nodiscard]] bool isBlocking() const;

    /**
     * \brief Set the blocking mode of the socket
     *
     * \param mode The blocking mode to set
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Error setBlocking(bool mode);
    /**
     * \brief Set if the socket reuse the address
     *
     * From MSDN:
     * The SO_REUSEADDR socket option allows the application to bind to an address that is already in use.
     *
     * \param mode The reuse mode to set
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Error setReuseAddress(bool mode);
    /**
     * \brief Set if the socket support broadcast
     *
     * From MSDN:
     * The SO_BROADCAST socket option enables the socket to send and receive broadcast messages.
     *
     * \param mode The broadcast mode to set
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Error setBroadcastOption(bool mode);

    /**
     * \brief Check the socket for readability or writability
     *
     * From MSDN:
     * The select function allows an application to determine the status of a socket.
     *
     * If timeout is 0, the function returns immediately (polling).
     *
     * \param read \b true if the socket should be checked for readability, \b false otherwise
     * \param timeoutms The timeout in milliseconds
     * \return Error::ERR_NOERROR if the socket is ready, otherwise an error code
     */
    Error select(bool read, uint32_t timeoutms);

    /**
     * \brief Init the low-level socket library
     *
     * This call WSAStartup() on Windows and nothing on other platforms
     *
     * \return \b true if successful, \b false otherwise
     */
    static bool initSocket();
    /**
     * \brief Shutdown the low-level socket library
     */
    static void uninitSocket();

    /**
     * \brief Get the last platform specific error code
     *
     * \return The last platform specific error code
     */
    [[nodiscard]] static int getPlatformSpecifiedError();

    fge::net::Socket& operator=(fge::net::Socket const& r) = delete;
    Socket(fge::net::Socket const& r) = delete;

protected:
    explicit Socket(Type type, IpAddress::Types addressType = IpAddress::Types::Ipv4);
    virtual ~Socket() = default;

    Type g_type;
    IpAddress::Types g_addressType{IpAddress::Types::Ipv4};
    SocketDescriptor g_socket;
    bool g_isBlocking;
};

/**
 * \class SocketUdp
 * \ingroup network
 * \brief A wrapper for UDP sockets inheriting from Socket
 */
class FGE_API SocketUdp : public fge::net::Socket
{
public:
    explicit SocketUdp(IpAddress::Types addressType = IpAddress::Types::Ipv4);
    SocketUdp(IpAddress::Types addressType, bool blocking, bool broadcast);
    ~SocketUdp() override;

    fge::net::Socket::Error create() override;

    /**
     * \brief Connect the socket to a remote address and port
     *
     * From MSDN:
     * For a connectionless socket the operation performed by connect is merely to establish a default
     * destination address that can be used on subsequent send and receive calls.
     * Any datagrams received from an address other than the destination address specified will be discarded.
     *
     * \param remoteAddress The remote address to connect to
     * \param remotePort The remote port to connect to
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error connect(fge::net::IpAddress const& remoteAddress, fge::net::Port remotePort);
    /**
     * \brief Bind the socket to a local address and port
     *
     * From MSDN:
     * The bind function associates a local address and port with a socket.
     *
     * \param port The local port to bind to
     * \param address The local address to bind to
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error bind(fge::net::Port port, IpAddress const& address);

    /**
     * \brief Send data to the connected remote address
     *
     * The connect function must be called before calling this function.
     *
     * \see fge::net::SocketUdp::connect
     *
     * \param data The data to send
     * \param size The size of the data to send
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error send(void const* data, std::size_t size);
    /**
     * \brief Send data to the specified address
     *
     * \param data The data to send
     * \param size The size of the data to send
     * \param remoteAddress The remote address to send to
     * \param remotePort The remote port to send to
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error
    sendTo(void const* data, std::size_t size, IpAddress const& remoteAddress, fge::net::Port remotePort);
    /**
     * \brief Receive data from an unspecified remote address
     *
     * \param data The data buffer to receive into
     * \param size The size of the data buffer
     * \param received The number of bytes received
     * \param remoteAddress The remote address of the sender
     * \param remotePort The remote port of the sender
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error receiveFrom(void* data,
                                        std::size_t size,
                                        std::size_t& received,
                                        fge::net::IpAddress& remoteAddress,
                                        fge::net::Port& remotePort);
    /**
     * \brief Receive data from the connected remote address
     *
     * The connect function must be called before calling this function.
     *
     * \see fge::net::SocketUdp::connect
     *
     * \param data The data buffer to receive into
     * \param size The size of the data buffer
     * \param received The number of bytes received
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error receive(void* data, std::size_t size, std::size_t& received);

    /**
     * \brief Send a fge::net::Packet to the connected remote address
     *
     * The connect function must be called before calling this function.
     *
     * \see fge::net::SocketUdp::connect
     * \see fge::net::Packet
     *
     * \param packet The packet to send
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error send(fge::net::Packet& packet);
    /**
     * \brief Send a fge::net::Packet to the specified address
     *
     * \param packet The packet to send
     * \param remoteAddress The remote address to send to
     * \param remotePort The remote port to send to
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error sendTo(fge::net::Packet& packet, IpAddress const& remoteAddress, fge::net::Port remotePort);
    /**
     * \brief Receive a fge::net::Packet from an unspecified remote address
     *
     * \param packet The packet to receive into
     * \param remoteAddress The remote address of the sender
     * \param remotePort The remote port of the sender
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error
    receiveFrom(fge::net::Packet& packet, fge::net::IpAddress& remoteAddress, fge::net::Port& remotePort);
    /**
     * \brief Receive a fge::net::Packet from the connected remote address
     *
     * The connect function must be called before calling this function.
     *
     * \see fge::net::SocketUdp::connect
     *
     * \param packet The packet to receive into
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error receive(fge::net::Packet& packet);

    fge::net::SocketUdp& operator=(fge::net::SocketUdp&& r) noexcept;

private:
    std::vector<uint8_t> g_buffer;
};

/**
 * \class SocketTcp
 * \ingroup network
 * \brief A wrapper for TCP sockets inheriting from Socket
 */
class FGE_API SocketTcp : public fge::net::Socket
{
public:
    explicit SocketTcp(IpAddress::Types addressType = IpAddress::Types::Ipv4);
    explicit SocketTcp(IpAddress::Types addressType, bool blocking);
    ~SocketTcp() override;

    /**
     * \brief Flush the internal data buffer
     *
     * This function will discard all received data until now.
     */
    void flush();

    /**
     * \brief Create the socket with an existing descriptor
     *
     * the nagle algorithm will be disabled
     * On Mac OS X, disable the SIGPIPE signal on disconnection.
     *
     * \param sck The socket descriptor
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error create(fge::net::Socket::SocketDescriptor sck);
    /**
     * \brief Create a socket
     *
     * by default, the nagle algorithm is disabled.
     * On Mac OS X, disable the SIGPIPE signal on disconnection.
     *
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error create() override;

    /**
     * \brief Connect to a remote address
     *
     * \param remoteAddress The remote address to connect to
     * \param remotePort The remote port to connect to
     * \param timeoutms The timeout in milliseconds
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error
    connect(fge::net::IpAddress const& remoteAddress, fge::net::Port remotePort, uint32_t timeoutms);

    /**
     * \brief Send data to the connected remote address
     *
     * \param data The data to send
     * \param size The size of the data to send
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error send(void const* data, std::size_t size);
    /**
     * \brief Send data to the connected remote address
     *
     * \param data The data to send
     * \param size The size of the data to send
     * \param sent The number of bytes sent
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error send(void const* data, std::size_t size, std::size_t& sent);
    /**
     * \brief Receive data from the connected remote address
     *
     * \param data The data buffer to receive into
     * \param size The size of the data buffer
     * \param received The number of bytes received
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error receive(void* data, std::size_t size, std::size_t& received);
    fge::net::Socket::Error receive(void* data, std::size_t size, std::size_t& received, uint32_t timeoutms);

    /**
     * \brief Send a fge::net::Packet to the connected remote address
     *
     * \param packet The packet to send
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error send(fge::net::Packet& packet);
    /**
     * \brief Receive a fge::net::Packet from the connected remote address
     *
     * If this function return fge::net::Socket::ERR_PARTIAL then the packet
     * is not complete and you should call this function again to receive the rest of the data.
     *
     * \param packet The packet to receive into
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error receive(fge::net::Packet& packet);

    /**
     * \brief Utility function to send and receive data
     *
     * \param sendPacket The packet to send
     * \param receivePacket The packet to receive into
     * \param timeoutms The timeout in milliseconds
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error
    sendAndReceive(fge::net::Packet& sendPacket, fge::net::Packet& receivePacket, uint32_t timeoutms);
    /**
     * \brief Receive a packet from the connected remote address with a timeout
     *
     * \param packet The packet to receive into
     * \param timeoutms The timeout in milliseconds
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error receive(fge::net::Packet& packet, uint32_t timeoutms);

    fge::net::SocketTcp& operator=(fge::net::SocketTcp&& r) noexcept;

private:
    std::size_t g_receivedSize;
    std::size_t g_wantedSize;
    std::vector<uint8_t> g_buffer;
};

/**
 * \class SocketListenerTcp
 * \ingroup network
 * \brief A wrapper for TCP listener sockets inheriting from Socket
 */
class FGE_API SocketListenerTcp : public fge::net::Socket
{
public:
    explicit SocketListenerTcp(IpAddress::Types addressType = IpAddress::Types::Ipv4);
    explicit SocketListenerTcp(IpAddress::Types addressType, bool blocking);
    ~SocketListenerTcp() override;

    /**
     * \brief Create the socket listener
     *
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error create() override;

    /**
     * \brief Start listening for new connections from a port
     *
     * \param port The port to listen on
     * \param address The address to listen on or fge::net::IpAddress::Any
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error listen(fge::net::Port port, fge::net::IpAddress const& address);
    /**
     * \brief Accept a new connection
     *
     * \param socket The socket to accept the connection into
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    fge::net::Socket::Error accept(fge::net::SocketTcp& socket);

    fge::net::SocketListenerTcp& operator=(fge::net::SocketListenerTcp&& r) noexcept;
};

} // namespace fge::net

#endif // _FGE_C_SOCKET_HPP_INCLUDED_
