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

#ifndef _FGE_C_CLIENTLIST_HPP_INCLUDED
#define _FGE_C_CLIENTLIST_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_client.hpp"
#include "C_protocol.hpp"
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace fge::net
{

class SocketUdp;

using ClientSharedPtr = std::shared_ptr<Client>;

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

    Events _event;
    Identity _id;
};

/**
 * \class ClientList
 * \ingroup network
 * \brief A list of clients used by a server
 */
class FGE_API ClientList
{
public:
    using ClientListData = std::unordered_map<Identity, ClientSharedPtr, IdentityHash>;
    using ClientEventList = std::deque<ClientListEvent>;

    ClientList() = default;
    ~ClientList() = default;

    /**
     * \brief Clear the client list and the event list
     */
    void clear();

    /**
     * \brief Directly send a packet to every client in the list
     *
     * This function sends the packet to every client in the list without
     * passing through a network thread and checking the latency.
     *
     * \param socket The UDP socket to use to send the packet
     * \param pck The packet to send
     */
    void sendToAll(SocketUdp& socket, Packet& pck) const;
    /**
     * \brief Push a packet to every client in the list
     *
     * \param pck The packet to push
     */
    void sendToAll(TransmissionPacketPtr const& pck) const;

    /**
     * \brief Add a client to the list
     *
     * \param id The client's identity
     * \param newClient The client to add
     */
    void add(Identity const& id, ClientSharedPtr const& newClient);
    /**
     * \brief Remove a client from the list
     *
     * \param id The client's identity
     */
    void remove(Identity const& id);
    /**
     * \brief Remove a client from the list
     *
     * You have to provide a valid reference to a unique lock acquire with
     * the method ClientList::acquireLock().
     *
     * \param itPos The client's iterator
     * \param lock A unique lock bound to this mutex
     * \return The iterator after the erased element
     */
    ClientListData::iterator remove(ClientListData::const_iterator itPos,
                                    std::unique_lock<std::recursive_mutex> const& lock);

    /**
     * \brief Get a client from the list
     *
     * \param id The client's identity
     * \return The client if found, nullptr otherwise
     */
    ClientSharedPtr get(Identity const& id) const;

    /**
     * \brief Acquire a unique lock, with the ClientList mutex
     *
     * In order to use iterators, you have to acquire a unique lock from this
     * ClientList.
     * The lock is not differed and will lock the mutex.
     *
     * \return A unique lock bound to this mutex
     */
    std::unique_lock<std::recursive_mutex> acquireLock() const;

    /**
     * \brief Get the begin iterator of the ClientList
     *
     * You have to provide a valid reference to a unique lock acquire with
     * the method ClientList::acquireLock().
     * This method will throw if one of this is not respected :
     * - The lock does not own the associated mutex.
     * - The mutex pointer of the lock does not correspond to this ClientList mutex.
     *
     * \param lock A unique lock bound to this mutex
     * \return The begin iterator
     */
    ClientListData::iterator begin(std::unique_lock<std::recursive_mutex> const& lock);
    ClientListData::const_iterator begin(std::unique_lock<std::recursive_mutex> const& lock) const;
    ClientListData::iterator end(std::unique_lock<std::recursive_mutex> const& lock);
    ClientListData::const_iterator end(std::unique_lock<std::recursive_mutex> const& lock) const;

    /**
     * \brief Get the number of clients in the list
     *
     * \return Number of clients
     */
    std::size_t getSize() const;

    /**
     * \brief Enable or disable the gathering of client events
     *
     * Default is disabled.
     *
     * \param on \b True to enable, \b false to disable
     */
    void watchEvent(bool on);
    /**
     * \brief Check if the gathering of client events is enabled
     *
     * \return \b True if enabled, \b false otherwise
     */
    bool isWatchingEvent() const;

    /**
     * \brief Manually push a client event
     *
     * \param evt A client event
     */
    void pushClientEvent(ClientListEvent const& evt);
    /**
     * \brief Get the client event with its index
     *
     * \return The client event
     */
    ClientListEvent const& getClientEvent(std::size_t index) const;
    /**
     * \brief Get the number of client events
     *
     * \return The number of client events
     */
    std::size_t getClientEventSize() const;
    /**
     * \brief Clear the client event list
     *
     * The client event list should be cleared manually after
     * client checkup has been done.
     */
    void clearClientEvent();

private:
    ClientListData g_data;
    ClientEventList g_events;
    mutable std::recursive_mutex g_mutex;
    bool g_enableClientEventsFlag = false;
};

} // namespace fge::net


#endif // _FGE_C_CLIENTLIST_HPP_INCLUDED
