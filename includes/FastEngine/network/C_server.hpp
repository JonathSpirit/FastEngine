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
#include "FastEngine/network/C_netCommand.hpp"
#include "FastEngine/network/C_packet.hpp"
#include "FastEngine/network/C_packetLZ4.hpp"
#include "FastEngine/network/C_protocol.hpp"
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#if defined(FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST) || defined(FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST)
    #include "FastEngine/C_random.hpp"
#endif

#define FGE_SERVER_DEFAULT_MAXPACKET 200
#define FGE_SERVER_MAX_TIME_DIFFERENCE_REALM                                                                           \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        2000                                                                                                           \
    }

#define FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS 250

namespace fge::net
{

class ServerSideNetUdp;
class ClientSideNetUdp;

enum class FluxProcessResults
{
    RETRIEVABLE,
    BAD_REALM,
    BAD_REORDER,
    NOT_RETRIEVABLE,
    INTERNALLY_HANDLED
};

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
    [[nodiscard]] ReceivedPacketPtr popNextPacket();

    [[nodiscard]] std::size_t getPacketsSize() const;
    [[nodiscard]] bool isEmpty() const;

    void setMaxPackets(std::size_t n);
    [[nodiscard]] std::size_t getMaxPackets() const;

protected:
    bool pushPacket(ReceivedPacketPtr&& fluxPck);
    void forcePushPacket(ReceivedPacketPtr fluxPck);
    void forcePushPacketFront(ReceivedPacketPtr fluxPck);
    [[nodiscard]] FluxProcessResults processReorder(Client& client,
                                                    ReceivedPacketPtr& packet,
                                                    ProtocolPacket::CounterType currentCounter,
                                                    bool ignoreRealm);

    mutable std::mutex _g_mutexFlux;
    std::deque<ReceivedPacketPtr> _g_packets;
    std::size_t _g_remainingPackets{0};

private:
    std::size_t g_maxPackets = FGE_SERVER_DEFAULT_MAXPACKET;

    friend class ServerSideNetUdp;
};

class FGE_API ServerNetFluxUdp : public NetFluxUdp
{
public:
    explicit ServerNetFluxUdp(ServerSideNetUdp& server) :
            NetFluxUdp(),
            g_server(&server)
    {}
    ~ServerNetFluxUdp() override = default;

    void processClients();
    [[nodiscard]] FluxProcessResults
    process(ClientSharedPtr& refClient, ReceivedPacketPtr& packet, bool allowUnknownClient);

    ClientList _clients;

    CallbackHandler<ClientSharedPtr const&> _onClientBadRealm;
    CallbackHandler<ClientSharedPtr, Identity> _onClientTimeout;

    CallbackHandler<ClientSharedPtr const&, Identity> _onClientAcknowledged;
    CallbackHandler<ClientSharedPtr const&, Identity> _onClientMTUDiscovered;
    CallbackHandler<ClientSharedPtr const&, Identity> _onClientConnected;
    CallbackHandler<ClientSharedPtr, Identity> _onClientDisconnected;
    CallbackHandler<ClientSharedPtr, Identity> _onClientDropped;

private:
    [[nodiscard]] bool verifyRealm(ClientSharedPtr const& refClient, ReceivedPacketPtr const& packet);
    [[nodiscard]] NetCommandResults
    checkCommands(ClientSharedPtr const& refClient, CommandQueue& commands, ReceivedPacketPtr& packet);

    [[nodiscard]] FluxProcessResults processUnknownClient(ClientSharedPtr& refClient, ReceivedPacketPtr& packet);
    [[nodiscard]] FluxProcessResults processAcknowledgedClient(ClientList::Data& refClientData,
                                                               ReceivedPacketPtr& packet);
    [[nodiscard]] FluxProcessResults processMTUDiscoveredClient(ClientList::Data& refClientData,
                                                                ReceivedPacketPtr& packet);

    ServerSideNetUdp* g_server{nullptr};
    std::chrono::milliseconds g_commandsUpdateTick{0};
    std::chrono::steady_clock::time_point g_lastCommandUpdateTimePoint{std::chrono::steady_clock::now()};
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

    [[nodiscard]] bool
    start(Port bindPort, IpAddress const& bindIp, IpAddress::Types addressType = IpAddress::Types::None);
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
    [[nodiscard]] ServerNetFluxUdp* newFlux();

    [[nodiscard]] ServerNetFluxUdp* getFlux(std::size_t index);
    [[nodiscard]] ServerNetFluxUdp* getDefaultFlux();

    [[nodiscard]] std::size_t getFluxSize() const;

    [[nodiscard]] IpAddress::Types getAddressType() const;

    void closeFlux(NetFluxUdp* flux);
    void closeAllFlux();

    void repushPacket(ReceivedPacketPtr&& packet);

    /**
     * \brief Notify the transmission thread
     *
     * This function notify the transmission thread to send packets.
     * This should be called when a packet when you finish to push packets
     * for clients.
     */
    void notifyTransmission();
    [[nodiscard]] bool isRunning() const;

    void notifyNewClient(Identity const& identity, ClientSharedPtr const& client);

    void sendTo(TransmitPacketPtr& pck, Client const& client, Identity const& id);
    void sendTo(TransmitPacketPtr& pck, Identity const& id);

    [[nodiscard]] void* getCryptContext() const;

private:
    void threadReception();
    void threadTransmission();

    std::unique_ptr<std::thread> g_threadReception;
    std::unique_ptr<std::thread> g_threadTransmission;

    std::condition_variable g_transmissionNotifier;

    mutable std::mutex g_mutexServer;

    std::vector<std::unique_ptr<ServerNetFluxUdp>> g_fluxes;
    ServerNetFluxUdp g_defaultFlux;
    std::queue<std::pair<TransmitPacketPtr, Identity>> g_transmissionQueue;

    std::unordered_map<Identity, std::weak_ptr<Client>, IdentityHash> g_clientsMap;

    SocketUdp g_socket;
    bool g_running;

    void* g_crypt_ctx;
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

    [[nodiscard]] bool start(Port bindPort,
                             IpAddress const& bindIp,
                             Port connectRemotePort,
                             IpAddress const& connectRemoteAddress,
                             IpAddress::Types addressType = IpAddress::Types::None);
    void stop();

    void notifyTransmission();
    [[nodiscard]] bool isRunning() const;

    [[nodiscard]] std::future<uint16_t> retrieveMTU();
    [[nodiscard]] std::future<bool> connect();

    [[nodiscard]] IpAddress::Types getAddressType() const;

    [[nodiscard]] std::size_t waitForPackets(std::chrono::milliseconds time_ms);

    [[nodiscard]] Identity const& getClientIdentity() const;

    template<class TPacket = Packet>
    void sendTo(TransmitPacketPtr& pck, Identity const& id);

    [[nodiscard]] FluxProcessResults process(ReceivedPacketPtr& packet);

    Client _client; //But it is the server :O

    CallbackHandler<ClientSideNetUdp&> _onClientTimeout;

private:
    void threadReception();
    void threadTransmission();

    std::recursive_mutex g_mutexCommands;
    CommandQueue g_commands;

    std::unique_ptr<std::thread> g_threadReception;
    std::unique_ptr<std::thread> g_threadTransmission;

    std::condition_variable g_transmissionNotifier;
    std::condition_variable g_receptionNotifier;

    SocketUdp g_socket;
    bool g_running;

    Identity g_clientIdentity;

    PacketDefragmentation g_defragmentation;

    void* g_crypt_ctx;
};

} // namespace fge::net

#include "C_server.inl"

#endif // _FGE_C_SERVER_HPP_INCLUDED
