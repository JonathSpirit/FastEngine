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

#include "FastEngine/network/C_server.hpp"
#include "FastEngine/C_alloca.hpp"
#include "FastEngine/manager/network_manager.hpp"

namespace fge::net
{

//NetCommands

NetCommandResults
NetMTUCommand::transmit(TransmissionPacketPtr& buffPacket, SocketUdp const& socket, [[maybe_unused]] Client& client)
{
    switch (this->g_state)
    {
    case States::ASKING:
        buffPacket = TransmissionPacket::create(NET_INTERNAL_ID_MTU_ASK);
        buffPacket->doNotDiscard().doNotReorder();
        this->g_state = States::WAITING_RESPONSE;
        break;
    case States::DISCOVER:
    {
        //Transmit the new target MTU
        buffPacket = TransmissionPacket::create(NET_INTERNAL_ID_MTU_TEST);
        auto const currentSize = buffPacket->doNotDiscard().doNotReorder().packet().getDataSize();

        auto const extraHeader = (socket.getAddressType() == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_HEADER_SIZE
                                                                                    : FGE_SOCKET_IPV6_HEADER_SIZE) +
                                 FGE_SOCKET_UDP_HEADER_SIZE;

        --this->g_tryCount;
        if (this->g_tryCount == 0 && this->g_currentMTU == 0)
        {
            //Last try
            this->g_targetMTU = socket.getAddressType() == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_MIN_MTU
                                                                                  : FGE_SOCKET_IPV6_MIN_MTU;
        }
        buffPacket->packet().append(this->g_targetMTU - currentSize - extraHeader);
        this->g_state = States::WAITING;
        break;
    }
    default:
        break;
    }

    return NetCommandResults::WORKING;
}
NetCommandResults NetMTUCommand::receive(std::unique_ptr<ProtocolPacket>& packet,
                                         SocketUdp const& socket,
                                         std::chrono::milliseconds deltaTime)
{
    switch (this->g_state)
    {
    case States::WAITING_RESPONSE:
        if (packet && packet->retrieveHeaderId().value() == NET_INTERNAL_ID_MTU_ASK_RESPONSE)
        {
            //Extract the target MTU
            uint16_t targetMTU;
            if (rules::RValid<uint16_t>({packet->packet(), &targetMTU}).end() || !packet->endReached())
            {
                //Invalid packet
                this->g_promise.set_value(0);
                return NetCommandResults::FAILURE;
            }

            auto const socketType = socket.getAddressType();
            auto const ourCurrentMTU = socket.retrieveCurrentAdapterMTU().value_or(FGE_SOCKET_FULL_DATAGRAM_SIZE);
            if (targetMTU == 0)
            {
                //We have to figure it ourselves
                this->g_maximumMTU = ourCurrentMTU;
            }
            else
            {
                this->g_maximumMTU = std::min(targetMTU, ourCurrentMTU);
            }

            this->g_currentMTU =
                    socketType == IpAddress::Types::Ipv4 ? FGE_SOCKET_IPV4_MIN_MTU : FGE_SOCKET_IPV6_MIN_MTU;
            if (this->g_currentMTU == this->g_maximumMTU)
            {
                this->g_promise.set_value(this->g_currentMTU);
                return NetCommandResults::SUCCESS;
            }

            //Compute a new target MTU
            this->g_targetMTU = this->g_maximumMTU;

            std::size_t const diff = this->g_maximumMTU - this->g_currentMTU;
            if (diff < FGE_SERVER_MTU_MIN_INTERVAL)
            {
                this->g_tryCount = 0;
            }
            else
            {
                this->g_tryCount = FGE_SERVER_MTU_TRY_COUNT;
                this->g_intervalMTU = diff / 2;
            }

            this->g_receiveTimeout = std::chrono::milliseconds::zero();
            this->g_state = States::DISCOVER;
            return NetCommandResults::WORKING;
        }

        this->g_receiveTimeout += deltaTime;
        if (this->g_receiveTimeout >= FGE_SERVER_MTU_TIMEOUT_MS)
        {
            this->g_promise.set_value(0);
            return NetCommandResults::FAILURE;
        }
        break;
    case States::DISCOVER:
    case States::WAITING:
        if (packet && packet->retrieveHeaderId().value() == NET_INTERNAL_ID_MTU_TEST_RESPONSE)
        {
            this->g_currentMTU = this->g_targetMTU;

            if (this->g_tryCount == 0 || this->g_currentMTU == this->g_maximumMTU)
            {
                this->g_promise.set_value(this->g_currentMTU);
                return this->g_currentMTU == 0 ? NetCommandResults::FAILURE : NetCommandResults::SUCCESS;
            }

            this->g_targetMTU += this->g_intervalMTU;
            this->g_intervalMTU = std::max<uint16_t>(FGE_SERVER_MTU_MIN_INTERVAL, this->g_intervalMTU / 2);
            if (this->g_targetMTU > this->g_maximumMTU)
            {
                this->g_targetMTU = this->g_maximumMTU;
                this->g_tryCount = 0;
            }

            this->g_receiveTimeout = std::chrono::milliseconds::zero();
            this->g_state = States::DISCOVER;
            break;
        }

        this->g_receiveTimeout += deltaTime;
        if (this->g_receiveTimeout >= FGE_SERVER_MTU_TIMEOUT_MS)
        {
            if (this->g_tryCount == 0)
            {
                this->g_promise.set_value(this->g_currentMTU);
                return this->g_currentMTU == 0 ? NetCommandResults::FAILURE : NetCommandResults::SUCCESS;
            }

            this->g_targetMTU -= this->g_intervalMTU;
            this->g_intervalMTU = std::max<uint16_t>(FGE_SERVER_MTU_MIN_INTERVAL, this->g_intervalMTU / 2);

            this->g_receiveTimeout = std::chrono::milliseconds::zero();
            this->g_state = States::DISCOVER;
            return NetCommandResults::WORKING;
        }
        break;
    default:
        break;
    }

    return NetCommandResults::WORKING;
}

//ServerFluxUdp
NetFluxUdp::~NetFluxUdp()
{
    this->clearPackets();
}

void NetFluxUdp::clearPackets()
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.clear();
    this->_g_remainingPackets = 0;
}
bool NetFluxUdp::pushPacket(ProtocolPacketPtr&& fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    if (this->_g_packets.size() >= this->g_maxPackets)
    {
        return false;
    }
    this->_g_packets.push_back(std::move(fluxPck));
    return true;
}
void NetFluxUdp::forcePushPacket(ProtocolPacketPtr fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.push_back(std::move(fluxPck));
}
void NetFluxUdp::forcePushPacketFront(ProtocolPacketPtr fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.push_front(std::move(fluxPck));
}

FluxProcessResults NetFluxUdp::processReorder(Client& client,
                                              ProtocolPacketPtr& packet,
                                              ProtocolPacket::CounterType currentCounter,
                                              bool ignoreRealm)
{
    auto const currentRealm = ignoreRealm ? packet->retrieveRealm().value() : client.getCurrentRealm();

    if (client.getPacketReorderer().isEmpty())
    {
        auto const stat = PacketReorderer::checkStat(packet, currentCounter, currentRealm);

        if (stat == PacketReorderer::Stats::RETRIEVABLE)
        {
            return FluxProcessResults::RETRIEVABLE;
        }

        client.getPacketReorderer().push(std::move(packet));
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //We push the packet in the reorderer
    client.getPacketReorderer().push(std::move(packet));

    //At this point we are sure that the reorderer contains at least 2 packets

    bool forced = client.getPacketReorderer().isForced();
    auto const stat = client.getPacketReorderer().checkStat(currentCounter, currentRealm).value();
    if (!forced && stat != PacketReorderer::Stats::RETRIEVABLE)
    {
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    packet = client.getPacketReorderer().pop();
    packet->addFlags(FGE_NET_HEADER_LOCAL_REORDERED_FLAG);

    std::size_t containerInversedSize = 0;
    auto* containerInversed = FGE_ALLOCA_T(ProtocolPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX);
    FGE_PLACE_CONSTRUCT(ProtocolPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);

    while (auto const stat = client.getPacketReorderer().checkStat(currentCounter, currentRealm))
    {
        if (!(stat == PacketReorderer::Stats::RETRIEVABLE || forced))
        {
            break;
        }

        auto reorderedPacket = client.getPacketReorderer().pop();

        //Add the LOCAL_REORDERED_FLAG to the header
        reorderedPacket->addFlags(FGE_NET_HEADER_LOCAL_REORDERED_FLAG);

        //Add it to the container (we are going to push in front of the flux queue, so we need to inverse the order)
        containerInversed[containerInversedSize++] = std::move(reorderedPacket);
    }

    //Now we push the packets (until the last one) in the correct order in the flux queue
    for (std::size_t i = containerInversedSize; i != 0; --i)
    {
        this->forcePushPacketFront(std::move(containerInversed[i - 1]));
        ++this->_g_remainingPackets;
    }

    FGE_PLACE_DESTRUCT(ProtocolPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);

    return FluxProcessResults::RETRIEVABLE;
}

ProtocolPacketPtr NetFluxUdp::popNextPacket()
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    if (!this->_g_packets.empty())
    {
        ProtocolPacketPtr tmpPck = std::move(this->_g_packets.front());
        this->_g_packets.pop_front();
        return tmpPck;
    }
    return nullptr;
}
std::size_t NetFluxUdp::getPacketsSize() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->_g_packets.size();
}
bool NetFluxUdp::isEmpty() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->_g_packets.empty();
}

void NetFluxUdp::setMaxPackets(std::size_t n)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->g_maxPackets = n;
}
std::size_t NetFluxUdp::getMaxPackets() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->g_maxPackets;
}

//ServerNetFluxUdp
FluxProcessResults
ServerNetFluxUdp::process(ClientSharedPtr& refClient, ProtocolPacketPtr& packet, bool allowUnknownClient)
{
    refClient.reset();
    packet.reset();

    if (this->_g_remainingPackets == 0)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //Popping the next packet
    packet = this->popNextPacket();
    if (!packet)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }
    --this->_g_remainingPackets;

    //Verify if the client is known
    auto* refClientData = this->_clients.getData(packet->getIdentity());

    if (refClientData == nullptr)
    {
        if (allowUnknownClient)
        {
            //Check if the packet is fragmented
            if (packet->isFragmented())
            { //We can't (disallow) process fragmented packets from unknown clients
                return FluxProcessResults::NOT_RETRIEVABLE;
            }
            return FluxProcessResults::RETRIEVABLE;
        }

        this->g_server->repushPacket(std::move(packet));
        return FluxProcessResults::NOT_RETRIEVABLE;
    }
    refClient = refClientData->_client;

    //Check if the packet is a fragment
    if (packet->isFragmented())
    {
        auto const identity = packet->getIdentity();
        auto const result = refClientData->_defragmentation.process(std::move(packet));
        if (result._result == PacketDefragmentation::Results::RETRIEVABLE)
        {
            packet = refClientData->_defragmentation.retrieve(result._id, identity);
        }
        else
        {
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
    }

    auto const headerFlags = packet->retrieveFlags().value();

    if ((headerFlags & FGE_NET_HEADER_DO_NOT_REORDER_FLAG) > 0)
    {
        return FluxProcessResults::RETRIEVABLE;
    }

    if ((headerFlags & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) == 0)
    {
        auto reorderResult = this->processReorder(*refClient, packet, refClient->getClientPacketCounter(), true);
        if (reorderResult != FluxProcessResults::RETRIEVABLE)
        {
            return reorderResult;
        }
    }

    //Verify the realm
    if (!this->verifyRealm(refClient, packet))
    {
        return FluxProcessResults::BAD_REALM;
    }

    auto const stat =
            PacketReorderer::checkStat(packet, refClient->getClientPacketCounter(), refClient->getCurrentRealm());

    if ((headerFlags & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) == 0)
    {
        if (stat == PacketReorderer::Stats::OLD_COUNTER)
        {
            refClient->advanceLostPacketCount();
            --this->_g_remainingPackets;
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
    }

    if (stat == PacketReorderer::Stats::WAITING_NEXT_REALM || stat == PacketReorderer::Stats::WAITING_NEXT_COUNTER)
    {
        refClient->advanceLostPacketCount(); //We are missing a packet
    }

    auto const countId = packet->retrieveCounter().value();
    refClient->setClientPacketCounter(countId);

    return FluxProcessResults::RETRIEVABLE;
}

bool ServerNetFluxUdp::verifyRealm(ClientSharedPtr const& refClient, ProtocolPacketPtr const& packet)
{
    auto const headerFlags = packet->retrieveFlags().value();
    auto const clientProvidedRealm = packet->retrieveRealm().value();

    if (clientProvidedRealm != refClient->getCurrentRealm() &&
        refClient->getLastRealmChangeElapsedTime() >= FGE_SERVER_MAX_TIME_DIFFERENCE_REALM)
    {
        this->_onClientBadRealm.call(refClient);
        return (headerFlags & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) > 0;
    }
    return true;
}

//ServerUdp
ServerSideNetUdp::ServerSideNetUdp(IpAddress::Types type) :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_defaultFlux(*this),
        g_socket(type),
        g_running(false)
{}
ServerSideNetUdp::~ServerSideNetUdp()
{
    this->stop();
}

void ServerSideNetUdp::stop()
{
    if (this->g_running)
    {
        this->g_running = false;

        this->g_threadReception->join();
        this->g_threadTransmission->join();

        this->g_threadReception = nullptr;
        this->g_threadTransmission = nullptr;

        this->g_socket.close();

        //Clear the flux
        std::scoped_lock const lock(this->g_mutexServer);
        for (auto& flux: this->g_fluxes)
        {
            flux->clearPackets();
        }
        this->g_defaultFlux.clearPackets();
        decltype(this->g_transmissionQueue)().swap(this->g_transmissionQueue);
    }
}

ServerNetFluxUdp* ServerSideNetUdp::newFlux()
{
    std::scoped_lock const lock(this->g_mutexServer);

    this->g_fluxes.push_back(std::make_unique<ServerNetFluxUdp>(*this));
    return this->g_fluxes.back().get();
}
ServerNetFluxUdp* ServerSideNetUdp::getFlux(std::size_t index)
{
    std::scoped_lock const lock(this->g_mutexServer);

    if (index >= this->g_fluxes.size())
    {
        return nullptr;
    }
    return this->g_fluxes[index].get();
}
ServerNetFluxUdp* ServerSideNetUdp::getDefaultFlux()
{
    return &this->g_defaultFlux;
}
std::size_t ServerSideNetUdp::getFluxSize() const
{
    return this->g_fluxes.size();
}
IpAddress::Types ServerSideNetUdp::getAddressType() const
{
    return this->g_socket.getAddressType();
}
void ServerSideNetUdp::closeFlux(NetFluxUdp* flux)
{
    std::scoped_lock const lock(this->g_mutexServer);

    for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
    {
        if (this->g_fluxes[i].get() == flux)
        {
            this->g_fluxes.erase(this->g_fluxes.begin() + i);
            break;
        }
    }
}
void ServerSideNetUdp::closeAllFlux()
{
    std::scoped_lock const lock(this->g_mutexServer);
    this->g_fluxes.clear();
}

void ServerSideNetUdp::repushPacket(ProtocolPacketPtr&& packet)
{
    if (!packet->checkFluxLifetime(this->g_fluxes.size()))
    {
        this->g_defaultFlux.pushPacket(std::move(packet));
        return;
    }
    this->g_fluxes[packet->getFluxIndex()]->forcePushPacket(std::move(packet));
}

void ServerSideNetUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

bool ServerSideNetUdp::isRunning() const
{
    return this->g_running;
}

void ServerSideNetUdp::sendTo(TransmissionPacketPtr& pck, Client const& client, Identity const& id)
{
    pck->applyOptions(client);
    pck->doNotReorder();

    {
        std::scoped_lock const lock(this->g_mutexServer);
        this->g_transmissionQueue.emplace(std::move(pck), id);
    }
    this->g_transmissionNotifier.notify_one();
}
void ServerSideNetUdp::sendTo(TransmissionPacketPtr& pck, Identity const& id)
{
    pck->applyOptions();
    pck->doNotReorder();

    {
        std::scoped_lock const lock(this->g_mutexServer);
        this->g_transmissionQueue.emplace(std::move(pck), id);
    }
    this->g_transmissionNotifier.notify_one();
}

void ServerSideNetUdp::threadReception()
{
    Identity idReceive;
    Packet pckReceive; //TODO
    std::size_t pushingIndex = 0;

    while (this->g_running)
    {
        if (this->g_socket.select(true, FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS) == Socket::Errors::ERR_NOERROR)
        {
            if (this->g_socket.receiveFrom(pckReceive, idReceive._ip, idReceive._port) == Socket::Errors::ERR_NOERROR)
            {
#ifdef FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST
                if (fge::_random.range(0, 1000) <= 10)
                {
                    continue;
                }
#endif

                std::scoped_lock const lck(this->g_mutexServer);

                if (pckReceive.getDataSize() < ProtocolPacket::HeaderSize)
                { //Bad header, packet is dismissed
                    continue;
                }

                //Skip the header for reading
                pckReceive.skip(ProtocolPacket::HeaderSize);
                auto packet = std::make_unique<ProtocolPacket>(std::move(pckReceive), idReceive);
                packet->setTimestamp(Client::getTimestamp_ms());

                //Verify headerId
                auto const headerId = packet->retrieveFullHeaderId().value();
                if ((headerId & ~FGE_NET_HEADER_FLAGS_MASK) == FGE_NET_BAD_ID ||
                    (headerId & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) > 0)
                { //Bad headerId, packet is dismissed
                    continue;
                }

                //Checking internal header ids
                switch (headerId & ~FGE_NET_HEADER_FLAGS_MASK)
                {
                case NET_INTERNAL_ID_MTU_TEST:
                {
                    auto response = TransmissionPacket::create(NET_INTERNAL_ID_MTU_TEST_RESPONSE);
                    response->doNotDiscard().doNotReorder();
                    this->g_transmissionQueue.emplace(std::move(response), packet->getIdentity());
                    continue;
                }
                case NET_INTERNAL_ID_MTU_ASK:
                {
                    auto response = TransmissionPacket::create(NET_INTERNAL_ID_MTU_ASK_RESPONSE);
                    response->doNotDiscard().doNotReorder().packet()
                            << SocketUdp::retrieveAdapterMTUForDestination(idReceive._ip).value_or(0);
                    this->g_transmissionQueue.emplace(std::move(response), packet->getIdentity());
                    continue;
                }
                }

                //Realm and countId is verified by the flux who have the corresponding client

                if (this->g_fluxes.empty())
                {
                    this->g_defaultFlux.pushPacket(std::move(packet));
                    continue;
                }

                //Try to push packet in a flux
                for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
                {
                    pushingIndex = packet->bumpFluxIndex(this->g_fluxes.size());
                    if (this->g_fluxes[pushingIndex]->pushPacket(std::move(packet)))
                    { //Packet is correctly pushed
                        break;
                    }
                }
                //If every flux is busy, the new packet is dismissed
            }
        }
    }
}
void ServerSideNetUdp::threadTransmission()
{
    std::unique_lock lckServer(this->g_mutexServer);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        //Checking fluxes
        for (std::size_t i = 0; i < this->g_fluxes.size() + 1; ++i)
        {
            ClientList* clients{nullptr};
            if (i == this->g_fluxes.size())
            { //Doing the default flux
                clients = &this->g_defaultFlux._clients;
            }
            else
            {
                clients = &this->g_fluxes[i]->_clients;
            }

            auto clientLock = clients->acquireLock();

            for (auto itClient = clients->begin(clientLock); itClient != clients->end(clientLock); ++itClient)
            {
                if (itClient->second._client->isPendingPacketsEmpty())
                {
                    continue;
                }

                if (itClient->second._client->getLastPacketElapsedTime() <
                    itClient->second._client->getSTOCLatency_ms())
                {
                    continue;
                }

                auto transmissionPacket = itClient->second._client->popPacket();

                //MTU check
                auto const headerId = transmissionPacket->packet().retrieveHeaderId().value();
                if (headerId != NET_INTERNAL_FRAGMENTED_PACKET)
                {
                    //Packet is not fragmented, we have to check is size
                    /*if (this->g_mtu == 0) TODO
                    { //We don't know the MTU yet
                        goto mtu_check_skip;
                    }*/

                    auto fragments = transmissionPacket->fragment(64); //TODO
                    for (std::size_t iFragment = 0; iFragment < fragments.size(); ++iFragment)
                    {
                        if (iFragment == 0)
                        {
                            transmissionPacket = std::move(fragments[iFragment]);
                        }
                        else
                        {
                            itClient->second._client->pushForcedFrontPacket(std::move(fragments[iFragment]));
                        }
                    }
                }
                //mtu_check_skip:

                if (!transmissionPacket->packet() || !transmissionPacket->packet().haveCorrectHeaderSize())
                { //Last verification of the packet
                    continue;
                }

                //Applying options
                transmissionPacket->applyOptions(*itClient->second._client);

                //Sending the packet
                //TPacket packet(transmissionPacket->packet()); TODO
                this->g_socket.sendTo(transmissionPacket->packet(), itClient->first._ip, itClient->first._port);
                itClient->second._client->resetLastPacketTimePoint();
            }
        }

        //Checking isolated transmission queue
        while (!this->g_transmissionQueue.empty())
        {
            auto data = std::move(this->g_transmissionQueue.front());
            this->g_transmissionQueue.pop();

            if (!data.first->packet() || !data.first->packet().haveCorrectHeaderSize())
            { //Last verification of the packet
                continue;
            }

            //Sending the packet
            //TPacket packet(data.first->packet()); TODO
            this->g_socket.sendTo(data.first->packet(), data.second._ip, data.second._port);
        }
    }
}

//ServerClientSideUdp
ClientSideNetUdp::ClientSideNetUdp(IpAddress::Types addressType) :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_socket(addressType),
        g_running(false),
        g_mtu(0)
{}

ClientSideNetUdp::~ClientSideNetUdp()
{
    this->stop();
}

void ClientSideNetUdp::stop()
{
    if (this->g_running)
    {
        this->g_running = false;

        this->g_threadReception->join();
        this->g_threadTransmission->join();

        this->g_threadReception = nullptr;
        this->g_threadTransmission = nullptr;

        this->g_socket.close();

        //Clear the flux
        this->clearPackets();
        //Clear client
        this->_client.clearPackets();
        this->_client.clearLostPacketCount();
        this->_client.setClientPacketCounter(0);
        this->_client.setCurrentRealm(FGE_NET_DEFAULT_REALM);
        this->_client.setCurrentPacketCounter(0);
    }
}

void ClientSideNetUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

IpAddress::Types ClientSideNetUdp::getAddressType() const
{
    return this->g_socket.getAddressType();
}

bool ClientSideNetUdp::isRunning() const
{
    return this->g_running;
}

std::future<uint16_t> ClientSideNetUdp::retrieveMTU()
{
    if (!this->g_running)
    {
        throw Exception("Cannot retrieve MTU without a running client");
    }

    auto command = std::make_unique<NetMTUCommand>();
    auto future = command->get_future();

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_commands.push(std::move(command));

    return future;
}

Identity const& ClientSideNetUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}

FluxProcessResults ClientSideNetUdp::process(ProtocolPacketPtr& packet)
{
    ///TODO: no lock ?
    packet.reset();

    if (this->_client.getStatus().getNetworkStatus() != ClientStatus::NetworkStatus::TIMEOUT &&
        this->_client.getStatus().isTimeout())
    {
        this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::TIMEOUT);
        this->_onClientTimeout.call(*this);
        this->_g_remainingPackets = 0;
        this->clearPackets();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    if (this->_g_remainingPackets == 0)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //Popping the next packet
    packet = this->popNextPacket();
    if (!packet)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }
    --this->_g_remainingPackets;

    auto const headerFlags = packet->retrieveFlags().value();

    if ((headerFlags & FGE_NET_HEADER_DO_NOT_REORDER_FLAG) > 0)
    {
        return FluxProcessResults::RETRIEVABLE;
    }

    if ((headerFlags & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) == 0)
    {
        auto reorderResult =
                this->processReorder(this->_client, packet, this->_client.getCurrentPacketCounter(), false);
        if (reorderResult != FluxProcessResults::RETRIEVABLE)
        {
            return reorderResult;
        }
    }

    auto const stat = PacketReorderer::checkStat(packet, this->_client.getCurrentPacketCounter(),
                                                 this->_client.getCurrentRealm());

    if ((headerFlags & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) == 0)
    {
        if (stat == PacketReorderer::Stats::OLD_REALM || stat == PacketReorderer::Stats::OLD_COUNTER)
        {
            this->_client.advanceLostPacketCount();
            --this->_g_remainingPackets;
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
    }

    if (stat == PacketReorderer::Stats::WAITING_NEXT_REALM || stat == PacketReorderer::Stats::WAITING_NEXT_COUNTER)
    {
        this->_client.advanceLostPacketCount(); //We are missing a packet
    }

    auto const serverRealm = packet->retrieveRealm().value();
    auto const serverCountId = packet->retrieveCounter().value();

    this->_client.setCurrentPacketCounter(serverCountId);
    this->_client.setCurrentRealm(serverRealm);

    return FluxProcessResults::RETRIEVABLE;
}

std::size_t ClientSideNetUdp::waitForPackets(std::chrono::milliseconds time_ms)
{
    std::unique_lock lock(this->_g_mutexFlux);
    auto packetSize = this->_g_packets.size();
    if (packetSize > 0)
    {
        return packetSize;
    }

    this->g_receptionNotifier.wait_for(lock, time_ms);
    return this->_g_packets.size();
}

void ClientSideNetUdp::threadReception()
{
    Packet pckReceive; //TODO
    auto lastTimePoint = std::chrono::steady_clock::now();

    while (this->g_running)
    {
        if (this->g_socket.select(true, FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS) == Socket::Errors::ERR_NOERROR)
        {
            if (this->g_socket.receive(pckReceive) != Socket::Errors::ERR_NOERROR)
            {
                continue;
            }

#ifdef FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST
            if (fge::_random.range(0, 1000) <= 10)
            {
                continue;
            }
#endif

            if (pckReceive.getDataSize() < ProtocolPacket::HeaderSize)
            { //Bad header, packet is dismissed
                continue;
            }

            //Skip the header
            pckReceive.skip(ProtocolPacket::HeaderSize);
            auto packet = std::make_unique<ProtocolPacket>(std::move(pckReceive), this->g_clientIdentity);
            packet->setTimestamp(Client::getTimestamp_ms());

            //Verify headerId
            auto const headerId = packet->retrieveFullHeaderId().value();
            if ((headerId & ~FGE_NET_HEADER_FLAGS_MASK) == FGE_NET_BAD_ID ||
                (headerId & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) > 0)
            { //Bad headerId, packet is dismissed
                continue;
            }

            auto const now = std::chrono::steady_clock::now();
            auto const deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimePoint);
            lastTimePoint = now;

            //Check client status and reset timeout
            if (this->_client.getStatus().getNetworkStatus() != ClientStatus::NetworkStatus::TIMEOUT)
            {
                this->_client.getStatus().resetTimeout();
            }

            //Check if the packet is a fragment
            if ((headerId & ~FGE_NET_HEADER_FLAGS_MASK) == NET_INTERNAL_FRAGMENTED_PACKET)
            {
                auto const result = this->g_defragmentation.process(std::move(packet));
                if (result._result == PacketDefragmentation::Results::RETRIEVABLE)
                {
                    packet = this->g_defragmentation.retrieve(result._id, this->g_clientIdentity);
                }
                else
                {
                    continue;
                }
            }

            //Checking commands
            {
                std::scoped_lock const commandLock(this->g_mutexCommands);
                if (!this->g_commands.empty())
                {

                    auto const result = this->g_commands.front()->receive(packet, this->g_socket, deltaTime);
                    if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                    {
                        this->g_commands.pop();
                    }

                    //Commands can drop the packet
                    if (!packet)
                    {
                        continue;
                    }
                }
            }

            this->pushPacket(std::move(packet));
            this->g_receptionNotifier.notify_all();
        }
        else
        {
            //Checking commands
            {
                std::scoped_lock const commandLock(this->g_mutexCommands);
                if (!this->g_commands.empty())
                {
                    std::unique_ptr<ProtocolPacket> dummyPacket;
                    auto const result = this->g_commands.front()->receive(
                            dummyPacket, this->g_socket,
                            std::chrono::milliseconds{FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS});
                    if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                    {
                        this->g_commands.pop();
                    }
                }
            }

            if (this->_client.getStatus().getNetworkStatus() != ClientStatus::NetworkStatus::TIMEOUT)
            {
                (void) this->_client.getStatus().updateTimeout(
                        std::chrono::milliseconds{FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS});
            }
        }
    }
}
void ClientSideNetUdp::threadTransmission()
{
    std::unique_lock lckServer(this->_g_mutexFlux);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        //Checking commands
        {
            std::scoped_lock const commandLock(this->g_mutexCommands);
            if (!this->g_commands.empty())
            {
                TransmissionPacketPtr possiblePacket;
                auto const result = this->g_commands.front()->transmit(possiblePacket, this->g_socket, this->_client);
                if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                {
                    this->g_commands.pop();
                }

                if (possiblePacket)
                {
                    //Applying options
                    possiblePacket->applyOptions(this->_client);

                    //Sending the packet
                    this->_client.pushPacket(std::move(possiblePacket));
                    //TPacket packet = possiblePacket->packet();
                    //this->g_socket.send(packet); TODO
                    //continue;
                }
            }
        }

        //Flux
        if (this->_client.isPendingPacketsEmpty())
        {
            continue;
        }

        if (this->_client.getLastPacketElapsedTime() >= this->_client.getCTOSLatency_ms())
        { //Ready to send !
            auto transmissionPacket = this->_client.popPacket();

            //MTU check
            auto const headerId = transmissionPacket->packet().retrieveHeaderId().value();
            if (headerId != NET_INTERNAL_FRAGMENTED_PACKET)
            {
                //Packet is not fragmented, we have to check is size
                if (this->g_mtu == 0)
                { //We don't know the MTU yet
                    goto mtu_check_skip;
                }

                auto fragments = transmissionPacket->fragment(this->g_mtu);
                for (std::size_t i = 0; i < fragments.size(); ++i)
                {
                    if (i == 0)
                    {
                        transmissionPacket = std::move(fragments[i]);
                    }
                    else
                    {
                        this->_client.pushForcedFrontPacket(std::move(fragments[i]));
                    }
                }
            }
        mtu_check_skip:

            if (!transmissionPacket->packet() || !transmissionPacket->packet().haveCorrectHeaderSize())
            { //Last verification of the packet
                continue;
            }

            //Applying options
            transmissionPacket->applyOptions(this->_client);

            //Sending the packet
            //TPacket packet = transmissionPacket->packet(); TODO
            this->g_socket.send(transmissionPacket->packet());
            this->_client.resetLastPacketTimePoint();
        }
    }
}

} // namespace fge::net
