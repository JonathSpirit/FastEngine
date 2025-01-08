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

void PacketReorderer::push(ProtocolPacketPtr&& packet)
{
    this->g_cache.emplace(std::move(packet));
    if (this->g_cache.size() >= FGE_NET_PACKET_REORDERER_CACHE_MAX)
    {
        //We can't wait anymore for the correct packet to arrive
        //as the cache is full, the waiting packet is considered lost
        //and we can move on. (if he is still missing)
        this->g_forceRetrieve = true;
    }
}
PacketReorderer::Stats PacketReorderer::checkStat(ProtocolPacketPtr const& packet,
                                                  ProtocolPacket::CounterType currentCounter,
                                                  ProtocolPacket::RealmType currentRealm)
{
    auto const counter = packet->retrieveCounter().value();
    auto const realm = packet->retrieveRealm().value();

    if (realm < currentRealm && currentRealm + 1 != 0)
    {
        return Stats::OLD_REALM;
    }

    if (currentRealm != realm && counter != 0)
    { //Different realm, we can switch to the new realm only if the counter is 0 (first packet of the new realm)
        return Stats::WAITING_NEXT_REALM;
    }

    if (counter == currentCounter + 1)
    { //Same realm, we can switch to the next counter
        return Stats::RETRIEVABLE;
    }

    if (counter < currentCounter)
    { //We are missing a packet
        return Stats::OLD_COUNTID;
    }

    //We can't switch to the next counter, we wait for the correct packet to arrive
    return Stats::WAITING_NEXT_COUNTID;
}
bool PacketReorderer::isForced() const
{
    return this->g_forceRetrieve;
}
std::optional<PacketReorderer::Stats> PacketReorderer::checkStat(ProtocolPacket::CounterType currentCounter,
                                                                 ProtocolPacket::RealmType currentRealm) const
{
    if (this->g_cache.empty())
    {
        return std::nullopt;
    }

    return this->g_cache.top().checkStat(currentCounter, currentRealm);
}
ProtocolPacketPtr PacketReorderer::pop()
{
    if (this->g_cache.empty())
    {
        return nullptr;
    }

    auto packet = std::move(const_cast<ProtocolPacketPtr&>(this->g_cache.top()._packet));
    this->g_cache.pop();

    if (this->g_cache.empty())
    {
        this->g_forceRetrieve = false;
    }

    return packet;
}

bool PacketReorderer::isEmpty() const
{
    return this->g_cache.empty();
}

PacketReorderer::Data::Data(ProtocolPacketPtr&& packet) :
        _packet(std::move(packet)),
        _counter(_packet->retrieveCounter().value()),
        _realm(_packet->retrieveRealm().value())
{}
PacketReorderer::Data::Data(Data&& r) noexcept :
        _packet(std::move(r._packet)),
        _counter(r._counter),
        _realm(r._realm)
{}
PacketReorderer::Data::~Data() = default;

PacketReorderer::Data& PacketReorderer::Data::operator=(Data&& r) noexcept
{
    this->_packet = std::move(r._packet);
    this->_counter = r._counter;
    this->_realm = r._realm;
    return *this;
}

PacketReorderer::Stats PacketReorderer::Data::checkStat(ProtocolPacket::CounterType currentCounter,
                                                        ProtocolPacket::RealmType currentRealm) const
{
    if (this->_realm < currentRealm && currentRealm + 1 != 0)
    {
        return Stats::OLD_REALM;
    }

    if (currentRealm != this->_realm && this->_counter != 0)
    { //Different realm, we can switch to the new realm only if the counter is 0 (first packet of the new realm)
        return Stats::WAITING_NEXT_REALM;
    }

    if (this->_counter == currentCounter + 1)
    { //Same realm, we can switch to the next counter
        return Stats::RETRIEVABLE;
    }

    if (this->_counter < currentCounter)
    { //We are missing a packet
        return Stats::OLD_COUNTID;
    }

    //We can't switch to the next counter, we wait for the correct packet to arrive
    return Stats::WAITING_NEXT_COUNTID;
}

} // namespace fge::net
