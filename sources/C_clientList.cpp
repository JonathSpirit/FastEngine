#include "FastEngine/C_clientList.hpp"

namespace fge
{
namespace net
{

///ClientList
void FGE_API ClientList::clear()
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    this->g_data.clear();
    this->clearClientEvent();
}

void FGE_API ClientList::sendToAll(fge::net::SocketUdp& socket, fge::net::Packet& pck)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    for (auto & it : this->g_data)
    {
        socket.sendTo(pck, it.first._ip, it.first._port);
    }
}
void FGE_API ClientList::sendToAll(const fge::net::ClientSendQueuePacket& pck)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    for (auto & it : this->g_data)
    {
        it.second->pushPacket(pck);
    }
}

void FGE_API ClientList::add(const fge::net::Identity& id, const fge::net::ClientSharedPtr& newClient)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    this->g_data[id] = newClient;
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.push_back({fge::net::ClientListEvent::CLEVT_NEWCLIENT, id});
    }
}
void FGE_API ClientList::remove(const fge::net::Identity& id)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    this->g_data.erase(id);
    if (this->g_enableClientEventsFlag)
    {
        this->g_events.push_back({fge::net::ClientListEvent::CLEVT_DELCLIENT, id});
    }
}

fge::net::ClientSharedPtr FGE_API ClientList::get(const fge::net::Identity& id)
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    auto it = this->g_data.find(id);
    if (it != this->g_data.end())
    {
        return it->second;
    }
    return nullptr;
}

fge::net::ClientList::ClientListData::iterator FGE_API ClientList::begin()
{
    return this->g_data.begin();
}
fge::net::ClientList::ClientListData::const_iterator FGE_API ClientList::cbegin() const
{
    return this->g_data.cbegin();
}
fge::net::ClientList::ClientListData::iterator FGE_API ClientList::end()
{
    return this->g_data.end();
}
fge::net::ClientList::ClientListData::const_iterator FGE_API ClientList::cend() const
{
    return this->g_data.cend();
}

std::size_t FGE_API ClientList::getSize()
{
    std::lock_guard<std::mutex> lck(this->g_mutex);
    return this->g_data.size();
}
std::mutex& FGE_API ClientList::getMutex()
{
    return this->g_mutex;
}

}//end net
}//end fge
