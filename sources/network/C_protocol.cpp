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

#include "FastEngine/network/C_protocol.hpp"
#include "FastEngine/network/C_server.hpp"

namespace fge::net
{

void PacketReorderer::clear()
{
    this->g_forceRetrieve = false;
    decltype(this->g_cache)().swap(this->g_cache);
}

void PacketReorderer::push(FluxPacketPtr&& fluxPacket)
{
    this->g_cache.emplace(std::move(fluxPacket));
    if (this->g_cache.size() >= FGE_NET_PACKET_REORDERER_CACHE_MAX)
    {
        //We can't wait anymore for the correct packet to arrive
        //as the cache is full, the waiting packet is considered lost
        //and we can move on. (if he is still missing)
        this->g_forceRetrieve = true;
    }
}
PacketReorderer::Stats PacketReorderer::checkStat(FluxPacketPtr const& fluxPacket,
                                                  ProtocolPacket::CountId currentCountId,
                                                  ProtocolPacket::Realm currentRealm)
{
    auto const countId = fluxPacket->retrieveCountId().value();
    auto const realm = fluxPacket->retrieveRealm().value();

    if (realm < currentRealm && currentRealm + 1 != 0)
    {
        return Stats::OLD_REALM;
    }

    if (currentRealm != realm && countId != 0)
    { //Different realm, we can switch to the new realm only if the countId is 0 (first packet of the new realm)
        return Stats::WAITING_NEXT_REALM;
    }

    if (countId == currentCountId + 1)
    { //Same realm, we can switch to the next countId
        return Stats::RETRIEVABLE;
    }

    if (countId < currentCountId)
    { //We are missing a packet
        return Stats::OLD_COUNTID;
    }

    //We can't switch to the next countId, we wait for the correct packet to arrive
    return Stats::WAITING_NEXT_COUNTID;
}
bool PacketReorderer::isForced() const
{
    return this->g_forceRetrieve;
}
std::optional<PacketReorderer::Stats> PacketReorderer::checkStat(ProtocolPacket::CountId currentCountId,
                                                                 ProtocolPacket::Realm currentRealm) const
{
    if (this->g_cache.empty())
    {
        return std::nullopt;
    }

    return this->g_cache.top().checkStat(currentCountId, currentRealm);
}
FluxPacketPtr PacketReorderer::pop()
{
    if (this->g_cache.empty())
    {
        return nullptr;
    }

    auto fluxPacket = std::move(const_cast<FluxPacketPtr&>(this->g_cache.top()._fluxPacket));
    this->g_cache.pop();

    if (this->g_cache.empty())
    {
        this->g_forceRetrieve = false;
    }

    return fluxPacket;
}

bool PacketReorderer::isEmpty() const
{
    return this->g_cache.empty();
}

PacketReorderer::Data::Data(FluxPacketPtr&& fluxPacket) :
        _fluxPacket(std::move(fluxPacket)),
        _countId(_fluxPacket->retrieveCountId().value()),
        _realm(_fluxPacket->retrieveRealm().value())
{}
PacketReorderer::Data::Data(Data&& r) noexcept :
        _fluxPacket(std::move(r._fluxPacket)),
        _countId(r._countId),
        _realm(r._realm)
{}
PacketReorderer::Data::~Data() = default;

PacketReorderer::Data& PacketReorderer::Data::operator=(Data&& r) noexcept
{
    this->_fluxPacket = std::move(r._fluxPacket);
    this->_countId = r._countId;
    this->_realm = r._realm;
    return *this;
}

PacketReorderer::Stats PacketReorderer::Data::checkStat(ProtocolPacket::CountId currentCountId,
                                                        ProtocolPacket::Realm currentRealm) const
{
    if (this->_realm < currentRealm && currentRealm + 1 != 0)
    {
        return Stats::OLD_REALM;
    }

    if (currentRealm != this->_realm && this->_countId != 0)
    { //Different realm, we can switch to the new realm only if the countId is 0 (first packet of the new realm)
        return Stats::WAITING_NEXT_REALM;
    }

    if (this->_countId == currentCountId + 1)
    { //Same realm, we can switch to the next countId
        return Stats::RETRIEVABLE;
    }

    if (this->_countId < currentCountId)
    { //We are missing a packet
        return Stats::OLD_COUNTID;
    }

    //We can't switch to the next countId, we wait for the correct packet to arrive
    return Stats::WAITING_NEXT_COUNTID;
}

} // namespace fge::net
