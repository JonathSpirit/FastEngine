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

#define FGE_SOCKET_ETHERNET_MTU 1500
#define FGE_SOCKET_IPV4_MIN_MTU 576
#define FGE_SOCKET_IPV6_MIN_MTU 1280
#define FGE_SOCKET_IPV4_HEADER_SIZE 20
#define FGE_SOCKET_IPV6_HEADER_SIZE 40
#define FGE_SOCKET_UDP_HEADER_SIZE 8

#define FGE_SOCKET_FULL_DATAGRAM_SIZE (0xFFFF)
#define FGE_SOCKET_IPV4_MAX_DATAGRAM_SIZE                                                                              \
    (FGE_SOCKET_FULL_DATAGRAM_SIZE - FGE_SOCKET_IPV4_HEADER_SIZE - FGE_SOCKET_UDP_HEADER_SIZE)
#define FGE_SOCKET_IPV6_MAX_DATAGRAM_SIZE                                                                              \
    (FGE_SOCKET_FULL_DATAGRAM_SIZE - FGE_SOCKET_IPV6_HEADER_SIZE - FGE_SOCKET_UDP_HEADER_SIZE)
#define FGE_SOCKET_IPV4_MAX_DATAGRAM_MTU_SIZE                                                                          \
    (FGE_SOCKET_IPV4_MIN_MTU - FGE_SOCKET_IPV4_HEADER_SIZE - FGE_SOCKET_UDP_HEADER_SIZE)
#define FGE_SOCKET_IPV6_MAX_DATAGRAM_MTU_SIZE                                                                          \
    (FGE_SOCKET_IPV6_MIN_MTU - FGE_SOCKET_IPV6_HEADER_SIZE - FGE_SOCKET_UDP_HEADER_SIZE)
#define FGE_SOCKET_IPV4_MAX_DATAGRAM_ETHMTU_SIZE                                                                       \
    (FGE_SOCKET_ETHERNET_MTU - FGE_SOCKET_IPV4_HEADER_SIZE - FGE_SOCKET_UDP_HEADER_SIZE)
#define FGE_SOCKET_IPV6_MAX_DATAGRAM_ETHMTU_SIZE                                                                       \
    (FGE_SOCKET_ETHERNET_MTU - FGE_SOCKET_IPV6_HEADER_SIZE - FGE_SOCKET_UDP_HEADER_SIZE)

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
     * \enum Types
     * \brief The possible types of sockets
     */
    enum class Types
    {
        UDP,
        TCP,
        TCP_LISTENER,
        UNKNOWN
    };

    /**
     * \enum Errors
     * \brief The error codes
     */
    enum class Errors
    {
        ERR_NOERROR = 0,
        ERR_SUCCESS = ERR_NOERROR,
        ERR_DONE = ERR_NOERROR,

        ERR_PARTIAL = 1,
        ERR_NOTREADY = 2,
        ERR_DISCONNECTED = 3,
        ERR_REFUSED = 4,

        ERR_ALREADYCONNECTED = 5,
        ERR_ALREADYUSED = 6,
        ERR_TOOMANYSOCKET = 7,

        ERR_NOTINIT = 8,

        ERR_INVALIDARGUMENT = 9,

        ERR_UNSUCCESS = 10,
        ERR_UNKNOWN = ERR_UNSUCCESS
    };

    struct AdapterInfo
    {
        struct Data
        {
            IpAddress _unicast;
        };

        std::string _name;
        std::string _description;
        uint16_t _mtu;
        std::vector<Data> _data;
    };

    /**
     * \brief Get the type of the socket
     *
     * Type can be one of the type defined in the Socket::Type enum
     *
     * \return The type of the socket
     */
    [[nodiscard]] inline Types getType() const { return this->g_type; }
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
    virtual Errors create() = 0;
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
    [[nodiscard]] Port getLocalPort() const;
    /**
     * \brief Get the local address of the socket
     *
     * From MSDN:
     * This function provide the only way to determine the local association that has been set by the system.
     *
     * \return The local address of the socket or IpAddress::None if there was an error
     */
    [[nodiscard]] IpAddress getLocalAddress() const;
    /**
     * \brief Get the remote port of the socket
     *
     * From MSDN:
     * For datagram sockets, only the address of a peer specified in a previous connect call will be returned.
     * Any address specified by a previous sendto call will not be returned.
     *
     * \return The remote port of the socket or 0 if there was an error
     */
    [[nodiscard]] Port getRemotePort() const;
    /**
     * \brief Get the remote address of the socket
     *
     * From MSDN:
     * For datagram sockets, only the address of a peer specified in a previous connect call will be returned.
     * Any address specified by a previous sendto call will not be returned.
     *
     * \return The remote address of the socket or IpAddress::None if there was an error
     */
    [[nodiscard]] IpAddress getRemoteAddress() const;

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
    Errors setBlocking(bool mode);
    /**
     * \brief Set if the socket reuse the address
     *
     * From MSDN:
     * The SO_REUSEADDR socket option allows the application to bind to an address that is already in use.
     *
     * \param mode The reuse mode to set
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors setReuseAddress(bool mode);
    /**
     * \brief Set if the socket support broadcast
     *
     * From MSDN:
     * The SO_BROADCAST socket option enables the socket to send and receive broadcast messages.
     *
     * \param mode The broadcast mode to set
     * \return Errors::ERR_NOERROR if successful, otherwise an error code
     */
    Errors setBroadcastOption(bool mode);
    /**
     * \brief Set if ipv6 socket should only use ipv6
     *
     * From MSDN:
     * The IPV6_V6ONLY socket option indicates if a socket created for the AF_INET6 address family is restricted to IPv6 communications only.
     *
     * \param mode The ipv6 only mode to set
     * \return Errors::ERR_NOERROR if successful, otherwise an error code
     */
    Errors setIpv6Only(bool mode);
    /**
     * \brief Set if the socket should append DF flag to the packet
     *
     * From MSDN: (IP_DONTFRAGMENT)
     * Indicates that data should not be fragmented regardless of the local MTU.
     * Valid only for message oriented protocols. Microsoft TCP/IP providers respect this option for UDP and ICMP.
     *
     * Note that on GNU/Linux, this option is IP_MTU_DISCOVER with IP_PMTUDISC_DO and IP_PMTUDISC_DONT flags.
     *
     * \param mode If the socket should append the DF flag
     * \return Errors::ERR_NOERROR if successful, otherwise an error code
     */
    Errors setDontFragment(bool mode);

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
     * \return Errors::ERR_NOERROR if the socket is ready, otherwise an error code
     */
    Errors select(bool read, uint32_t timeoutms);

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
     * \brief Retrieve the current adapter MTU
     *
     * From MSDN:
     * This function get all the adapter addresses and compare the local address with the current local address in
     * order to find the adapter that is currently used.
     *
     * From a connection-less socket, this function can only work if the socket is connected to a remote address or/and
     * if I/O operations have been performed.
     *
     * \return The current adapter MTU or \b std::nullopt if an error occurred
     */
    [[nodiscard]] std::optional<uint16_t> retrieveCurrentAdapterMTU() const;

    /**
     * \brief Retrieve adapters information
     *
     * The information retrieved is the name, description, mtu and unicast addresses of the adapters.
     *
     * \param type The type of address to get, if None, all addresses will be returned
     * \return A vector containing the adapters information or an empty vector
     */
    [[nodiscard]] static std::vector<AdapterInfo> getAdaptersInfo(IpAddress::Types type = IpAddress::Types::None);

    /**
     * \brief Get the last platform specific error code
     *
     * \return The last platform specific error code
     */
    [[nodiscard]] static int getPlatformSpecifiedError();

    Socket& operator=(Socket const& r) = delete;
    Socket(Socket const& r) = delete;

protected:
    explicit Socket(Types type, IpAddress::Types addressType = IpAddress::Types::Ipv4);
    virtual ~Socket();

    Types g_type;
    IpAddress::Types g_addressType{IpAddress::Types::Ipv4};
    SocketDescriptor g_socket;
    bool g_isBlocking;
};

/**
 * \class SocketUdp
 * \ingroup network
 * \brief A wrapper for UDP sockets inheriting from Socket
 */
class FGE_API SocketUdp : public Socket
{
public:
    explicit SocketUdp(IpAddress::Types addressType = IpAddress::Types::Ipv4);
    SocketUdp(IpAddress::Types addressType, bool blocking, bool broadcast);
    ~SocketUdp() override = default;

    Errors create() override;

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
    Errors connect(IpAddress const& remoteAddress, Port remotePort);
    Errors disconnect();
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
    Errors bind(Port port, IpAddress const& address);

    /**
     * \brief Send data to the connected remote address
     *
     * The connect function must be called before calling this function.
     *
     * \see SocketUdp::connect
     *
     * \param data The data to send
     * \param size The size of the data to send
     * \return Errors::ERR_NOERROR if successful, otherwise an error code
     */
    Errors send(void const* data, std::size_t size);
    /**
     * \brief Send data to the specified address
     *
     * \param data The data to send
     * \param size The size of the data to send
     * \param remoteAddress The remote address to send to
     * \param remotePort The remote port to send to
     * \return Errors::ERR_NOERROR if successful, otherwise an error code
     */
    Errors sendTo(void const* data, std::size_t size, IpAddress const& remoteAddress, Port remotePort);
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
    Errors receiveFrom(void* data, std::size_t size, std::size_t& received, IpAddress& remoteAddress, Port& remotePort);
    /**
     * \brief Receive data from the connected remote address
     *
     * The connect function must be called before calling this function.
     *
     * \see SocketUdp::connect
     *
     * \param data The data buffer to receive into
     * \param size The size of the data buffer
     * \param received The number of bytes received
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors receive(void* data, std::size_t size, std::size_t& received);

    /**
     * \brief Send a Packet to the connected remote address
     *
     * The connect function must be called before calling this function.
     *
     * \see SocketUdp::connect
     * \see Packet
     *
     * \param packet The packet to send
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors send(Packet& packet);
    /**
     * \brief Send a Packet to the specified address
     *
     * \param packet The packet to send
     * \param remoteAddress The remote address to send to
     * \param remotePort The remote port to send to
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors sendTo(Packet& packet, IpAddress const& remoteAddress, Port remotePort);
    /**
     * \brief Receive a Packet from an unspecified remote address
     *
     * \param packet The packet to receive into
     * \param remoteAddress The remote address of the sender
     * \param remotePort The remote port of the sender
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors receiveFrom(Packet& packet, IpAddress& remoteAddress, Port& remotePort);
    /**
     * \brief Receive a Packet from the connected remote address
     *
     * The connect function must be called before calling this function.
     *
     * \see SocketUdp::connect
     *
     * \param packet The packet to receive into
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors receive(Packet& packet);

    /**
     * \brief Helper to retrieve the MTU of the adapter used to reach the destination ip address
     *
     * This function will create a temporary socket, bind it to any port and connect it to the destination address.
     *
     * \param destination The destination address to reach
     * \return The MTU of the adapter used to reach the destination address or \b std::nullopt if an error occurred
     */
    [[nodiscard]] static std::optional<uint16_t> retrieveAdapterMTUForDestination(IpAddress const& destination);

    SocketUdp& operator=(SocketUdp&& r) noexcept;

private:
    std::vector<uint8_t> g_buffer;
};

/**
 * \class SocketTcp
 * \ingroup network
 * \brief A wrapper for TCP sockets inheriting from Socket
 */
class FGE_API SocketTcp : public Socket
{
public:
    explicit SocketTcp(IpAddress::Types addressType = IpAddress::Types::Ipv4);
    explicit SocketTcp(IpAddress::Types addressType, bool blocking);
    ~SocketTcp() override = default;

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
    Errors create(SocketDescriptor sck);
    /**
     * \brief Create a socket
     *
     * by default, the nagle algorithm is disabled.
     * On Mac OS X, disable the SIGPIPE signal on disconnection.
     *
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors create() override;

    /**
     * \brief Connect to a remote address
     *
     * \param remoteAddress The remote address to connect to
     * \param remotePort The remote port to connect to
     * \param timeoutms The timeout in milliseconds
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors connect(IpAddress const& remoteAddress, Port remotePort, uint32_t timeoutms);

    /**
     * \brief Send data to the connected remote address
     *
     * \param data The data to send
     * \param size The size of the data to send
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors send(void const* data, std::size_t size);
    /**
     * \brief Send data to the connected remote address
     *
     * \param data The data to send
     * \param size The size of the data to send
     * \param sent The number of bytes sent
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors send(void const* data, std::size_t size, std::size_t& sent);
    /**
     * \brief Receive data from the connected remote address
     *
     * \param data The data buffer to receive into
     * \param size The size of the data buffer
     * \param received The number of bytes received
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors receive(void* data, std::size_t size, std::size_t& received);
    Errors receive(void* data, std::size_t size, std::size_t& received, uint32_t timeoutms);

    /**
     * \brief Send a Packet to the connected remote address
     *
     * \param packet The packet to send
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors send(Packet& packet);
    /**
     * \brief Receive a Packet from the connected remote address
     *
     * If this function return Socket::ERR_PARTIAL then the packet
     * is not complete, and you should call this function again to receive the rest of the data.
     *
     * \param packet The packet to receive into
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors receive(Packet& packet);

    /**
     * \brief Utility function to send and receive data
     *
     * \param sendPacket The packet to send
     * \param receivePacket The packet to receive into
     * \param timeoutms The timeout in milliseconds
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors sendAndReceive(Packet& sendPacket, Packet& receivePacket, uint32_t timeoutms);
    /**
     * \brief Receive a packet from the connected remote address with a timeout
     *
     * \param packet The packet to receive into
     * \param timeoutms The timeout in milliseconds
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors receive(Packet& packet, uint32_t timeoutms);

    SocketTcp& operator=(SocketTcp&& r) noexcept;

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
class FGE_API SocketListenerTcp : public Socket
{
public:
    explicit SocketListenerTcp(IpAddress::Types addressType = IpAddress::Types::Ipv4);
    explicit SocketListenerTcp(IpAddress::Types addressType, bool blocking);
    ~SocketListenerTcp() override = default;

    /**
     * \brief Create the socket listener
     *
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors create() override;

    /**
     * \brief Start listening for new connections from a port
     *
     * \param port The port to listen on
     * \param address The address to listen on or IpAddress::Any
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors listen(Port port, IpAddress const& address);
    /**
     * \brief Accept a new connection
     *
     * \param socket The socket to accept the connection into
     * \return Error::ERR_NOERROR if successful, otherwise an error code
     */
    Errors accept(SocketTcp& socket);

    SocketListenerTcp& operator=(SocketListenerTcp&& r) noexcept;
};

} // namespace fge::net

#endif // _FGE_C_SOCKET_HPP_INCLUDED_
