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

void PacketReorderer::push(FluxPacketPtr fluxPacket)
{
    this->g_cache.emplace(std::move(fluxPacket));
    if (this->g_cache.size() >= FGE_NET_PACKET_REORDERER_CACHE_MAX)
    {
        //We can't wait anymore for the correct packet to arrive
        //as the cache is full, the waiting packet is considered lost
        //and we can move on.
        this->g_retrievable = true;
    }
}
bool PacketReorderer::isRetrievable(ProtocolPacket::CountId currentCountId, ProtocolPacket::Realm currentRealm)
{
    if (this->g_cache.empty())
    {
        this->g_retrievable = false;
        return false;
    }

    if (this->g_retrievable)
    {
        return true;
    }

    if (this->g_cache.top()._realm < currentRealm && currentRealm+1 != 0)
    {
        //pop the packet if the realm is lower than the current realm
        this->g_cache.pop();
        if (this->g_cache.empty())
        {
            this->g_retrievable = false;
        }
        return false;
    }

    if (currentRealm != this->g_cache.top()._realm && this->g_cache.top()._countId != 0)
    {//Different realm, we can switch to the new realm only if the countId is 0 (first packet of the new realm)
        return false;
    }

    if (this->g_cache.top()._countId == currentCountId+1)
    {//Same realm, we can switch to the next countId
        return true;
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

PacketReorderer::Data::Data(FluxPacketPtr&& fluxPacket) :
        _fluxPacket(std::move(fluxPacket)),
        _countId(_fluxPacket->retrieveCountId().value()),
        _realm(_fluxPacket->retrieveRealm().value())
{}

} // namespace fge::net
