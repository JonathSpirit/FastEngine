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
#include "FastEngine/network/C_packet.hpp"
#include <memory>
#include <optional>
#include <queue>
#include <vector>

#define FGE_NET_HEADERID_TYPE uint16_t
#define FGE_NET_HEADERID_MAX 0x3FFE
#define FGE_NET_HEADERID_DO_NOT_DISCARD_FLAG 0x8000
#define FGE_NET_HEADERID_DO_NOT_REORDER_FLAG 0x4000
#define FGE_NET_HEADERID_FLAGS_MASK 0xC000
#define FGE_NET_HEADERID_START 1
#define FGE_NET_BAD_HEADERID 0

#define FGE_NET_DEFAULT_REALM 0
#define FGE_NET_PACKET_REORDERER_CACHE_MAX 5

namespace fge::net
{

/**
 * \class ProtocolPacket
 * \ingroup network
 * \brief A special inheritance of Packet with a predefined communication protocol
 *
 * This class is used to handle the communication between the server and the client.
 */
class ProtocolPacket : public Packet
{
public:
    using HeaderId = FGE_NET_HEADERID_TYPE;
    using Realm = uint8_t;
    using CountId = uint16_t;
    constexpr static std::size_t HeaderSize = sizeof(HeaderId) + sizeof(Realm) + sizeof(CountId);
    constexpr static std::size_t HeaderIdPosition = 0;
    constexpr static std::size_t RealmPosition = sizeof(HeaderId);
    constexpr static std::size_t CountIdPosition = sizeof(HeaderId) + sizeof(Realm);

    inline ProtocolPacket(HeaderId headerId, Realm realmId, CountId countId);
    inline ProtocolPacket(ProtocolPacket const& r) = default;
    inline ProtocolPacket(Packet const& r) :
            Packet(r)
    {}
    inline ProtocolPacket(ProtocolPacket&& r) noexcept = default;
    inline ProtocolPacket(Packet&& r) noexcept :
            Packet(std::move(r))
    {}
    inline ~ProtocolPacket() override = default;

    [[nodiscard]] inline Packet& packet() noexcept { return *this; }
    [[nodiscard]] inline Packet const& packet() const noexcept { return *this; }

    [[nodiscard]] inline bool haveCorrectHeaderSize() const;
    [[nodiscard]] inline std::optional<HeaderId> retrieveHeaderId() const;
    [[nodiscard]] inline std::optional<Realm> retrieveRealm() const;
    [[nodiscard]] inline std::optional<CountId> retrieveCountId() const;

    inline void setHeaderId(HeaderId headerId);
    inline void setRealm(Realm realmId);
    inline void setCountId(CountId countId);
};

class FluxPacket;
using FluxPacketPtr = std::unique_ptr<FluxPacket>;

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
    PacketReorderer() = default;
    PacketReorderer(PacketReorderer const& r) = delete;
    PacketReorderer(PacketReorderer&& r) noexcept = default;
    ~PacketReorderer() = default;

    PacketReorderer& operator=(PacketReorderer const& r) = delete;
    PacketReorderer& operator=(PacketReorderer&& r) noexcept = default;

    void clear();

    void push(FluxPacketPtr&& fluxPacket);
    [[nodiscard]] bool isRetrievable(ProtocolPacket::CountId currentCountId, ProtocolPacket::Realm currentRealm);
    [[nodiscard]] FluxPacketPtr pop();

    [[nodiscard]] bool isEmpty() const;

private:
    struct Data
    {
        explicit Data(FluxPacketPtr&& fluxPacket);

        FluxPacketPtr _fluxPacket;
        ProtocolPacket::CountId _countId;
        ProtocolPacket::Realm _realm;

        struct Compare
        {
            [[nodiscard]] constexpr bool operator()(Data const& l, Data const& r) const
            {
                if (l._realm == r._realm)
                {
                    return l._countId > r._countId;
                }
                return l._realm > r._realm;
            }
        };
    };

    std::priority_queue<Data, std::vector<Data>, Data::Compare> g_cache;
    mutable bool g_retrievable{false};
};

} // namespace fge::net

#include "C_protocol.inl"

#endif // _FGE_C_PROTOCOL_HPP_INCLUDED
