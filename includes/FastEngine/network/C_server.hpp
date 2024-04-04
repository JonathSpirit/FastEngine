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

#ifndef _FGE_C_SERVER_HPP_INCLUDED
#define _FGE_C_SERVER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_socket.hpp"
#include "FastEngine/network/C_clientList.hpp"
#include "FastEngine/network/C_packet.hpp"
#include "FastEngine/network/C_packetBZ2.hpp"
#include "FastEngine/network/C_packetLZ4.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#if defined(FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST) || defined(FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST)
    #include "FastEngine/C_random.hpp"
#endif

#define FGE_SERVER_DEFAULT_MAXPACKET 200

namespace fge::net
{

class ServerSideNetUdp;
class ClientSideNetUdp;

class FluxPacket
{
public:
    inline FluxPacket(fge::net::Packet const& pck,
                      fge::net::Identity const& id,
                      std::size_t fluxIndex = 0,
                      std::size_t fluxCount = 0);
    inline FluxPacket(fge::net::Packet&& pck,
                      fge::net::Identity const& id,
                      std::size_t fluxIndex = 0,
                      std::size_t fluxCount = 0);
    ~FluxPacket() = default;

    fge::net::Packet _packet;

    [[nodiscard]] inline fge::net::Timestamp getTimeStamp() const;
    [[nodiscard]] inline fge::net::Identity const& getIdentity() const;

private:
    friend class ServerSideNetUdp;
    friend class ClientSideNetUdp;

    fge::net::Identity g_id;
    fge::net::Timestamp g_timestamp;

    std::size_t g_fluxIndex;
    std::size_t g_fluxCount;
};
using FluxPacketPtr = std::unique_ptr<fge::net::FluxPacket>;

/**
 * \class NetFluxUdp
 * \ingroup network
 * \brief A network flux
 *
 * Every flux have its own client list and packet queue.
 *
 * When a packet is received by the network, it is pushed into the flux only
 * if FGE_SERVER_DEFAULT_MAXPACKET is not reached. If it is, the packet is
 * transmitted to another flux or dismissed if no one is available.
 */
class FGE_API NetFluxUdp
{
public:
    NetFluxUdp() = default;
    NetFluxUdp(NetFluxUdp const& r) = delete;
    NetFluxUdp(NetFluxUdp&& r) noexcept = delete;
    virtual ~NetFluxUdp();

    NetFluxUdp& operator=(NetFluxUdp const& r) = delete;
    NetFluxUdp& operator=(NetFluxUdp&& r) noexcept = delete;

    void clearPackets();
    [[nodiscard]] FluxPacketPtr popNextPacket();

    [[nodiscard]] std::size_t getPacketsSize() const;
    [[nodiscard]] bool isEmpty() const;

    void setMaxPackets(std::size_t n);
    [[nodiscard]] std::size_t getMaxPackets() const;

protected:
    bool pushPacket(FluxPacketPtr&& fluxPck);
    void forcePushPacket(FluxPacketPtr fluxPck);

    mutable std::mutex _g_mutexFlux;
    std::queue<FluxPacketPtr> _g_packets;

private:
    std::size_t g_maxPackets = FGE_SERVER_DEFAULT_MAXPACKET;

    friend class ServerSideNetUdp;
};
class FGE_API ServerNetFluxUdp : public NetFluxUdp
{
public:
    ServerNetFluxUdp() = default;
    ~ServerNetFluxUdp() override = default;

    fge::net::ClientList _clients;
};

/**
 * \class ServerSideNetUdp
 * \ingroup network
 * \brief A server side network manager
 *
 * This class is used to manage clients on the server side. When started, it will create
 * two threads, one for the reception and one for the transmission.
 *
 * The reception thread will receive packets from the network and push them in created fluxes.
 *
 * The transmission thread will pop packets from the client queued packets and send them
 * taking care of the latency.
 */
class FGE_API ServerSideNetUdp
{
public:
    explicit ServerSideNetUdp(IpAddress::Types addressType = IpAddress::Types::Ipv4);
    ServerSideNetUdp(ServerSideNetUdp const& r) = delete;
    ServerSideNetUdp(ServerSideNetUdp&& r) noexcept = delete;
    ~ServerSideNetUdp();

    ServerSideNetUdp& operator=(ServerSideNetUdp const& r) = delete;
    ServerSideNetUdp& operator=(ServerSideNetUdp&& r) noexcept = delete;

    template<class TPacket = fge::net::Packet>
    [[nodiscard]] bool start(fge::net::Port bindPort,
                             fge::net::IpAddress const& bindIp,
                             IpAddress::Types addressType = IpAddress::Types::None);
    template<class TPacket = fge::net::Packet>
    [[nodiscard]] bool start(IpAddress::Types addressType = IpAddress::Types::None);
    void stop();

    /**
     * \brief Create a new flux
     *
     * When receiving a packet, the server will push it into a flux sequentially, if the
     * thread that handle the flux doesn't want to handle the packet (because the packet
     * is not from his client list, or other reasons) the thread must call repushPacket()
     * to push the packet into another flux.
     *
     * If no flux is available, the packet is pushed into the default flux or dismissed if
     * the default flux is full.
     *
     * \see NetFluxUdp
     *
     * \return A pointer to the new flux
     */
    [[nodiscard]] fge::net::ServerNetFluxUdp* newFlux();

    [[nodiscard]] fge::net::ServerNetFluxUdp* getFlux(std::size_t index);
    [[nodiscard]] fge::net::ServerNetFluxUdp* getDefaultFlux();

    [[nodiscard]] std::size_t getFluxSize() const;

    void closeFlux(fge::net::NetFluxUdp* flux);
    void closeAllFlux();

    void repushPacket(FluxPacketPtr&& fluxPck);

    /**
     * \brief Notify the transmission thread
     *
     * This function notify the transmission thread to send packets.
     * This should be called when a packet when you finish to push packets
     * for clients.
     */
    void notifyTransmission();
    [[nodiscard]] bool isRunning() const;

    fge::net::Socket::Error sendTo(fge::net::Packet& pck, fge::net::Identity const& id);
    fge::net::Socket::Error
    sendTo(fge::net::TransmissionPacketPtr& pck, fge::net::Client const& client, fge::net::Identity const& id);
    fge::net::Socket::Error sendTo(fge::net::TransmissionPacketPtr& pck, fge::net::Identity const& id);

private:
    template<class TPacket>
    void threadReception();
    void threadTransmission();

    std::unique_ptr<std::thread> g_threadReception;
    std::unique_ptr<std::thread> g_threadTransmission;

    std::condition_variable g_transmissionNotifier;

    mutable std::mutex g_mutexTransmission;
    mutable std::mutex g_mutexServer;

    std::vector<std::unique_ptr<fge::net::ServerNetFluxUdp>> g_fluxes;
    fge::net::ServerNetFluxUdp g_defaultFlux;

    fge::net::SocketUdp g_socket;
    bool g_running;
};

/**
 * \class ClientSideNetUdp
 * \ingroup network
 * \brief A client side network manager
 *
 * \see ServerSideNetUdp
 */
class FGE_API ClientSideNetUdp : public NetFluxUdp
{
public:
    explicit ClientSideNetUdp(IpAddress::Types addressType = IpAddress::Types::Ipv4);
    ClientSideNetUdp(ClientSideNetUdp const& r) = delete;
    ClientSideNetUdp(ClientSideNetUdp&& r) noexcept = delete;
    ~ClientSideNetUdp() override;

    ClientSideNetUdp& operator=(ClientSideNetUdp const& r) = delete;
    ClientSideNetUdp& operator=(ClientSideNetUdp&& r) noexcept = delete;

    template<class TPacket = fge::net::Packet>
    [[nodiscard]] bool start(fge::net::Port bindPort,
                             fge::net::IpAddress const& bindIp,
                             fge::net::Port connectRemotePort,
                             fge::net::IpAddress const& connectRemoteAddress,
                             IpAddress::Types addressType = IpAddress::Types::None);
    void stop();

    void notifyTransmission();
    [[nodiscard]] bool isRunning() const;

    fge::net::Socket::Error send(fge::net::Packet& pck);
    fge::net::Socket::Error send(fge::net::TransmissionPacketPtr& pck);

    [[nodiscard]] std::size_t waitForPackets(std::chrono::milliseconds time_ms);

    [[nodiscard]] fge::net::Identity const& getClientIdentity() const;

    fge::net::Client _client; //But it is the server :O

private:
    template<class TPacket>
    void threadReception();
    void threadTransmission();

    std::unique_ptr<std::thread> g_threadReception;
    std::unique_ptr<std::thread> g_threadTransmission;

    std::condition_variable g_transmissionNotifier;
    std::condition_variable g_receptionNotifier;

    mutable std::mutex g_mutexTransmission;

    fge::net::SocketUdp g_socket;
    bool g_running;

    fge::net::Identity g_clientIdentity;
};

} // namespace fge::net

#include "C_server.inl"

#endif // _FGE_C_SERVER_HPP_INCLUDED
