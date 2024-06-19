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
#include <optional>

#define FGE_NET_BAD_HEADER 0

namespace fge::net
{

/**
 * \ingroup network
 * @{
 */

/**
 * \class ProtocolPacket
 * \brief A special inheritance of Packet with a predefined communication protocol
 *
 * This class is used to handle the communication between the server and the client.
 */
class ProtocolPacket : public Packet
{
public:
    using HeaderId = uint16_t;
    using Realm = uint8_t;
    using CountId = uint16_t;
    constexpr static std::size_t HeaderSize = sizeof(HeaderId) + sizeof(Realm) + sizeof(CountId);
    constexpr static std::size_t HeaderIdPosition = 0;
    constexpr static std::size_t RealmPosition = sizeof(HeaderId);
    constexpr static std::size_t CountIdPosition = sizeof(HeaderId) + sizeof(Realm);

    inline ProtocolPacket(HeaderId headerId, Realm realmId, CountId countId);
    inline ProtocolPacket(ProtocolPacket const& r) = default;
    inline ProtocolPacket(Packet const& r) : Packet(r) {}
    inline ProtocolPacket(ProtocolPacket&& r) noexcept = default;
    inline ProtocolPacket(Packet&& r) noexcept : Packet(std::move(r)) {}
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

} // namespace fge::net

#include "C_protocol.inl"

#endif // _FGE_C_PROTOCOL_HPP_INCLUDED
