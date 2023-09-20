/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/C_clientList.hpp"
#include "FastEngine/C_socket.hpp"
#include <exception>

namespace fge::net
{

///ClientList
void ClientList::clear()
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    this->g_data.clear();
    this->clearClientEvent();
}

void ClientList::sendToAll(fge::net::SocketUdp& socket, fge::net::Packet& pck)
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    for (auto& it: this->g_data)
    {
        socket.sendTo(pck, it.first._ip, it.first._port);
    }
}
void ClientList::sendToAll(fge::net::SendQueuePacket const& pck)
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    for (auto& it: this->g_data)
    {
        it.second->pushPacket(pck);
    }
}

void ClientList::add(fge::net::Identity const& id, fge::net::ClientSharedPtr const& newClient)
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    this->g_data[id] = newClient;
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.push_back({fge::net::ClientListEvent::CLEVT_NEWCLIENT, id});
    }
}
void ClientList::remove(fge::net::Identity const& id)
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    this->g_data.erase(id);
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.push_back({fge::net::ClientListEvent::CLEVT_DELCLIENT, id});
    }
}
fge::net::ClientList::ClientListData::iterator
ClientList::remove(fge::net::ClientList::ClientListData::const_iterator itPos,
                   std::unique_lock<std::recursive_mutex> const& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &this->g_mutex)
    {
        throw fge::Exception("ClientList::remove : lock is not owned or not my mutex !");
    }
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.push_back({fge::net::ClientListEvent::CLEVT_DELCLIENT, itPos->first});
    }
    return this->g_data.erase(itPos);
}

fge::net::ClientSharedPtr ClientList::get(fge::net::Identity const& id) const
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    auto it = this->g_data.find(id);
    if (it != this->g_data.end())
    {
        return it->second;
    }
    return nullptr;
}

std::unique_lock<std::recursive_mutex> ClientList::acquireLock() const
{
    return std::unique_lock<std::recursive_mutex>(this->g_mutex);
}

fge::net::ClientList::ClientListData::iterator ClientList::begin(std::unique_lock<std::recursive_mutex> const& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &this->g_mutex)
    {
        throw fge::Exception("ClientList::begin : lock is not owned or not my mutex !");
    }
    return this->g_data.begin();
}
fge::net::ClientList::ClientListData::const_iterator
ClientList::begin(std::unique_lock<std::recursive_mutex> const& lock) const
{
    if (!lock.owns_lock() || lock.mutex() != &this->g_mutex)
    {
        throw fge::Exception("ClientList::begin : lock is not owned or not my mutex !");
    }
    return this->g_data.cbegin();
}
fge::net::ClientList::ClientListData::iterator ClientList::end(std::unique_lock<std::recursive_mutex> const& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &this->g_mutex)
    {
        throw fge::Exception("ClientList::begin : lock is not owned or not my mutex !");
    }
    return this->g_data.end();
}
fge::net::ClientList::ClientListData::const_iterator
ClientList::end(std::unique_lock<std::recursive_mutex> const& lock) const
{
    if (!lock.owns_lock() || lock.mutex() != &this->g_mutex)
    {
        throw fge::Exception("ClientList::begin : lock is not owned or not my mutex !");
    }
    return this->g_data.cend();
}

std::size_t ClientList::getSize() const
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    return this->g_data.size();
}

void ClientList::watchEvent(bool on)
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    this->g_enableClientEventsFlag = on;
}
bool ClientList::isWatchingEvent() const
{
    return this->g_enableClientEventsFlag;
}

void ClientList::pushClientEvent(fge::net::ClientListEvent const& evt)
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    this->g_events.push_back(evt);
}

fge::net::ClientListEvent const& ClientList::getClientEvent(std::size_t index) const
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    return this->g_events[index];
}
std::size_t ClientList::getClientEventSize() const
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    return this->g_events.size();
}

void ClientList::clearClientEvent()
{
    std::scoped_lock<std::recursive_mutex> lck(this->g_mutex);
    this->g_events.clear();
}

} // namespace fge::net
