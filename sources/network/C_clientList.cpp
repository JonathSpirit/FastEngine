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

#include "FastEngine/network/C_clientList.hpp"
#include "FastEngine/network/C_socket.hpp"

namespace fge::net
{

///ClientList
void ClientList::clear()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_data.clear();
    this->clearClientEvent();
}

void ClientList::sendToAll(SocketUdp& socket, Packet& pck) const
{
    std::scoped_lock const lck(this->g_mutex);
    for (auto& it: this->g_data)
    {
        socket.sendTo(pck, it.first._ip, it.first._port);
    }
}
void ClientList::sendToAll(TransmitPacketPtr const& pck) const
{
    std::scoped_lock const lck(this->g_mutex);
    for (auto& it: this->g_data)
    {
        it.second._client->pushPacket(std::make_unique<ProtocolPacket>(*pck));
    }
}

bool ClientList::moveTo(ClientList& targetList, Identity const& id)
{
    std::scoped_lock const lck(this->g_mutex);
    auto it = this->g_data.find(id);
    if (it == this->g_data.end())
    {
        return false;
    }

    std::scoped_lock const lckTarget(targetList.g_mutex);
    auto itTarget = targetList.g_data.find(id);
    if (itTarget != targetList.g_data.end())
    {
        return false;
    }

    targetList.g_data.emplace(id, std::move(it->second));
    this->g_data.erase(it);
    return true;
}

void ClientList::add(Identity const& id, ClientSharedPtr const& newClient)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_data.emplace(id, newClient);
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.emplace_back(Event::Types::EVT_NEWCLIENT, id);
    }
}
void ClientList::remove(Identity const& id)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_data.erase(id);
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.emplace_back(Event::Types::EVT_DELCLIENT, id);
    }
}
ClientList::DataList::iterator ClientList::remove(DataList::const_iterator itPos,
                                                  AccessLock<std::recursive_mutex> const& lock)
{
    lock.throwIfDifferent(this->g_mutex);
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.emplace_back(Event::Types::EVT_DELCLIENT, itPos->first);
    }
    return this->g_data.erase(itPos);
}

ClientSharedPtr ClientList::get(Identity const& id) const
{
    std::scoped_lock const lck(this->g_mutex);
    auto it = this->g_data.find(id);
    if (it != this->g_data.end())
    {
        return it->second._client;
    }
    return nullptr;
}
ClientList::Data const* ClientList::getData(Identity const& id) const
{
    std::scoped_lock const lck(this->g_mutex);
    auto it = this->g_data.find(id);
    if (it != this->g_data.end())
    {
        return &it->second;
    }
    return nullptr;
}
ClientList::Data* ClientList::getData(Identity const& id)
{
    std::scoped_lock const lck(this->g_mutex);
    auto it = this->g_data.find(id);
    if (it != this->g_data.end())
    {
        return &it->second;
    }
    return nullptr;
}

AccessLock<std::recursive_mutex> ClientList::acquireLock() const
{
    return AccessLock{this->g_mutex};
}

ClientList::DataList::iterator ClientList::begin(AccessLock<std::recursive_mutex> const& lock)
{
    lock.throwIfDifferent(this->g_mutex);
    return this->g_data.begin();
}
ClientList::DataList::const_iterator ClientList::begin(AccessLock<std::recursive_mutex> const& lock) const
{
    lock.throwIfDifferent(this->g_mutex);
    return this->g_data.cbegin();
}
ClientList::DataList::iterator ClientList::end(AccessLock<std::recursive_mutex> const& lock)
{
    lock.throwIfDifferent(this->g_mutex);
    return this->g_data.end();
}
ClientList::DataList::const_iterator ClientList::end(AccessLock<std::recursive_mutex> const& lock) const
{
    lock.throwIfDifferent(this->g_mutex);
    return this->g_data.cend();
}

std::size_t ClientList::getSize() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_data.size();
}

void ClientList::watchEvent(bool on)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_enableClientEventsFlag = on;
}
bool ClientList::isWatchingEvent() const
{
    return this->g_enableClientEventsFlag;
}

void ClientList::pushClientEvent(Event const& evt)
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_events.push_back(evt);
}

ClientList::Event const& ClientList::getClientEvent(std::size_t index) const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_events[index];
}
std::size_t ClientList::getClientEventSize() const
{
    std::scoped_lock const lck(this->g_mutex);
    return this->g_events.size();
}

void ClientList::clearClientEvent()
{
    std::scoped_lock const lck(this->g_mutex);
    this->g_events.clear();
}

} // namespace fge::net
