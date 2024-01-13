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

#ifndef _FGE_C_CLIENT_HPP_INCLUDED
#define _FGE_C_CLIENT_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_identity.hpp"
#include "C_packet.hpp"
#include "FastEngine/C_event.hpp"
#include "FastEngine/C_propertyList.hpp"
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#define FGE_NET_BAD_SKEY 0
#define FGE_NET_DEFAULT_LATENCY 20
#define FGE_NET_CLIENT_TIMESTAMP_MODULO 65536
#define FGE_NET_BAD_LATENCY std::numeric_limits<fge::net::Latency_ms>::max()
#define FGE_NET_BAD_LATENCY std::numeric_limits<fge::net::Latency_ms>::max()
#define FGE_NET_LATENCY_PLANNER_MEAN 6

namespace fge::net
{

/** \addtogroup network
 * @{
 */

using Skey = uint32_t; ///< The session key can be used to identify a client when connecting to a server.

using Timestamp = uint16_t;          ///< An timestamp represent modulated current time in milliseconds
using FullTimestamp = uint64_t;      ///< An timestamp represent current time in milliseconds
using FullTimestampOffset = int64_t; ///< An timestamp offset
using Latency_ms = uint16_t; ///< An latency represent the latency of the client->server / server->client connection

class FluxPacket;
class Client;

/**
 * \struct TransmissionPacket
 * \brief A packet with configurable options to transmit via a network thread
 *
 * Options will be applied at the moment when the packet will be sent.
 *
 * \warning When the packet is pushed via Client::pushPacket or ClientList::sendToAll, the user must not modify
 * the packet/options anymore causing undefined behavior.
 */
class FGE_API TransmissionPacket
{
public:
    /**
     * \enum Options
     * \brief Options to pass to the network thread when sending a packet
     */
    enum class Options
    {
        UPDATE_TIMESTAMP,         ///< The timestamp of the packet will be updated when sending
        UPDATE_FULL_TIMESTAMP,    ///< The full timestamp of the packet will be updated when sending
        UPDATE_CORRECTION_LATENCY ///< The latency of the packet will be updated with the corrector latency from the Client
    };

    struct Option
    {
        constexpr explicit Option(Options option, std::size_t argument = 0) :
                _option(option),
                _argument(argument)
        {}

        Options _option;       ///< The option to send the packet with
        std::size_t _argument; ///< The option argument
    };

    template<class TPacket>
    [[nodiscard]] static inline std::shared_ptr<TransmissionPacket> create();
    template<class TPacket>
    [[nodiscard]] static inline std::shared_ptr<TransmissionPacket> create(TPacket&& packet);

    TransmissionPacket(TransmissionPacket const& r) = delete;
    TransmissionPacket(TransmissionPacket&& r) noexcept = delete;
    ~TransmissionPacket() = default;

    TransmissionPacket& operator=(TransmissionPacket const& r) = delete;
    TransmissionPacket& operator=(TransmissionPacket&& r) noexcept = delete;

    [[nodiscard]] inline fge::net::Packet const& packet() const;
    [[nodiscard]] inline fge::net::Packet& packet();
    [[nodiscard]] inline std::vector<Option> const& options() const;
    [[nodiscard]] inline std::vector<Option>& options();

    void applyOptions(fge::net::Client const& client);
    void applyOptions();

private:
    inline explicit TransmissionPacket(std::unique_ptr<Packet>&& packet);

    std::unique_ptr<Packet> g_packet;
    std::vector<Option> g_options;
};

using TransmissionPacketPtr = std::shared_ptr<fge::net::TransmissionPacket>;

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
     * \param tPacket A TransmissionPacket
     */
    void pack(fge::net::TransmissionPacketPtr& tPacket);
    /**
     * \brief Unpack the data received by another client/server planner
     *
     * \param packet A received packet
     * \param client A client
     */
    void unpack(fge::net::FluxPacket* packet, fge::net::Client& client);

    /**
     * \brief Retrieve the clock offset
     *
     * The clock offset represent the delta time between 2 computers clocks.
     * It is useful in order to predicate some data.
     *
     * \return Optionally a full timestamp offset
     */
    [[nodiscard]] std::optional<fge::net::FullTimestampOffset> getClockOffset() const;
    /**
     * \brief Retrieve the latency
     *
     * \return Optionally a latency
     */
    [[nodiscard]] std::optional<fge::net::Latency_ms> getLatency() const;
    /**
     * \brief Retrieve the other side latency
     *
     * This is the latency computed at the other end (client or server)
     *
     * \return Optionally a latency
     */
    [[nodiscard]] std::optional<fge::net::Latency_ms> getOtherSideLatency() const;
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
    [[nodiscard]] std::optional<fge::net::Latency_ms> getRoundTripTime() const;

private:
    std::optional<fge::net::Latency_ms> g_latency;
    std::optional<fge::net::Latency_ms> g_otherSideLatency;

    std::optional<fge::net::FullTimestampOffset> g_meanClockOffset;
    std::array<fge::net::FullTimestampOffset, FGE_NET_LATENCY_PLANNER_MEAN> g_clockOffsets{};
    std::size_t g_clockOffsetCount{0};

    std::optional<fge::net::Latency_ms> g_roundTripTime;

    fge::net::Timestamp g_externalStoredTimestamp{0};
    std::underlying_type_t<Stats> g_syncStat{0};
};

/**
 * \class Client
 * \brief Class that represent the identity of a client
 */
class FGE_API Client
{
public:
    Client();
    /**
     * \brief Constructor with default latencies
     *
     * \param CTOSLatency The "Client To Server" latency
     * \param STOCLatency The "Server To Client" latency
     */
    explicit Client(fge::net::Latency_ms CTOSLatency, fge::net::Latency_ms STOCLatency);

    /**
     * \brief Generate a new random session key
     *
     * \return The generated session key
     */
    static fge::net::Skey GenerateSkey();
    /**
     * \brief Set the session key for this client
     *
     * \param key The session key
     */
    void setSkey(fge::net::Skey key);
    /**
     * \brief Get the session key for this client
     *
     * \return The session key
     */
    fge::net::Skey getSkey() const;

    /**
     * \brief Set the "Client To Server" latency
     *
     * \param latency Latency in milliseconds
     */
    void setCTOSLatency_ms(fge::net::Latency_ms latency);
    /**
     * \brief Set the "Server To Client" latency
     *
     * \param latency Latency in milliseconds
     */
    void setSTOCLatency_ms(fge::net::Latency_ms latency);
    /**
     * \brief Get the "Client To Server" latency
     *
     * \return Latency in milliseconds
     */
    fge::net::Latency_ms getCTOSLatency_ms() const;
    /**
     * \brief Get the "Server To Client" latency
     *
     * \return Latency in milliseconds
     */
    fge::net::Latency_ms getSTOCLatency_ms() const;
    /**
     * \brief Compute the ping
     *
     * The ping is simply the CTOS + STOC latencies.
     *
     * \return Pin in milliseconds
     */
    fge::net::Latency_ms getPing_ms() const;

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
    void setCorrectorTimestamp(fge::net::Timestamp timestamp);
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
    std::optional<fge::net::Timestamp> getCorrectorTimestamp() const;
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
    std::optional<fge::net::Latency_ms> getCorrectorLatency() const;

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
    fge::net::Latency_ms getLastPacketElapsedTime();

    /**
     * \brief Get a modulated timestamp of the current time
     *
     * \return The modulated timestamp
     */
    static fge::net::Timestamp getTimestamp_ms();
    static fge::net::Timestamp getTimestamp_ms(fge::net::FullTimestamp fullTimestamp);
    /**
     * \brief Get a timestamp of the current time
     *
     * \return The timestamp
     */
    static fge::net::FullTimestamp getFullTimestamp_ms();
    /**
     * \brief Compute the latency for the client->server / server->client with the given timestamps
     *
     * \param sentTimestamp The timestamp that have been sent
     * \param receivedTimestamp The received timestamp
     * \return Latency in milliseconds
     */
    static fge::net::Latency_ms computeLatency_ms(fge::net::Timestamp const& sentTimestamp,
                                                  fge::net::Timestamp const& receivedTimestamp);

    /**
     * \brief Clear the packet queue
     */
    void clearPackets();
    /**
     * \brief Add a TransmissionPacket to the queue
     *
     * The packet will be sent when the network thread is ready to send it.
     * The network thread is ready to send a packet when the time interval between the last sent packet
     * is greater than the latency of the server->client.
     *
     * \param pck The packet to send with eventual options
     */
    void pushPacket(fge::net::TransmissionPacketPtr pck);
    /**
     * \brief Pop a packet from the queue
     *
     * \return The popped packet or nullptr if the queue is empty
     */
    fge::net::TransmissionPacketPtr popPacket();
    /**
     * \brief Check if the packet queue is empty
     *
     * \return True if the queue is empty, false otherwise
     */
    bool isPendingPacketsEmpty();

    fge::Event _event;       ///< Optional client-side event that can be synchronized with the server
    fge::PropertyList _data; ///< Some user-defined client properties
    fge::net::OneWayLatencyPlanner _latencyPlanner; ///< A latency planner that will help latency calculation

private:
    mutable std::optional<fge::net::Timestamp> g_correctorTimestamp;
    fge::net::Latency_ms g_CTOSLatency_ms;
    fge::net::Latency_ms g_STOCLatency_ms;
    std::chrono::steady_clock::time_point g_lastPacketTimePoint;

    std::queue<fge::net::TransmissionPacketPtr> g_pendingTransmitPackets;
    mutable std::recursive_mutex g_mutex;

    fge::net::Skey g_skey;
};

/**
 * @}
 */

} // namespace fge::net

#include "C_client.inl"

#endif // _FGE_C_CLIENT_HPP_INCLUDED
