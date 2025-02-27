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
#define FGE_NET_HEADER_LOCAL_REORDERED_FLAG 0x2000
#define FGE_NET_HEADER_DO_NOT_FRAGMENT_FLAG 0x1000
#define FGE_NET_HEADER_FLAGS_MASK 0xF000
#define FGE_NET_HEADER_FLAGS_COUNT 4

#define FGE_NET_ID_MAX ((~FGE_NET_HEADER_FLAGS_MASK) - 1)

#define FGE_NET_INTERNAL_ID_MAX 1024
#define FGE_NET_INTERNAL_ID_START 1

#define FGE_NET_CUSTOM_ID_MAX FGE_NET_ID_MAX
#define FGE_NET_CUSTOM_ID_START (FGE_NET_INTERNAL_ID_MAX + 1)

#define FGE_NET_BAD_ID 0

#define FGE_NET_DEFAULT_REALM 0
#define FGE_NET_PACKET_REORDERER_CACHE_MAX 5

#define FGE_NET_HANDSHAKE_STRING "FGE:HANDSHAKE:AZCgMVg4d4Sl2xYvZcqXqljIOqSrKX6H"

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
    };

    constexpr static std::size_t HeaderSize = sizeof(IdType) + sizeof(RealmType) + sizeof(CounterType);
    constexpr static std::size_t IdPosition = 0;
    constexpr static std::size_t RealmPosition = sizeof(IdType);
    constexpr static std::size_t CounterPosition = sizeof(IdType) + sizeof(RealmType);

    inline ProtocolPacket(Packet const& pck,
                          Identity const& id,
                          std::size_t fluxIndex = 0,
                          std::size_t fluxLifetime = 0);
    inline ProtocolPacket(Packet&& pck, Identity const& id, std::size_t fluxIndex = 0, std::size_t fluxLifetime = 0);
    inline ProtocolPacket(IdType header, RealmType realmId = FGE_NET_DEFAULT_REALM, CounterType countId = 0);

    inline ProtocolPacket(Packet const& r);
    inline ProtocolPacket(Packet&& r) noexcept;

    inline ProtocolPacket(ProtocolPacket const& r);
    inline ProtocolPacket(ProtocolPacket&& r) noexcept;

    inline ~ProtocolPacket() override = default;

    [[nodiscard]] inline Packet& packet() noexcept { return *this; }
    [[nodiscard]] inline Packet const& packet() const noexcept { return *this; }

    [[nodiscard]] inline bool haveCorrectHeaderSize() const;
    [[nodiscard]] inline std::optional<IdType> retrieveHeaderId() const;
    [[nodiscard]] inline std::optional<IdType> retrieveFlags() const;
    [[nodiscard]] inline std::optional<IdType> retrieveFullHeaderId() const;
    [[nodiscard]] inline std::optional<RealmType> retrieveRealm() const;
    [[nodiscard]] inline std::optional<CounterType> retrieveCounter() const;
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

    inline void setTimestamp(Timestamp timestamp);
    [[nodiscard]] inline Timestamp getTimeStamp() const;
    [[nodiscard]] inline Identity const& getIdentity() const;

    [[nodiscard]] inline std::vector<Option> const& options() const;
    [[nodiscard]] inline std::vector<Option>& options();

    inline void markForEncryption();
    [[nodiscard]] inline bool isMarkedForEncryption() const;

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

    [[nodiscard]] std::vector<std::shared_ptr<ProtocolPacket>> fragment(uint16_t mtu) const;

private:
    Identity g_identity{};
    Timestamp g_timestamp{0};

    std::size_t g_fluxIndex{0};
    std::size_t g_fluxLifetime{0};

    bool g_markedForEncryption{false};

    std::vector<Option> g_options;
};

using TransmitPacketPtr = std::shared_ptr<ProtocolPacket>;
using ReceivedPacketPtr = std::unique_ptr<ProtocolPacket>;

template<class... Args>
[[nodiscard]] inline TransmitPacketPtr CreatePacket(Args&&... args)
{
    return std::make_shared<ProtocolPacket>(std::forward<Args>(args)...);
}
template<class... Args>
[[nodiscard]] inline TransmitPacketPtr CreatePacket()
{
    return std::make_shared<ProtocolPacket>(FGE_NET_BAD_ID);
}

enum InternalProtocolIds : ProtocolPacket::IdType
{
    NET_INTERNAL_ID_MTU_ASK = FGE_NET_INTERNAL_ID_START,
    NET_INTERNAL_ID_MTU_ASK_RESPONSE,
    NET_INTERNAL_ID_MTU_TEST,
    NET_INTERNAL_ID_MTU_TEST_RESPONSE,
    NET_INTERNAL_ID_MTU_FINAL,

    NET_INTERNAL_FRAGMENTED_PACKET,

    NET_INTERNAL_ID_FGE_HANDSHAKE,
    NET_INTERNAL_ID_CRYPT_HANDSHAKE
};

struct InternalFragmentedPacketData
{
    uint8_t _fragmentTotal;
};

class PacketDefragmentation
{
public:
    PacketDefragmentation() = default;
    ~PacketDefragmentation() = default;

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
                                         ProtocolPacket::CounterType currentCounter,
                                         ProtocolPacket::RealmType currentRealm);
    [[nodiscard]] bool isForced() const;
    [[nodiscard]] std::optional<Stats> checkStat(ProtocolPacket::CounterType currentCounter,
                                                 ProtocolPacket::RealmType currentRealm) const;
    [[nodiscard]] ReceivedPacketPtr pop();

    [[nodiscard]] bool isEmpty() const;

private:
    struct FGE_API Data
    {
        explicit Data(ReceivedPacketPtr&& packet);
        Data(Data const& r) = delete;
        Data(Data&& r) noexcept;
        ~Data();

        Data& operator=(Data const& r) = delete;
        Data& operator=(Data&& r) noexcept;

        [[nodiscard]] Stats checkStat(ProtocolPacket::CounterType currentCounter,
                                      ProtocolPacket::RealmType currentRealm) const;

        ReceivedPacketPtr _packet;
        ProtocolPacket::CounterType _counter;
        ProtocolPacket::RealmType _realm;

        struct Compare
        {
            [[nodiscard]] constexpr bool operator()(Data const& l, Data const& r) const
            {
                if (l._realm == r._realm)
                {
                    return l._counter > r._counter;
                }
                return l._realm > r._realm;
            }
        };
    };

    std::priority_queue<Data, std::vector<Data>, Data::Compare> g_cache;
    bool g_forceRetrieve{false};
};

} // namespace fge::net

#include "C_protocol.inl"

#endif // _FGE_C_PROTOCOL_HPP_INCLUDED
