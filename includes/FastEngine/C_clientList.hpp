#ifndef _FGE_C_CLIENTLIST_HPP_INCLUDED
#define _FGE_C_CLIENTLIST_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_socket.hpp>
#include <FastEngine/C_client.hpp>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <deque>

namespace fge::net
{

using ClientSharedPtr = std::shared_ptr<fge::net::Client>;

/**
 * \struct ClientListEvent
 * \ingroup network
 * \brief Represents an event on the client list (client added, client removed, ...)
 */
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

/**
 * \class ClientList
 * \ingroup network
 * \brief A list of clients used by a server
 */
class FGE_API ClientList
{
public:
    using ClientListData = std::unordered_map<fge::net::Identity, fge::net::ClientSharedPtr, fge::net::IdentityHash>;
    using ClientEventList = std::deque<fge::net::ClientListEvent>;

    ClientList() = default;
    ~ClientList() = default;

    /**
     * \brief Clear the client list and the event list
     */
    void clear();

    /**
     * \brief Directly send a packet to every clients in the list
     *
     * This function sends the packet to every clients in the list without
     * passing through a network thread and checking the latency.
     *
     * \param socket The UDP socket to use to send the packet
     * \param pck The packet to send
     */
    void sendToAll(fge::net::SocketUdp& socket, fge::net::Packet& pck);
    /**
     * \brief Push a packet to every clients in the list
     *
     * \param pck The packet to push
     */
    void sendToAll(const fge::net::ClientSendQueuePacket& pck);

    /**
     * \brief Add a client to the list
     *
     * \param id The client's identity
     * \param newClient The client to add
     */
    void add(const fge::net::Identity& id, const fge::net::ClientSharedPtr& newClient);
    /**
     * \brief Remove a client from the list
     *
     * \param id The client's identity
     */
    void remove(const fge::net::Identity& id);

    /**
     * \brief Get a client from the list
     *
     * \param id The client's identity
     * \return The client if found, nullptr otherwise
     */
    fge::net::ClientSharedPtr get(const fge::net::Identity& id);

    fge::net::ClientList::ClientListData::iterator begin();
    fge::net::ClientList::ClientListData::const_iterator begin() const;
    fge::net::ClientList::ClientListData::iterator end();
    fge::net::ClientList::ClientListData::const_iterator end() const;

    /**
     * \brif Get the number of clients in the list
     *
     * \return Number of clients
     */
    std::size_t getSize();
    /**
     * \brief Get client list mutex
     *
     * \return The client list mutex
     */
    std::mutex& getMutex();

    /**
     * \brief Enable or disable the gathering of client events
     *
     * Default is disabled.
     *
     * \param on \b True to enable, \b false to disable
     */
    inline void watchEvent(bool on)
    {
        this->g_enableClientEventsFlag = on;
    }
    /**
     * \brief Check if the gathering of client events is enabled
     *
     * \return \b True if enabled, \b false otherwise
     */
    inline bool isWatchingEvent() const
    {
        return this->g_enableClientEventsFlag;
    }

    /**
     * \brief Manually push a client event
     *
     * \param evt A client event
     */
    inline void pushClientEvent(const fge::net::ClientListEvent& evt)
    {
        this->g_events.push_back(evt);
    }
    /**
     * \brief Get the client event with its index
     *
     * \return The client event
     */
    inline const fge::net::ClientListEvent& getClientEvent(std::size_t index) const
    {
        return this->g_events[index];
    }
    /**
     * \brief Get the number of client events
     *
     * \return The number of client events
     */
    inline std::size_t getClientEventSize() const
    {
        return this->g_events.size();
    }
    /**
     * \brief Clear the client event list
     *
     * The client event list should be cleared manually after
     * client checkup has been done.
     */
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

}//end fge::net


#endif // _FGE_C_CLIENTLIST_HPP_INCLUDED
