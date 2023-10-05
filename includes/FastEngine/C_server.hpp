/*
 * Copyright 2023 Guillaume Guillet
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
#include "FastEngine/C_clientList.hpp"
#include "FastEngine/C_packet.hpp"
#include "FastEngine/C_packetBZ2.hpp"
#include "FastEngine/C_packetLZ4.hpp"
#include "FastEngine/C_socket.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#define FGE_SERVER_DEFAULT_MAXPACKET 200

namespace fge::net
{

struct FGE_API FluxPacket
{
    FluxPacket(fge::net::Packet const& pck,
               fge::net::Identity const& id,
               std::size_t fluxIndex = 0,
               std::size_t fluxCount = 0) :
            _pck(pck),
            _id(id),
            _timestamp(fge::net::Client::getTimestamp_ms()),
            _fluxIndex(fluxIndex),
            _fluxCount(fluxCount)
    {}
    FluxPacket(fge::net::Packet&& pck,
               fge::net::Identity const& id,
               std::size_t fluxIndex = 0,
               std::size_t fluxCount = 0) :
            _pck(std::move(pck)),
            _id(id),
            _timestamp(fge::net::Client::getTimestamp_ms()),
            _fluxIndex(fluxIndex),
            _fluxCount(fluxCount)
    {}

    fge::net::Packet _pck;
    fge::net::Identity _id;
    fge::net::Timestamp _timestamp;

    std::size_t _fluxIndex;
    std::size_t _fluxCount;
};
using FluxPacketSharedPtr = std::shared_ptr<fge::net::FluxPacket>;

class ServerUdp;

class FGE_API ServerFluxUdp
{
public:
    ServerFluxUdp() = default;
    ServerFluxUdp(ServerFluxUdp const& r) = delete;
    ServerFluxUdp(ServerFluxUdp&& r) noexcept = delete;
    ~ServerFluxUdp();

    ServerFluxUdp& operator=(ServerFluxUdp const& r) = delete;
    ServerFluxUdp& operator=(ServerFluxUdp&& r) noexcept = delete;

    void clearPackets();
    [[nodiscard]] FluxPacketSharedPtr popNextPacket();

    [[nodiscard]] std::size_t getPacketsSize() const;
    [[nodiscard]] bool isEmpty() const;

    void setMaxPackets(std::size_t n);
    [[nodiscard]] std::size_t getMaxPackets() const;

    fge::net::ClientList _clients;

private:
    bool pushPacket(FluxPacketSharedPtr const& fluxPck);
    void forcePushPacket(FluxPacketSharedPtr fluxPck);

    mutable std::mutex g_mutexLocal;

    std::queue<FluxPacketSharedPtr> g_packets;
    std::size_t g_maxPackets = FGE_SERVER_DEFAULT_MAXPACKET;

    friend class ServerUdp;
};

class FGE_API ServerUdp
{
public:
    ServerUdp();
    ServerUdp(ServerUdp const& r) = delete;
    ServerUdp(ServerUdp&& r) noexcept = delete;
    ~ServerUdp();

    ServerUdp& operator=(ServerUdp const& r) = delete;
    ServerUdp& operator=(ServerUdp&& r) noexcept = delete;

    template<class TPacket = fge::net::Packet>
    [[nodiscard]] bool start(fge::net::Port bindPort, fge::net::IpAddress const& bindIp = fge::net::IpAddress::Any);
    template<class TPacket = fge::net::Packet>
    [[nodiscard]] bool start();
    void stop();

    [[nodiscard]] fge::net::ServerFluxUdp* newFlux();

    [[nodiscard]] fge::net::ServerFluxUdp* getFlux(std::size_t index);
    [[nodiscard]] fge::net::ServerFluxUdp* getDefaultFlux();

    [[nodiscard]] std::size_t getFluxSize() const;

    void closeFlux(fge::net::ServerFluxUdp* flux);
    void closeAllFlux();

    void repushPacket(FluxPacketSharedPtr&& fluxPck);

    void notifyTransmission();
    [[nodiscard]] bool isRunning() const;

    fge::net::Socket::Error sendTo(fge::net::Packet& pck, fge::net::IpAddress const& ip, fge::net::Port port);
    fge::net::Socket::Error sendTo(fge::net::Packet& pck, fge::net::Identity const& id);

private:
    template<class TPacket>
    void serverThreadReception();
    void serverThreadTransmission();

    std::unique_ptr<std::thread> g_threadReception;
    std::unique_ptr<std::thread> g_threadTransmission;

    std::condition_variable g_transmissionNotifier;

    mutable std::mutex g_mutexTransmission;
    mutable std::mutex g_mutexServer;

    std::vector<std::unique_ptr<fge::net::ServerFluxUdp>> g_fluxes;
    fge::net::ServerFluxUdp g_defaultFlux;

    fge::net::SocketUdp g_socket;
    bool g_running;
};

class FGE_API ServerClientSideUdp
{
public:
    ServerClientSideUdp();
    ServerClientSideUdp(ServerClientSideUdp const& r) = delete;
    ServerClientSideUdp(ServerClientSideUdp&& r) noexcept = delete;
    ~ServerClientSideUdp();

    ServerClientSideUdp& operator=(ServerClientSideUdp const& r) = delete;
    ServerClientSideUdp& operator=(ServerClientSideUdp&& r) noexcept = delete;

    template<class TPacket = fge::net::Packet>
    [[nodiscard]] bool start(fge::net::Port bindPort,
                             fge::net::IpAddress const& bindIp,
                             fge::net::Port connectRemotePort,
                             fge::net::IpAddress const& connectRemoteAddress);
    void stop();

    void notifyTransmission();
    [[nodiscard]] bool isRunning() const;

    fge::net::Socket::Error send(fge::net::Packet& pck);

    [[nodiscard]] FluxPacketSharedPtr popNextPacket();

    [[nodiscard]] std::size_t getPacketsSize() const;
    [[nodiscard]] bool isEmpty() const;

    void setMaxPackets(std::size_t n);
    [[nodiscard]] std::size_t getMaxPackets() const;

    [[nodiscard]] std::size_t waitForPackets(std::chrono::milliseconds const& ms);

    [[nodiscard]] fge::net::Identity const& getClientIdentity() const;

    fge::net::Client _client; //But it is the server :O

private:
    template<class TPacket>
    void serverThreadReception();
    void serverThreadTransmission();

    bool pushPacket(FluxPacketSharedPtr const& fluxPck);

    std::unique_ptr<std::thread> g_threadReception;
    std::unique_ptr<std::thread> g_threadTransmission;

    std::condition_variable g_transmissionNotifier;
    std::condition_variable g_receptionNotifier;

    mutable std::mutex g_mutexTransmission;
    mutable std::mutex g_mutexServer;

    fge::net::SocketUdp g_socket;
    bool g_running;

    std::queue<FluxPacketSharedPtr> g_packets;
    std::size_t g_maxPackets = FGE_SERVER_DEFAULT_MAXPACKET;

    fge::net::Identity g_clientIdentity;
};

} // namespace fge::net

#include "FastEngine/C_server.inl"

#endif // _FGE_C_SERVER_HPP_INCLUDED
