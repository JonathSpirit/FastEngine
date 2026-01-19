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

#ifndef _FGE_C_PROTOCOL_HPP_INCLUDED
#define _FGE_C_PROTOCOL_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/network/C_identity.hpp"
#include "FastEngine/network/C_packet.hpp"
#include <memory>
#include <optional>
#include <queue>
#include <vector>

#define FGE_NET_HEADER_DO_NOT_DISCARD_FLAG 0x8000
#define FGE_NET_HEADER_DO_NOT_REORDER_FLAG 0x4000
#define FGE_NET_HEADER_COMPRESSED_FLAG 0x2000
#define FGE_NET_HEADER_DO_NOT_FRAGMENT_FLAG 0x1000
#define FGE_NET_HEADER_FLAGS_MASK 0xF000
#define FGE_NET_HEADER_FLAGS_COUNT 4

#define FGE_NET_ID_MAX ((~FGE_NET_HEADER_FLAGS_MASK) - 1)

#define FGE_NET_INTERNAL_ID_MAX 1024
#define FGE_NET_INTERNAL_ID_START 1

#define FGE_NET_CUSTOM_ID_MAX FGE_NET_ID_MAX
#define FGE_NET_CUSTOM_ID_START (FGE_NET_INTERNAL_ID_MAX + 1)

#define FGE_NET_BAD_ID 0

#define FGE_NET_PACKET_CACHE_DELAY_FACTOR 2.2f
#define FGE_NET_PACKET_CACHE_MAX 100
#define FGE_NET_PACKET_CACHE_MIN_LATENCY_MS 10

#define FGE_NET_DEFAULT_REALM 0
#define FGE_NET_DEFAULT_PACKET_REORDERER_CACHE_SIZE 5
#define FGE_NET_PACKET_REORDERER_CACHE_COMPUTE(_clientReturnRate, _serverTickRate)                                     \
    ((static_cast<unsigned>(static_cast<float>(_clientReturnRate) * FGE_NET_PACKET_CACHE_DELAY_FACTOR /                \
                            static_cast<float>(_serverTickRate)) +                                                     \
      1) *                                                                                                             \
     2)

#define FGE_NET_HANDSHAKE_STRING "FGE:HANDSHAKE:AZCgMVg4d4Sl2xYvZcqXqljIOqSrKX6H"

namespace fge
{

class Compressor;

} // namespace fge

namespace fge::net
{

class Client;

using Timestamp = uint16_t;

/**
 * \class ProtocolPacket
 * \ingroup network
 * \brief A special inheritance of Packet with a predefined communication protocol
 *
 * This class is used to handle the communication between the server and the client
 * by storing a packet with its identity and timestamp.
 * This also add some data used internally by the server in order to handle multiple received packets.
 *
 * When transmitting a packet, the server will apply some options to the packet when sending it.
 *
 * \warning When the packet is pushed via Client::pushPacket or ClientList::sendToAll, the user must not modify
 * the packet/options anymore causing undefined behavior.
 */
class FGE_API ProtocolPacket : public Packet, public std::enable_shared_from_this<ProtocolPacket>
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

    using IdType = uint16_t;
    using RealmType = uint16_t;
    using CounterType = uint16_t;

    struct Header
    {
        IdType _id;
        RealmType _realm;
        CounterType _counter;
        CounterType _lastCounter;
    };

    constexpr static std::size_t HeaderSize = sizeof(IdType) + sizeof(RealmType) + sizeof(CounterType) * 2;
    constexpr static std::size_t IdPosition = 0;
    constexpr static std::size_t RealmPosition = sizeof(IdType);
    constexpr static std::size_t CounterPosition = sizeof(IdType) + sizeof(RealmType);
    constexpr static std::size_t ReorderedCounterPosition = sizeof(IdType) + sizeof(RealmType) + sizeof(CounterType);

    inline ProtocolPacket(Packet const& pck,
                          Identity const& id,
                          std::size_t fluxIndex = 0,
                          std::size_t fluxLifetime = 0);
    inline ProtocolPacket(Packet&& pck, Identity const& id, std::size_t fluxIndex = 0, std::size_t fluxLifetime = 0);
    inline ProtocolPacket(IdType header,
                          RealmType realmId = FGE_NET_DEFAULT_REALM,
                          CounterType countId = 0,
                          CounterType lastCountId = 0);

    inline ProtocolPacket(Packet const& r);
    inline ProtocolPacket(Packet&& r) noexcept;

    inline ProtocolPacket(ProtocolPacket const& r);
    inline ProtocolPacket(ProtocolPacket&& r) noexcept;

    inline ~ProtocolPacket() override = default;

    [[nodiscard]] inline Packet& packet() noexcept { return *this; }
    [[nodiscard]] inline Packet const& packet() const noexcept { return *this; }

    [[nodiscard]] inline bool haveCorrectHeader() const;
    [[nodiscard]] inline bool haveCorrectHeaderSize() const;
    [[nodiscard]] inline std::optional<IdType> retrieveHeaderId() const;
    [[nodiscard]] inline std::optional<IdType> retrieveFlags() const;
    [[nodiscard]] inline std::optional<IdType> retrieveFullHeaderId() const;
    [[nodiscard]] inline std::optional<RealmType> retrieveRealm() const;
    [[nodiscard]] inline std::optional<CounterType> retrieveCounter() const;
    [[nodiscard]] inline std::optional<CounterType> retrieveReorderedCounter() const;
    [[nodiscard]] inline std::optional<Header> retrieveHeader() const;

    [[nodiscard]] inline bool isFragmented() const;

    inline ProtocolPacket& setHeader(Header const& header);
    inline ProtocolPacket& setHeaderId(IdType id);

    inline ProtocolPacket& setFlags(IdType flags);
    inline ProtocolPacket& addFlags(IdType flags);
    inline ProtocolPacket& removeFlags(IdType flags);
    [[nodiscard]] inline bool checkFlags(IdType flags) const;
    inline ProtocolPacket& doNotDiscard();
    inline ProtocolPacket& doNotReorder();
    inline ProtocolPacket& doNotFragment();

    inline ProtocolPacket& setRealm(RealmType realm);
    inline ProtocolPacket& setCounter(CounterType counter);
    inline ProtocolPacket& setReorderedCounter(CounterType counter);

    inline void setTimestamp(Timestamp timestamp);
    [[nodiscard]] inline Timestamp getTimeStamp() const;
    [[nodiscard]] inline Identity const& getIdentity() const;

    [[nodiscard]] inline std::vector<Option> const& options() const;
    [[nodiscard]] inline std::vector<Option>& options();

    [[nodiscard]] bool compress(Compressor& compressor);
    [[nodiscard]] bool decompress(Compressor& compressor);

    inline void markForEncryption();
    inline void unmarkForEncryption();
    [[nodiscard]] inline bool isMarkedForEncryption() const;

    inline void markAsLocallyReordered();
    inline void unmarkAsLocallyReordered();
    [[nodiscard]] inline bool isMarkedAsLocallyReordered() const;

    inline void markAsCached();
    inline void unmarkAsCached();
    [[nodiscard]] inline bool isMarkedAsCached() const;

    /**
     * \brief Apply packet options to the packet
     *
     * \see Options
     *
     * \param client The client to apply the options
     */
    void applyOptions(Client const& client);
    /**
     * \brief Apply packet options to the packet
     *
     * Same as applyOptions(Client const& client) but without the client parameter.
     * UPDATE_CORRECTION_LATENCY will throw.
     */
    void applyOptions();

    /**
     * \brief Check if the flux lifetime is reached
     *
     * Increment the flux lifetime and return \b false if the lifetime is greater or equal to \p fluxSize.
     * Otherwise, increment the flux index and return \b true.
     * The flux index is incremented by 1 and modulo \p fluxSize.
     *
     * \param fluxSize The number of fluxes
     * \return \b true if lifetime is not reached, \b false otherwise
     */
    [[nodiscard]] inline bool checkFluxLifetime(std::size_t fluxSize);
    inline std::size_t getFluxIndex() const;
    inline std::size_t bumpFluxIndex(std::size_t fluxSize);

    [[nodiscard]] std::vector<std::unique_ptr<ProtocolPacket>> fragment(uint16_t mtu) const;

private:
    Identity g_identity{};
    Timestamp g_timestamp{0};

    std::size_t g_fluxIndex{0};
    std::size_t g_fluxLifetime{0};

    bool g_markedForEncryption{false};
    bool g_markedAsLocallyReordered{false};
    bool g_markedAsCached{false};

    std::vector<Option> g_options;
};

using TransmitPacketPtr = std::unique_ptr<ProtocolPacket>;
using ReceivedPacketPtr = std::unique_ptr<ProtocolPacket>;

template<class... Args>
[[nodiscard]] inline TransmitPacketPtr CreatePacket(Args&&... args)
{
    return std::make_unique<ProtocolPacket>(std::forward<Args>(args)...);
}
[[nodiscard]] inline TransmitPacketPtr CreatePacket()
{
    return std::make_unique<ProtocolPacket>(FGE_NET_BAD_ID);
}

enum InternalProtocolIds : ProtocolPacket::IdType
{
    NET_INTERNAL_ID_MTU_ASK = FGE_NET_INTERNAL_ID_START,
    NET_INTERNAL_ID_MTU_ASK_RESPONSE,
    NET_INTERNAL_ID_MTU_TEST,
    NET_INTERNAL_ID_MTU_TEST_RESPONSE,
    NET_INTERNAL_ID_MTU_FINAL,

    NET_INTERNAL_ID_FRAGMENTED_PACKET,

    NET_INTERNAL_ID_FGE_HANDSHAKE,
    NET_INTERNAL_ID_CRYPT_HANDSHAKE,

    NET_INTERNAL_ID_RETURN_PACKET,

    NET_INTERNAL_ID_DISCONNECT
};

[[nodiscard]] inline TransmitPacketPtr CreateDisconnectPacket()
{
    auto packet = std::make_unique<ProtocolPacket>(NET_INTERNAL_ID_DISCONNECT);
    packet->doNotDiscard().doNotReorder();
    return packet;
}

struct InternalFragmentedPacketData
{
    uint8_t _fragmentTotal;
};

class PacketDefragmentation
{
public:
    PacketDefragmentation() = default;
    PacketDefragmentation(PacketDefragmentation const& r) = delete;
    PacketDefragmentation(PacketDefragmentation&& r) noexcept = default;
    ~PacketDefragmentation() = default;

    PacketDefragmentation& operator=(PacketDefragmentation const& r) = delete;
    PacketDefragmentation& operator=(PacketDefragmentation&& r) noexcept = default;

    enum class Results
    {
        RETRIEVABLE,
        WAITING,
        DISCARDED
    };
    struct Result
    {
        Results _result;
        ProtocolPacket::RealmType _id;
    };

    void clear();

    [[nodiscard]] Result process(ReceivedPacketPtr&& packet);
    [[nodiscard]] ReceivedPacketPtr retrieve(ProtocolPacket::RealmType id, Identity const& client);

private:
    struct Data
    {
        Data(ProtocolPacket::RealmType id, ProtocolPacket::CounterType total) :
                _id(id),
                _count(1),
                _fragments(total)
        {}

        ProtocolPacket::RealmType _id;
        decltype(InternalFragmentedPacketData::_fragmentTotal) _count;
        std::vector<ReceivedPacketPtr> _fragments;
    };
    std::vector<Data> g_data;
};

/**
 * \class PacketReorderer
 * \ingroup network
 * \brief A packet reorderer
 *
 * This class is used to cache unordered packets and retrieve them in order later.
 */
class FGE_API PacketReorderer
{
public:
    enum class Stats
    {
        OLD_REALM,
        OLD_COUNTER,
        WAITING_NEXT_REALM,
        WAITING_NEXT_COUNTER,
        RETRIEVABLE
    };

    PacketReorderer() = default;
    PacketReorderer(PacketReorderer const& r) = delete;
    PacketReorderer(PacketReorderer&& r) noexcept = default;
    ~PacketReorderer() = default;

    PacketReorderer& operator=(PacketReorderer const& r) = delete;
    PacketReorderer& operator=(PacketReorderer&& r) noexcept = default;

    void clear();

    void push(ReceivedPacketPtr&& packet);
    [[nodiscard]] static Stats checkStat(ReceivedPacketPtr const& packet,
                                         ProtocolPacket::CounterType peerCounter,
                                         ProtocolPacket::CounterType peerReorderedCounter,
                                         ProtocolPacket::RealmType peerRealm);
    [[nodiscard]] bool isForced() const;
    [[nodiscard]] std::optional<Stats> checkStat(ProtocolPacket::CounterType peerCounter,
                                                 ProtocolPacket::CounterType peerReorderedCounter,
                                                 ProtocolPacket::RealmType peerRealm) const;
    [[nodiscard]] ReceivedPacketPtr pop();

    [[nodiscard]] bool isEmpty() const;

    void setMaximumSize(std::size_t size);
    [[nodiscard]] std::size_t getMaximumSize() const;

private:
    struct FGE_API Data
    {
        explicit Data(ReceivedPacketPtr&& packet);
        Data(Data const& r) = delete;
        Data(Data&& r) noexcept;
        ~Data();

        Data& operator=(Data const& r) = delete;
        Data& operator=(Data&& r) noexcept;

        [[nodiscard]] Stats checkStat(ProtocolPacket::CounterType peerCounter,
                                      ProtocolPacket::CounterType peerReorderedCounter,
                                      ProtocolPacket::RealmType peerRealm) const;

        ReceivedPacketPtr _packet;
        ProtocolPacket::CounterType _counter;
        ProtocolPacket::CounterType _reorderedCounter;
        ProtocolPacket::RealmType _realm;

        struct Compare
        {
            [[nodiscard]] constexpr bool operator()(Data const& l, Data const& r) const
            {
                if (l._realm == r._realm)
                {
                    return l._reorderedCounter > r._reorderedCounter;
                }
                return l._realm > r._realm;
            }
        };
    };

    std::priority_queue<Data, std::vector<Data>, Data::Compare> g_cache;
    std::size_t g_cacheSize{FGE_NET_DEFAULT_PACKET_REORDERER_CACHE_SIZE};
    bool g_forceRetrieve{false};
};

class FGE_API PacketCache
{
public:
    struct Label
    {
        constexpr Label() = default;
        constexpr Label(ProtocolPacket::CounterType counter, ProtocolPacket::RealmType realm) :
                _counter(counter),
                _realm(realm)
        {}

        ProtocolPacket::CounterType _counter{0};
        ProtocolPacket::RealmType _realm{0};

        [[nodiscard]] constexpr bool operator==(Label const& r) const
        {
            return this->_counter == r._counter && this->_realm == r._realm;
        }

        struct Hash
        {
            [[nodiscard]] inline std::size_t operator()(Label const& label) const
            {
                static_assert(sizeof(label._counter) == sizeof(label._realm) && sizeof(label._counter) == 2,
                              "ProtocolPacket::CounterType and ProtocolPacket::RealmType must be 16 bits");
                return std::hash<std::size_t>()(static_cast<std::size_t>(label._counter) << 16 |
                                                static_cast<std::size_t>(label._realm));
            }
        };
    };

    PacketCache() = default;
    PacketCache(PacketCache const& r) = delete;
    PacketCache(PacketCache&& r) noexcept;
    ~PacketCache() = default;

    PacketCache& operator=(PacketCache const& r) = delete;
    PacketCache& operator=(PacketCache&& r) noexcept;

    void clear();
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] bool isEnabled() const;
    void enable(bool enable);

    //Transmit
    void push(TransmitPacketPtr const& packet);

    //Receive
    void acknowledgeReception(std::span<Label> labels);

    //Check for unacknowledged packet
    [[nodiscard]] bool check(std::chrono::steady_clock::time_point const& timePoint,
                             std::chrono::milliseconds clientDelay);
    [[nodiscard]] TransmitPacketPtr pop();

private:
    struct Data
    {
        Data() = default;
        explicit Data(TransmitPacketPtr&& packet);

        Data& operator=(TransmitPacketPtr&& packet);

        TransmitPacketPtr _packet;
        Label _label;
        std::chrono::steady_clock::time_point _time{};
    };

    mutable std::mutex g_mutex;

    //Circular buffer
    std::vector<Data> g_cache{FGE_NET_PACKET_CACHE_MAX};
    std::size_t g_start{0};
    std::size_t g_end{0};

    bool g_enable{false};
};

} // namespace fge::net

#include "C_protocol.inl"

#endif // _FGE_C_PROTOCOL_HPP_INCLUDED
