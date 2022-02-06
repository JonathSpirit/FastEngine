#ifndef _FGE_C_SOCKET_HPP_INCLUDED_
#define _FGE_C_SOCKET_HPP_INCLUDED_

#include <FastEngine/fastengine_extern.hpp>
#include "C_ipAddress.hpp"
#include <vector>
#include <cstdint>

#define FGE_SOCKET_MAXDATAGRAMSIZE 65507
#define FGE_SOCKET_TCP_DEFAULT_BUFFERSIZE 2048

namespace fge
{
namespace net
{

class Packet;

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

    enum Type
    {
        TYPE_UDP,
        TYPE_TCP,
        TYPE_LISTENER_TCP
    };

    enum Error
    {
        ERR_NOERROR = 0,
        ERR_SUCCESS = ERR_NOERROR,
        ERR_DONE    = ERR_NOERROR,

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

    inline fge::net::Socket::Type getType()
    {
        return this->g_type;
    }

    virtual fge::net::Socket::Error create() = 0;
    void close();
    bool isValid() const;

    fge::net::Port getLocalPort() const;
    fge::net::IpAddress getLocalAddress() const;
    fge::net::Port getRemotePort() const;
    fge::net::IpAddress getRemoteAddress() const;

    bool isBlocking() const;

    fge::net::Socket::Error setBlocking(bool mode);
    fge::net::Socket::Error setReuseAddress(bool mode);
    fge::net::Socket::Error setBroadcastOption(bool mode);

    fge::net::Socket::Error select(bool read, uint32_t timeoutms);

    static bool initSocket();
    static void uninitSocket();

    int getPlatformSpecifiedError() const;

    fge::net::Socket& operator=(const fge::net::Socket& r) = delete;

protected:
    Socket(fge::net::Socket::Type type);
    Socket(const fge::net::Socket& r) = delete;
    virtual ~Socket() = default;

    fge::net::Socket::Type g_type;
    fge::net::Socket::SocketDescriptor g_socket;
    bool g_isBlocking;
};

class FGE_API SocketUdp : public fge::net::Socket
{
public:
    SocketUdp();
    SocketUdp(bool blocking, bool broadcast);
    ~SocketUdp() override;

    fge::net::Socket::Error create() override;

    fge::net::Socket::Error connect(const fge::net::IpAddress& remoteAddress, fge::net::Port remotePort);
    fge::net::Socket::Error bind(fge::net::Port port, const IpAddress& address = fge::net::IpAddress::Any);

    fge::net::Socket::Error send(const void* data, std::size_t size);
    fge::net::Socket::Error sendTo(const void* data, std::size_t size, const IpAddress& remoteAddress, fge::net::Port remotePort);
    fge::net::Socket::Error receiveFrom(void* data, std::size_t size, std::size_t& received, fge::net::IpAddress& remoteAddress, fge::net::Port& remotePort);
    fge::net::Socket::Error receive(void* data, std::size_t size, std::size_t& received);

    fge::net::Socket::Error send(fge::net::Packet& packet);
    fge::net::Socket::Error sendTo(fge::net::Packet& packet, const IpAddress& remoteAddress, fge::net::Port remotePort);
    fge::net::Socket::Error receiveFrom(fge::net::Packet& packet, fge::net::IpAddress& remoteAddress, fge::net::Port& remotePort);
    fge::net::Socket::Error receive(fge::net::Packet& packet);

    fge::net::SocketUdp& operator=(fge::net::SocketUdp&& r);

private:
    std::vector<uint8_t> g_buffer;
};

class FGE_API SocketTcp : public fge::net::Socket
{
public:
    SocketTcp();
    SocketTcp(bool blocking);
    ~SocketTcp() override;

    void flush();

    fge::net::Socket::Error create(fge::net::Socket::SocketDescriptor sck);
    fge::net::Socket::Error create() override;

    fge::net::Socket::Error connect(const fge::net::IpAddress& remoteAddress, fge::net::Port remotePort, uint32_t timeoutms);

    fge::net::Socket::Error send(const void* data, std::size_t size);
    fge::net::Socket::Error send(const void* data, std::size_t size, std::size_t& sent);
    fge::net::Socket::Error receive(void* data, std::size_t size, std::size_t& received);

    fge::net::Socket::Error send(fge::net::Packet& packet);
    fge::net::Socket::Error receive(fge::net::Packet& packet);

    fge::net::Socket::Error sendAndReceive(fge::net::Packet& sendPacket, fge::net::Packet& receivePacket, uint32_t timeoutms);
    fge::net::Socket::Error receive(fge::net::Packet& packet, uint32_t timeoutms);

    fge::net::SocketTcp& operator=(fge::net::SocketTcp&& r);

private:
    std::size_t g_receivedSize;
    std::size_t g_wantedSize;
    std::vector<uint8_t> g_buffer;
};

class FGE_API SocketListenerTcp : public fge::net::Socket
{
public:
    SocketListenerTcp();
    SocketListenerTcp(bool blocking);
    ~SocketListenerTcp() override;

    fge::net::Socket::Error create() override;

    fge::net::Socket::Error listen(fge::net::Port port, const fge::net::IpAddress& address=fge::net::IpAddress::Any);
    fge::net::Socket::Error accept(fge::net::SocketTcp& socket);

    fge::net::SocketListenerTcp& operator=(fge::net::SocketListenerTcp&& r);

private:
};

}//end net
}//end fge

#endif // _FGE_C_SOCKET_HPP_INCLUDED_
