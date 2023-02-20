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

#include "FastEngine/fastengine_extern.hpp"
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
    FluxPacket(const fge::net::Packet& pck,
               const fge::net::Identity& id,
               std::size_t fluxIndex = 0,
               std::size_t fluxCount = 0) :
            _pck(pck),
            _id(id),
            _timestamp(fge::net::Client::getTimestamp_ms()),
            _fluxIndex(fluxIndex),
            _fluxCount(fluxCount)
    {}
    FluxPacket(fge::net::Packet&& pck,
               const fge::net::Identity& id,
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
    ~ServerFluxUdp();

    void clear();

    FluxPacketSharedPtr popNextPacket();

    std::size_t getPacketsSize() const;
    bool isEmpty() const;

    void setMaxPackets(std::size_t n);
    std::size_t getMaxPackets() const;

    fge::net::ClientList _clients;

private:
    bool pushPacket(const FluxPacketSharedPtr& fluxPck);
    void forcePushPacket(const FluxPacketSharedPtr& fluxPck);

    mutable std::mutex g_mutexLocal;

    std::queue<FluxPacketSharedPtr> g_packets;
    std::size_t g_maxPackets = FGE_SERVER_DEFAULT_MAXPACKET;

    friend class ServerUdp;
};

class FGE_API ServerUdp
{
public:
    ServerUdp();
    ~ServerUdp();

    template<typename Tpacket = fge::net::Packet>
    bool start(fge::net::Port port, const fge::net::IpAddress& ip = fge::net::IpAddress::Any);
    template<typename Tpacket = fge::net::Packet>
    bool start();
    void stop();

    fge::net::ServerFluxUdp* newFlux();

    fge::net::ServerFluxUdp* getFlux(std::size_t index);
    fge::net::ServerFluxUdp* getDefaultFlux();

    std::size_t getFluxSize() const;

    void delFlux(fge::net::ServerFluxUdp* flux);
    void delAllFlux();

    void repushPacket(const FluxPacketSharedPtr& fluxPck);

    const fge::net::SocketUdp& getSocket() const;
    fge::net::SocketUdp& getSocket();

    void notify();
    std::mutex& getSendMutex();

    fge::net::Socket::Error sendTo(fge::net::Packet& pck, const fge::net::IpAddress& ip, fge::net::Port port);
    fge::net::Socket::Error sendTo(fge::net::Packet& pck, const fge::net::Identity& id);

    bool isRunning() const;

private:
    template<typename Tpacket>
    void serverThreadReception();
    void serverThreadTransmission();

    std::thread* g_threadReception;
    std::thread* g_threadTransmission;

    std::condition_variable g_cv;

    mutable std::mutex g_mutexSend;
    mutable std::mutex g_mutexServer;

    std::vector<std::unique_ptr<fge::net::ServerFluxUdp>> g_flux;
    fge::net::ServerFluxUdp g_defaultFlux;

    fge::net::SocketUdp g_socket;
    bool g_running;
};

class FGE_API ServerClientSideUdp
{
public:
    ServerClientSideUdp();
    ~ServerClientSideUdp();

    template<typename Tpacket = fge::net::Packet>
    bool start(fge::net::Port port,
               const fge::net::IpAddress& ip,
               const fge::net::IpAddress& remoteAddress,
               fge::net::Port remotePort);
    template<typename Tpacket = fge::net::Packet>
    bool start();
    void stop();

    const fge::net::SocketUdp& getSocket() const;
    fge::net::SocketUdp& getSocket();

    void notify();
    std::mutex& getSendMutex();

    fge::net::Socket::Error send(fge::net::Packet& pck);

    bool isRunning() const;

    FluxPacketSharedPtr popNextPacket();

    std::size_t getPacketsSize() const;
    bool isEmpty() const;

    void setMaxPackets(std::size_t n);
    std::size_t getMaxPackets() const;

    bool waitForPackets(const std::chrono::milliseconds& ms);

    const fge::net::Identity& getClientIdentity() const;

    fge::net::Client _client; //But it is the server :O

private:
    template<typename Tpacket>
    void serverThreadReception();
    void serverThreadTransmission();

    bool pushPacket(const FluxPacketSharedPtr& fluxPck);

    std::thread* g_threadReception;
    std::thread* g_threadTransmission;

    std::condition_variable g_cv;
    std::condition_variable g_cvReceiveNotifier;

    mutable std::mutex g_mutexSend;
    mutable std::mutex g_mutexServer;

    fge::net::SocketUdp g_socket;
    bool g_running;

    std::queue<FluxPacketSharedPtr> g_packets;
    std::size_t g_maxPackets = FGE_SERVER_DEFAULT_MAXPACKET;

    fge::net::Identity g_clientIdentity;
};

} // namespace fge::net

#include <FastEngine/C_server.inl>

#endif // _FGE_C_SERVER_HPP_INCLUDED
