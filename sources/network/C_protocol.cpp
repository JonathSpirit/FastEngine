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
    this->g_retrievable = false;
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
        this->g_retrievable = true;
    }
}
bool PacketReorderer::isRetrievable(ProtocolPacket::CountId currentCountId,
                                    ProtocolPacket::Realm currentRealm,
                                    Client& client)
{
    if (this->g_cache.empty())
    {
        this->g_retrievable = false;
        return false;
    }

    auto const stat = this->g_cache.top().checkStat(currentCountId, currentRealm);

#ifdef FGE_DEF_SERVER
    if (stat == Data::Stats::OLD_COUNTID)
#else
    if (stat == Data::Stats::OLD_REALM || stat == Data::Stats::OLD_COUNTID)
#endif
    {
        //check discard flag
        auto const headerId = this->g_cache.top()._fluxPacket->retrieveHeaderId().value();
        if ((headerId & FGE_NET_HEADERID_DO_NOT_DISCARD_FLAG) > 0)
        {
            return false;
        }

        //pop the old packet
        client.advanceLostPacketCount();
        this->g_cache.pop();
        if (this->g_cache.empty())
        {
            this->g_retrievable = false;
        }
        return false;
    }

    if (this->g_retrievable)
    {
        return true;
    }

    return stat == Data::Stats::RETRIEVABLE;
}
bool PacketReorderer::verifyContinuity(FluxPacketPtr const& fluxPacket,
                                       ProtocolPacket::CountId currentCountId,
                                       ProtocolPacket::Realm currentRealm)
{
    auto const headerId = fluxPacket->retrieveHeaderId().value();
    if ((headerId & FGE_NET_HEADERID_DO_NOT_REORDER_FLAG) > 0)
    {
        return true;
    }

    auto const countId = fluxPacket->retrieveCountId().value();
    auto const realm = fluxPacket->retrieveRealm().value();

    if (realm < currentRealm && currentRealm + 1 != 0)
    {
        return false;
    }

    if (currentRealm != realm && countId != 0)
    { //Different realm, we can switch to the new realm only if the countId is 0 (first packet of the new realm)
        return false;
    }

    if (countId == currentCountId + 1)
    { //Same realm, we can switch to the next countId
        return true;
    }

    if (currentCountId < countId)
    { //We are missing a packet
        return false;
    }

    //We can't switch to the next countId, we wait for the correct packet to arrive
    return false;
}
FluxPacketPtr PacketReorderer::pop()
{
    if (this->g_cache.empty())
    {
        return nullptr;
    }

    auto fluxPacket = std::move(const_cast<FluxPacketPtr&>(this->g_cache.top()._fluxPacket));
    this->g_cache.pop();
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

PacketReorderer::Data::Stats PacketReorderer::Data::checkStat(ProtocolPacket::CountId currentCountId,
                                                              ProtocolPacket::Realm currentRealm) const
{
    if (this->_realm < currentRealm && currentRealm + 1 != 0)
    {
        return Stats::OLD_REALM;
    }

    if (currentRealm != this->_realm && this->_countId != 0)
    { //Different realm, we can switch to the new realm only if the countId is 0 (first packet of the new realm)
        return Stats::WAIT_NEXT_REALM;
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
    return Stats::WAIT_NEXT_COUNTID;
}

} // namespace fge::net
