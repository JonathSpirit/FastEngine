#ifndef _FGE_C_CLIENTLIST_HPP_INCLUDED
#define _FGE_C_CLIENTLIST_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_socket.hpp>
#include <FastEngine/C_client.hpp>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <deque>

namespace fge
{
namespace net
{

using ClientSharedPtr = std::shared_ptr<fge::net::Client>;

struct ClientListEvent
{
    enum Events : uint8_t
    {
        CLEVT_DELCLIENT = 0,
        CLEVT_NEWCLIENT
    };

    fge::net::ClientListEvent::Events _event;
    fge::net::Identity _id;
};

class FGE_API ClientList
{
public:
    using ClientListData = std::unordered_map<fge::net::Identity, fge::net::ClientSharedPtr, fge::net::IdentityHash>;
    using ClientEventList = std::deque<fge::net::ClientListEvent>;

    ClientList() = default;
    ~ClientList() = default;

    void clear();

    void sendToAll(fge::net::SocketUdp& socket, fge::net::Packet& pck);
    void sendToAll(const std::shared_ptr<fge::net::Packet>& pck);

    void add(const fge::net::Identity& id, const fge::net::ClientSharedPtr& newClient);
    void remove(const fge::net::Identity& id);

    fge::net::ClientSharedPtr get(const fge::net::Identity& id);

    fge::net::ClientList::ClientListData::iterator begin();
    fge::net::ClientList::ClientListData::const_iterator cbegin() const;
    fge::net::ClientList::ClientListData::iterator end();
    fge::net::ClientList::ClientListData::const_iterator cend() const;

    std::size_t getSize();
    std::mutex& getMutex();

    inline void watchEvent(bool on)
    {
        this->g_enableClientEventsFlag = on;
    }
    inline bool isWatchingEvent() const
    {
        return this->g_enableClientEventsFlag;
    }

    inline void pushClientEvent(const fge::net::ClientListEvent& evt)
    {
        this->g_events.push_back(evt);
    }
    inline const fge::net::ClientListEvent& getClientEvent(std::size_t index) const
    {
        return this->g_events[index];
    }
    inline std::size_t getClientEventSize() const
    {
        return this->g_events.size();
    }
    inline void clearClientEvent()
    {
        this->g_events.clear();
    }

private:
    fge::net::ClientList::ClientListData g_data;
    fge::net::ClientList::ClientEventList g_events;
    std::mutex g_mutex;
    bool g_enableClientEventsFlag = false;
};

}//end net
}//end fge


#endif // _FGE_C_CLIENTLIST_HPP_INCLUDED
