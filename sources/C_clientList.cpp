/*
 * Copyright 2022 Guillaume Guillet
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

namespace fge
{
namespace net
{

///ClientList
void ClientList::clear()
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    this->g_data.clear();
    this->clearClientEvent();
}

void ClientList::sendToAll(fge::net::SocketUdp& socket, fge::net::Packet& pck)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    for (auto & it : this->g_data)
    {
        socket.sendTo(pck, it.first._ip, it.first._port);
    }
}
void ClientList::sendToAll(const fge::net::ClientSendQueuePacket& pck)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    for (auto & it : this->g_data)
    {
        it.second->pushPacket(pck);
    }
}

void ClientList::add(const fge::net::Identity& id, const fge::net::ClientSharedPtr& newClient)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    this->g_data[id] = newClient;
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.push_back({fge::net::ClientListEvent::CLEVT_NEWCLIENT, id});
    }
}
void ClientList::remove(const fge::net::Identity& id)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    this->g_data.erase(id);
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.push_back({fge::net::ClientListEvent::CLEVT_DELCLIENT, id});
    }
}

fge::net::ClientSharedPtr ClientList::get(const fge::net::Identity& id)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    auto it = this->g_data.find(id);
    if (it != this->g_data.end())
    {
        return it->second;
    }
    return nullptr;
}

fge::net::ClientList::ClientListData::iterator ClientList::begin()
{
    return this->g_data.begin();
}
fge::net::ClientList::ClientListData::const_iterator ClientList::begin() const
{
    return this->g_data.cbegin();
}
fge::net::ClientList::ClientListData::iterator ClientList::end()
{
    return this->g_data.end();
}
fge::net::ClientList::ClientListData::const_iterator ClientList::end() const
{
    return this->g_data.cend();
}

std::size_t ClientList::getSize()
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    return this->g_data.size();
}
std::mutex& ClientList::getMutex()
{
    return this->g_mutex;
}

}//end net
}//end fge
