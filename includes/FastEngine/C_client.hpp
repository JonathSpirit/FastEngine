/*
 * Copyright 2022 Guillaume Guillet
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

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_identity.hpp>
#include <FastEngine/C_propertyList.hpp>
#include <FastEngine/C_event.hpp>
#include <queue>
#include <chrono>
#include <mutex>
#include <memory>

#define FGE_NET_BAD_SKEY 0
#define FGE_NET_DEFAULT_LATENCY 80

namespace fge::net
{

/** \addtogroup network
 * @{
 */

/**
 * \typedef Skey
 * \brief The session key type
 *
 * The session key can be used to identify a client when connecting to a server.
 */
using Skey = uint32_t;

/**
 * \enum ClientSendQueuePacketOptions
 * \brief Options to pass to the network thread when sending a packet
 */
enum ClientSendQueuePacketOptions : uint8_t
{
    QUEUE_PACKET_OPTION_NONE = 0, ///< No option, the packet will be sent immediately
    QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP ///< The timestamp of the packet will be updated when sending
};

/**
 * \struct ClientSendQueuePacket
 * \brief A packet to send to the network thread
 */
struct ClientSendQueuePacket
{
    std::shared_ptr<fge::net::Packet> _pck; ///< The data packet to send
    fge::net::ClientSendQueuePacketOptions _option{fge::net::QUEUE_PACKET_OPTION_NONE}; ///< The option to send the packet with
    std::size_t _optionArg{0}; ///< The option argument
};

/**
 * \class Client
 * \brief Class that represent the identity of a client
 */
class FGE_API Client
{
public:
    using Timestamp = uint16_t; ///< An timestamp represent modulated current time in milliseconds
    using Latency_ms = uint16_t; ///< An latency represent the latency of the client->server / server->client connection

    Client();
    /**
     * \brief Constructor with a default server->client latency
     *
     * \param latency The server->client latency
     */
    explicit Client(fge::net::Client::Latency_ms latency);

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
     * \brief Set the server->client latency
     *
     * \param t Latency in milliseconds
     */
    void setLatency_ms(fge::net::Client::Latency_ms t);
    /**
     * \brief Get the server->client latency
     *
     * \return Latency in milliseconds
     */
    fge::net::Client::Latency_ms getLatency_ms() const;

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
    fge::net::Client::Latency_ms getLastPacketElapsedTime();

    /**
     * \brief Get a modulated timestamp of the current time
     *
     * \return The modulated timestamp
     */
    static fge::net::Client::Timestamp getTimestamp_ms();
    /**
     * \brief Compute the latency for the client->server / server->client with the given timestamps
     *
     * \param startedTime The timestamp of the sender packet
     * \param returnedTime The timestamp returned by the client/server
     * \return Latency in milliseconds
     */
    static fge::net::Client::Latency_ms computeLatency_ms(const fge::net::Client::Timestamp& startedTime,
                                                          const fge::net::Client::Timestamp& returnedTime);
    /**
     * \brief Compute the ping (latency of the client + latency of the server)
     *
     * \param startedTime The timestamp of the sender packet
     * \return The ping in milliseconds
     */
    static fge::net::Client::Latency_ms computePing_ms(const fge::net::Client::Timestamp& startedTime);

    /**
     * \brief Clear the packet queue
     */
    void clearPackets();
    /**
     * \brief Add a packet to the queue
     *
     * The packet will be sent when the network thread is ready to send it.
     * The network thread is ready to send a packet when the time interval between the last sent packet
     * is greater than the latency of the server->client.
     *
     * \param pck The packet to send with eventual options
     */
    void pushPacket(const fge::net::ClientSendQueuePacket& pck);
    /**
     * \brief Pop a packet from the queue
     *
     * \return The popped packet or nullptr if the queue is empty
     */
    fge::net::ClientSendQueuePacket popPacket();
    /**
     * \brief Check if the packet queue is empty
     *
     * \return True if the queue is empty, false otherwise
     */
    bool isPendingPacketsEmpty();

    fge::Event _event; ///< Optional client-side event that can be synchronized with the server
    fge::PropertyList _data; ///< Some user-defined client properties

private:
    fge::net::Client::Latency_ms g_latency_ms;
    std::chrono::steady_clock::time_point g_lastPacketTimePoint;

    std::queue<fge::net::ClientSendQueuePacket> g_pendingTransmitPackets;
    std::recursive_mutex g_mutex;

    fge::net::Skey g_skey;
};

/**
 * @}
 */

}//end fge::net

#endif // _FGE_C_CLIENT_HPP_INCLUDED
