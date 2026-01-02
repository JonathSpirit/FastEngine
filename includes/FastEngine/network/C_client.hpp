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

#ifndef _FGE_C_CLIENT_HPP_INCLUDED
#define _FGE_C_CLIENT_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_identity.hpp"
#include "FastEngine/C_event.hpp"
#include "FastEngine/C_propertyList.hpp"
#include "FastEngine/network/C_netCommand.hpp"
#include "FastEngine/network/C_protocol.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_set>

#define FGE_NET_DEFAULT_LATENCY 20
#define FGE_NET_CLIENT_TIMESTAMP_MODULO 65536
#define FGE_NET_BAD_LATENCY std::numeric_limits<fge::net::Latency_ms>::max()
#define FGE_NET_LATENCY_PLANNER_MEAN 6
#define FGE_NET_DEFAULT_lOST_PACKET_THRESHOLD 15
#define FGE_NET_STATUS_DEFAULT_TIMEOUT                                                                                 \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        2000                                                                                                           \
    }
#define FGE_NET_STATUS_DEFAULT_CONNECTED_TIMEOUT                                                                       \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        6000                                                                                                           \
    }
#define FGE_NET_STATUS_DEFAULT_STATUS "none"

#define FGE_NET_DEFAULT_RETURN_PACKET_RATE                                                                             \
    std::chrono::milliseconds                                                                                          \
    {                                                                                                                  \
        500                                                                                                            \
    }

namespace fge::net
{

/** \addtogroup network
 * @{
 */

using Timestamp = uint16_t;          ///< An timestamp represent modulated current time in milliseconds
using FullTimestamp = uint64_t;      ///< An timestamp represent current time in milliseconds
using FullTimestampOffset = int64_t; ///< An timestamp offset
using Latency_ms = uint16_t; ///< An latency represent the latency of the client->server / server->client connection

/**
 * \struct OneWayLatencyPlanner
 * \brief A helper class that measure latency between client/server
 *
 * This class is called "one-way latency" but it actually compute the
 * Round Trip Time minus a latency corrector divided by 2.
 *
 * \warning When using this class, do not manually edit the corrector latency
 * in the Client class.
 */
class FGE_API OneWayLatencyPlanner
{
public:
    OneWayLatencyPlanner() = default;

    enum Stats : uint8_t
    {
        HAVE_EXTERNAL_TIMESTAMP = 1 << 0
    };

    /**
     * \brief Pack the required data by the planner to the client/server
     *
     * \param tPacket A ProtocolPacket
     */
    void pack(TransmitPacketPtr& tPacket);
    /**
     * \brief Unpack the data received by another client/server planner
     *
     * \param packet A received packet
     * \param client A client
     */
    void unpack(ProtocolPacket* packet, Client& client);

    /**
     * \brief Retrieve the clock offset
     *
     * The clock offset represent the delta time between 2 computers clocks.
     * It is useful in order to predicate some data.
     *
     * \return Optionally a full timestamp offset
     */
    [[nodiscard]] std::optional<FullTimestampOffset> getClockOffset() const;
    /**
     * \brief Retrieve the latency
     *
     * \return Optionally a latency
     */
    [[nodiscard]] std::optional<Latency_ms> getLatency() const;
    /**
     * \brief Retrieve the other side latency
     *
     * This is the latency computed at the other end (client or server)
     *
     * \return Optionally a latency
     */
    [[nodiscard]] std::optional<Latency_ms> getOtherSideLatency() const;
    /**
     * \brief Retrieve the RTT (Round Trip Time)
     *
     * CLIENT
     *
     * FIRST_PACKET         LAST_PACKET
     * O                    O
     * |                    |
     * -                    -
     *  -                  -
     *   -                -
     *    -              -
     *     -            -
     *      -          -
     *      |          |
     *      O -------- O
     *    RECEIVING PACKETS
     *
     * SERVER
     *
     * |                    |
     * -----------------------> RTT (Round Trip Time) ms
     * |    |          |    |
     * |    |          -------> Server To Client (STOC) time ms
     * -------> Client To Server (CTOS) time ms
     *      |          |
     *      -------------> Latency corrector computed by the server
     *
     * \return Optionally a RTT in ms
     */
    [[nodiscard]] std::optional<Latency_ms> getRoundTripTime() const;

private:
    std::optional<Latency_ms> g_latency;
    std::optional<Latency_ms> g_otherSideLatency;

    std::optional<FullTimestampOffset> g_meanClockOffset;
    std::array<FullTimestampOffset, FGE_NET_LATENCY_PLANNER_MEAN> g_clockOffsets{};
    std::size_t g_clockOffsetCount{0};

    std::optional<Latency_ms> g_roundTripTime;

    Timestamp g_externalStoredTimestamp{0};
    std::underlying_type_t<Stats> g_syncStat{0};
};

struct ClientContext
{
    PacketDefragmentation _defragmentation;
    PacketCache _cache;
    PacketReorderer _reorderer;
    CommandQueue _commands;
};

class FGE_API ClientStatus
{
public:
    enum class NetworkStatus
    {
        UNKNOWN,

        ACKNOWLEDGED,
        MTU_DISCOVERED,
        CONNECTED,
        AUTHENTICATED,

        DISCONNECTED,
        TIMEOUT,
    };

    ClientStatus() = default;
    explicit ClientStatus(std::string_view status, NetworkStatus networkStatus = NetworkStatus::UNKNOWN);

    [[nodiscard]] bool isInEncryptedState() const;
    [[nodiscard]] bool isDisconnected() const;
    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] bool isConnecting() const;
    [[nodiscard]] bool isAuthenticated() const;

    [[nodiscard]] std::string const& getStatus() const;
    [[nodiscard]] NetworkStatus getNetworkStatus() const;
    [[nodiscard]] std::chrono::milliseconds getTimeout() const;
    [[nodiscard]] std::chrono::milliseconds getRemainingTimeout() const;

    void set(std::string_view status, NetworkStatus networkStatus);
    void setStatus(std::string_view status);
    void setNetworkStatus(NetworkStatus networkStatus);

    void setTimeout(std::chrono::milliseconds timeout);
    void resetTimeout();
    [[nodiscard]] bool isTimeout() const;

private:
    std::string g_status{FGE_NET_STATUS_DEFAULT_STATUS};
    std::atomic<NetworkStatus> g_networkStatus{NetworkStatus::UNKNOWN};
    std::chrono::milliseconds g_timeout{FGE_NET_STATUS_DEFAULT_TIMEOUT};
    std::chrono::steady_clock::time_point g_currentTimeout{std::chrono::steady_clock::now()};
};

struct FGE_API CryptInfo
{
    ~CryptInfo();

    void* _ssl{nullptr};
    void* _rbio{nullptr};
    void* _wbio{nullptr};
};

/**
 * \class Client
 * \brief Class that represent the identity of a client
 */
class FGE_API Client
{
public:
    Client();
    ~Client();
    /**
     * \brief Constructor with default latencies
     *
     * \param CTOSLatency The "Client To Server" latency
     * \param STOCLatency The "Server To Client" latency
     */
    explicit Client(Latency_ms CTOSLatency, Latency_ms STOCLatency);

    /**
     * \brief Set the "Client To Server" latency
     *
     * \param latency Latency in milliseconds
     */
    void setCTOSLatency_ms(Latency_ms latency);
    /**
     * \brief Set the "Server To Client" latency
     *
     * \param latency Latency in milliseconds
     */
    void setSTOCLatency_ms(Latency_ms latency);
    /**
     * \brief Get the "Client To Server" latency
     *
     * \return Latency in milliseconds
     */
    Latency_ms getCTOSLatency_ms() const;
    /**
     * \brief Get the "Server To Client" latency
     *
     * \return Latency in milliseconds
     */
    Latency_ms getSTOCLatency_ms() const;
    /**
     * \brief Compute the ping
     *
     * The ping is simply the CTOS + STOC latencies.
     *
     * \return Pin in milliseconds
     */
    Latency_ms getPing_ms() const;

    /**
     * \brief Set the corrector timestamp
     *
     * A corrector timestamp is generally set with the timestamp
     * of a received packet.
     *
     * \see getCorrectorLatency
     *
     * \param timestamp The corrector timestamp
     */
    void setCorrectorTimestamp(Timestamp timestamp);
    /**
     * \brief Get the corrector timestamp
     *
     * When the getCorrectorLatency method is called, it will
     * clear the corrector timestamp and std::nullopt will be
     * returned.
     *
     * \see getCorrectorLatency
     *
     * \return The corrector timestamp
     */
    std::optional<Timestamp> getCorrectorTimestamp() const;
    /**
     * \brief Compute the corrector latency
     *
     * The corrector latency is simply the time between the set
     * timestamp and the timestamp now.
     *
     * This is useful in order to compute more precise latency
     * between client/server
     *
     * \return The corrector latency
     */
    std::optional<Latency_ms> getCorrectorLatency() const;

    /**
     * \brief Reset the time point for limiting the packets sending frequency
     *
     * This function is generally automatically called by the network thread.
     */
    void resetLastPacketTimePoint();
    /**
     * \brief Get the delta time between the last sent packet and the current time
     *
     * \return The delta time in milliseconds
     */
    [[nodiscard]] std::chrono::milliseconds getLastPacketElapsedTime() const;
    [[nodiscard]] Latency_ms getLastPacketLatency() const;

    /**
     * \brief Get a modulated timestamp of the current time
     *
     * \return The modulated timestamp
     */
    static Timestamp getTimestamp_ms();
    static Timestamp getTimestamp_ms(FullTimestamp fullTimestamp);
    /**
     * \brief Get a timestamp of the current time
     *
     * \return The timestamp
     */
    static FullTimestamp getFullTimestamp_ms();
    /**
     * \brief Compute the latency for the client->server / server->client with the given timestamps
     *
     * \param sentTimestamp The timestamp that have been sent
     * \param receivedTimestamp The received timestamp
     * \return Latency in milliseconds
     */
    static Latency_ms computeLatency_ms(Timestamp const& sentTimestamp, Timestamp const& receivedTimestamp);

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

    void disconnect(bool pushDisconnectPacket = true);

    [[nodiscard]] ProtocolPacket::RealmType getCurrentRealm() const;
    [[nodiscard]] std::chrono::milliseconds getLastRealmChangeElapsedTime() const;
    void setCurrentRealm(ProtocolPacket::RealmType realm);
    ProtocolPacket::RealmType advanceCurrentRealm();

    [[nodiscard]] ProtocolPacket::CounterType getCurrentPacketCounter() const;
    ProtocolPacket::CounterType advanceCurrentPacketCounter();
    void setCurrentPacketCounter(ProtocolPacket::CounterType counter);

    [[nodiscard]] ProtocolPacket::CounterType getClientPacketCounter() const;
    ProtocolPacket::CounterType advanceClientPacketCounter();
    void setClientPacketCounter(ProtocolPacket::CounterType counter);
    void resetLastReorderedPacketCounter();
    [[nodiscard]] ProtocolPacket::CounterType getLastReorderedPacketCounter() const;

    void acknowledgeReception(ReceivedPacketPtr const& packet);
    [[nodiscard]] std::unordered_set<PacketCache::Label, PacketCache::Label::Hash> const& getAcknowledgedList() const;
    void clearAcknowledgedList();

    void clearLostPacketCount();
    uint32_t advanceLostPacketCount();
    void setLostPacketThreshold(uint32_t threshold);
    [[nodiscard]] uint32_t getLostPacketThreshold() const;
    [[nodiscard]] uint32_t getLostPacketCount() const;

    [[nodiscard]] ClientStatus const& getStatus() const;
    [[nodiscard]] ClientStatus& getStatus();

    [[nodiscard]] CryptInfo const& getCryptInfo() const;
    [[nodiscard]] CryptInfo& getCryptInfo();

    [[nodiscard]] uint16_t getMTU() const;
    void setMTU(uint16_t mtu);

    void setPacketReturnRate(std::chrono::milliseconds rate);
    [[nodiscard]] std::chrono::milliseconds getPacketReturnRate() const;

    CallbackHandler<Client&> _onThresholdLostPacket;

    Event _event;                         ///< Optional client-side event that can be synchronized with the server
    PropertyList _data;                   ///< Some user-defined client properties
    OneWayLatencyPlanner _latencyPlanner; ///< A latency planner that will help latency calculation
    ClientContext _context;        ///< The client context containing utility classes for server/client networking
    bool _mtuFinalizedFlag{false}; ///< A flag that indicate if the MTU has been finalized from the remote side

private:
    mutable std::optional<Timestamp> g_correctorTimestamp;
    Latency_ms g_CTOSLatency_ms;
    Latency_ms g_STOCLatency_ms;
    std::chrono::steady_clock::time_point g_lastPacketTimePoint;

    std::deque<TransmitPacketPtr> g_pendingTransmitPackets;
    mutable std::recursive_mutex g_mutex;

    std::chrono::steady_clock::time_point g_lastRealmChangeTimePoint;
    ProtocolPacket::RealmType g_currentRealm{FGE_NET_DEFAULT_REALM};
    ProtocolPacket::CounterType g_currentPacketCounter{0};
    ProtocolPacket::CounterType g_lastReorderedPacketCounter{0};
    ProtocolPacket::CounterType g_clientPacketCounter{0};

    std::unordered_set<PacketCache::Label, PacketCache::Label::Hash> g_acknowledgedPackets;
    uint32_t g_lostPacketCount{0};
    uint32_t g_lostPacketThreshold{FGE_NET_DEFAULT_lOST_PACKET_THRESHOLD};

    std::chrono::milliseconds g_returnPacketRate{FGE_NET_DEFAULT_RETURN_PACKET_RATE};

    uint16_t g_mtu{0};

    ClientStatus g_status;
    CryptInfo g_cryptInfo;
};

/**
 * @}
 */

} // namespace fge::net

#endif // _FGE_C_CLIENT_HPP_INCLUDED
