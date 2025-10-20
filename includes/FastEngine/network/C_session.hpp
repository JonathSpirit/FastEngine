/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_C_SESSION_HPP_INCLUDED
#define _FGE_C_SESSION_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/network/C_protocol.hpp"
#include <chrono>
#include <mutex>

#define FGE_NET_DEFAULT_SESSION 0
#define FGE_NET_SESSION_ACK_TIMEOUT_MS 500

namespace fge::net
{

using MTU = uint16_t;

class SessionManager;

class FGE_API Session
{
public:
    using Id = uint8_t;

    enum class States
    {
        UNINITIALIZED,
        INITIALIZED,
        RECONFIGURING,
        DELETING
    };

    explicit Session(Id session);
    Session(Session const& r) = delete;
    Session(Session&& r) noexcept;
    ~Session() = default;

    Session& operator=(Session const& r) = delete;
    Session& operator=(Session&& r) noexcept = delete;

    void updateSession(SessionManager& manager, std::chrono::milliseconds deltaTime);

    //Normal workflow
    ProtocolPacket::RealmType advanceRealm();

    //Need reconfiguration (or not, in some cases)
    void resetPacketCounter();
    void resetRealm();
    void setMTU(MTU mtu);
    void forceMTU(bool force);

    [[nodiscard]] MTU getMTU() const;
    [[nodiscard]] bool isMTUForced() const;

    //Reception handling
    void pushPacket(ReceivedPacketPtr&& pck);
    ReceivedPacketPtr popPacket();

    [[nodiscard]] States getCurrentState() const;
    
private:
    void forceState(States state);

    friend class SessionManager;

    mutable std::mutex g_mutex;

    std::chrono::steady_clock::time_point g_lastReceivedPacketTimePoint;
    std::chrono::steady_clock::time_point g_lastRealmTimePoint;

    ProtocolPacket::RealmType g_serverRealm{FGE_NET_DEFAULT_REALM};
    ProtocolPacket::CounterType g_serverPacketCounter{0};
    ProtocolPacket::CounterType g_clientPacketCounter{0};

    PacketCache g_packetCache;
    std::vector<PacketCache::Label> g_acknowledgedPackets;
    PacketReorderer g_packetReorderer;
    PacketDefragmentation g_packetDefragmentation;

    enum class ComStates
    {
        NONE,
        NEED_ACK
    };

    std::chrono::milliseconds g_tryTimeout{0};
    unsigned int g_tryCount{0};
    ComStates g_comState{ComStates::NONE};
    States g_state{States::UNINITIALIZED};
    Id g_id{FGE_NET_DEFAULT_SESSION};

    MTU g_mtu{0};
    bool g_forceMTU{false};
    bool g_enableCache{false};
    bool g_enableReorderer{false};
    bool g_enableDefragmentation{false};
};

class FGE_API SessionManager
{
public:
    using TransmitQueue = std::deque<TransmitPacketPtr>;

    SessionManager();
    ~SessionManager() = default;

    void updateSessions(std::chrono::milliseconds deltaTime);
    Session& createSession();
    void deleteSession(Session::Id id);
    Session* getSession(Session::Id id);
    void clearSessions();
    [[nodiscard]] std::size_t getSessionCount() const;

    void handlePacketReception(ReceivedPacketPtr&& packet);

    /**
     * \brief Clear the packet queue
     */
    void clearPackets();
    /**
     * \brief Add a Packet to the queue
     *
     * The packet will be sent when the network thread is ready to send it.
     * The network thread is ready to send a packet when the time interval between the last sent packet
     * is greater than the latency of the server->client.
     *
     * The current realm will be set and the current countId will be incremented and set to the packet.
     * - on the client side, advanceClientPacketCountId() will be called
     * - on the server side, advanceCurrentPacketCountId() will be called
     *
     * \param pck The packet to send with eventual options
     */
    void pushPacket(TransmitPacketPtr pck);
    void pushForcedFrontPacket(TransmitPacketPtr pck);
    /**
     * \brief Pop a packet from the queue
     *
     * \return The popped packet or nullptr if the queue is empty
     */
    TransmitPacketPtr popPacket();
    /**
     * \brief Check if the packet queue is empty
     *
     * \return True if the queue is empty, false otherwise
     */
    bool isPendingPacketsEmpty() const;

    [[nodiscard]] AccessLock<std::mutex> acquireLock() const;
    [[nodiscard]] TransmitQueue::const_iterator begin(AccessLock<std::mutex> const& lock) const;
    [[nodiscard]] TransmitQueue::const_iterator end(AccessLock<std::mutex> const& lock) const;
    TransmitQueue::const_iterator erase(AccessLock<std::mutex> const& lock, TransmitQueue::const_iterator it);

private:
    mutable std::mutex g_mutex;

    std::vector<Session> g_sessions;
    TransmitQueue g_pendingTransmitPackets;
};

} // namespace fge::net

#endif // _FGE_C_SESSION_HPP_INCLUDED
