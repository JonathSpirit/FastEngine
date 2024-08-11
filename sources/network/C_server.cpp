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

namespace fge::net
{

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
bool NetFluxUdp::pushPacket(FluxPacketPtr&& fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    if (this->_g_packets.size() >= this->g_maxPackets)
    {
        return false;
    }
    this->_g_packets.push_back(std::move(fluxPck));
    return true;
}
void NetFluxUdp::forcePushPacket(FluxPacketPtr fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.push_back(std::move(fluxPck));
}
void NetFluxUdp::forcePushPacketFront(FluxPacketPtr fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.push_front(std::move(fluxPck));
}

FluxProcessResults NetFluxUdp::processReorder(Client& client,
                                              FluxPacketPtr& refFluxPacket,
                                              ProtocolPacket::CountId currentCountId,
                                              bool ignoreRealm)
{
    auto const currentRealm = ignoreRealm ? refFluxPacket->retrieveRealm().value() : client.getCurrentRealm();

    if (client.getPacketReorderer().isEmpty())
    {
        auto const stat = PacketReorderer::checkStat(refFluxPacket, currentCountId, currentRealm);

        if (stat == PacketReorderer::Stats::RETRIEVABLE)
        {
            return FluxProcessResults::RETRIEVABLE;
        }

        client.getPacketReorderer().push(std::move(refFluxPacket));
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //We push the packet in the reorderer
    client.getPacketReorderer().push(std::move(refFluxPacket));

    //At this point we are sure that the reorderer contains at least 2 packets

    bool forced = client.getPacketReorderer().isForced();
    auto const stat = client.getPacketReorderer().checkStat(currentCountId, currentRealm).value();
    if (!forced && stat != PacketReorderer::Stats::RETRIEVABLE)
    {
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    refFluxPacket = client.getPacketReorderer().pop();
    refFluxPacket->addHeaderFlags(FGE_NET_HEADER_LOCAL_REORDERED_FLAG);

    std::size_t containerInversedSize = 0;
    auto* containerInversed = FGE_ALLOCA_T(FluxPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX);
    FGE_PLACE_CONSTRUCT(FluxPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);

    while (auto const stat = client.getPacketReorderer().checkStat(currentCountId, currentRealm))
    {
        if (!(stat == PacketReorderer::Stats::RETRIEVABLE || forced))
        {
            break;
        }

        auto reorderedPacket = client.getPacketReorderer().pop();

        //Add the LOCAL_REORDERED_FLAG to the header
        reorderedPacket->addHeaderFlags(FGE_NET_HEADER_LOCAL_REORDERED_FLAG);

        //Add it to the container (we are going to push in front of the flux queue, so we need to inverse the order)
        containerInversed[containerInversedSize++] = std::move(reorderedPacket);
    }

    //Now we push the packets (until the last one) in the correct order in the flux queue
    for (std::size_t i = containerInversedSize; i != 0; --i)
    {
        this->forcePushPacketFront(std::move(containerInversed[i - 1]));
        ++this->_g_remainingPackets;
    }

    FGE_PLACE_DESTRUCT(FluxPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);

    return FluxProcessResults::RETRIEVABLE;
}

FluxPacketPtr NetFluxUdp::popNextPacket()
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    if (!this->_g_packets.empty())
    {
        FluxPacketPtr tmpPck = std::move(this->_g_packets.front());
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
ServerNetFluxUdp::process(ClientSharedPtr& refClient, FluxPacketPtr& refFluxPacket, bool allowUnknownClient)
{
    refClient.reset();
    refFluxPacket.reset();

    if (this->_g_remainingPackets == 0)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //Popping the next packet
    refFluxPacket = this->popNextPacket();
    if (!refFluxPacket)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }
    --this->_g_remainingPackets;

    //Verify if the client is known
    refClient = this->_clients.get(refFluxPacket->getIdentity());

    if (!refClient)
    {
        if (allowUnknownClient)
        {
            return FluxProcessResults::RETRIEVABLE;
        }

        this->g_server->repushPacket(std::move(refFluxPacket));
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    auto const header = refFluxPacket->retrieveHeader().value();

    if ((header & FGE_NET_HEADER_DO_NOT_REORDER_FLAG) > 0)
    {
        return FluxProcessResults::RETRIEVABLE;
    }

    if ((header & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) == 0)
    {
        auto reorderResult = this->processReorder(*refClient, refFluxPacket, refClient->getClientPacketCountId(), true);
        if (reorderResult != FluxProcessResults::RETRIEVABLE)
        {
            return reorderResult;
        }
    }

    //Verify the realm
    if (!this->verifyRealm(refClient, refFluxPacket))
    {
        return FluxProcessResults::BAD_REALM;
    }

    auto const stat = PacketReorderer::checkStat(refFluxPacket, refClient->getClientPacketCountId(),
                                                 refClient->getCurrentRealm());

    if ((header & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) == 0)
    {
        if (stat == PacketReorderer::Stats::OLD_COUNTID)
        {
            refClient->advanceLostPacketCount();
            --this->_g_remainingPackets;
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
    }

    if (stat == PacketReorderer::Stats::WAITING_NEXT_REALM || stat == PacketReorderer::Stats::WAITING_NEXT_COUNTID)
    {
        refClient->advanceLostPacketCount(); //We are missing a packet
    }

    auto const countId = refFluxPacket->retrieveCountId().value();
    refClient->setClientPacketCountId(countId);

    return FluxProcessResults::RETRIEVABLE;
}

bool ServerNetFluxUdp::verifyRealm(ClientSharedPtr const& refClient, FluxPacketPtr const& refFluxPacket)
{
    auto const header = refFluxPacket->retrieveHeader().value();
    auto const clientProvidedRealm = refFluxPacket->retrieveRealm().value();

    if (clientProvidedRealm != refClient->getCurrentRealm() &&
        refClient->getLastRealmChangeElapsedTime() >= FGE_SERVER_MAX_TIME_DIFFERENCE_REALM)
    {
        this->_onClientBadRealm.call(refClient);
        return (header & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) > 0;
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

void ServerSideNetUdp::repushPacket(FluxPacketPtr&& fluxPck)
{
    if ((++fluxPck->g_fluxCount) >= this->g_fluxes.size())
    {
        this->g_defaultFlux.pushPacket(std::move(fluxPck));
        return;
    }
    auto newIndex = (fluxPck->g_fluxIndex + 1) % this->g_fluxes.size();
    fluxPck->g_fluxIndex = newIndex;
    this->g_fluxes[newIndex]->forcePushPacket(std::move(fluxPck));
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

//ServerClientSideUdp
ClientSideNetUdp::ClientSideNetUdp(IpAddress::Types addressType) :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_socket(addressType),
        g_running(false)
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
        this->_client.setClientPacketCountId(0);
        this->_client.setCurrentRealm(FGE_NET_DEFAULT_REALM);
        this->_client.setCurrentPacketCountId(0);
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

Identity const& ClientSideNetUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}

FluxProcessResults ClientSideNetUdp::process(FluxPacketPtr& refFluxPacket)
{
    ///TODO: no lock ?
    refFluxPacket.reset();

    if (this->_g_remainingPackets == 0)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //Popping the next packet
    refFluxPacket = this->popNextPacket();
    if (!refFluxPacket)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }
    --this->_g_remainingPackets;

    auto const header = refFluxPacket->retrieveHeader().value();

    if ((header & FGE_NET_HEADER_DO_NOT_REORDER_FLAG) > 0)
    {
        return FluxProcessResults::RETRIEVABLE;
    }

    if ((header & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) == 0)
    {
        auto reorderResult =
                this->processReorder(this->_client, refFluxPacket, this->_client.getCurrentPacketCountId(), false);
        if (reorderResult != FluxProcessResults::RETRIEVABLE)
        {
            return reorderResult;
        }
    }

    auto const stat = PacketReorderer::checkStat(refFluxPacket, this->_client.getCurrentPacketCountId(),
                                                 this->_client.getCurrentRealm());

    if ((header & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) == 0)
    {
        if (stat == PacketReorderer::Stats::OLD_REALM || stat == PacketReorderer::Stats::OLD_COUNTID)
        {
            this->_client.advanceLostPacketCount();
            --this->_g_remainingPackets;
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
    }

    if (stat == PacketReorderer::Stats::WAITING_NEXT_REALM || stat == PacketReorderer::Stats::WAITING_NEXT_COUNTID)
    {
        this->_client.advanceLostPacketCount(); //We are missing a packet
    }

    auto const serverRealm = refFluxPacket->retrieveRealm().value();
    auto const serverCountId = refFluxPacket->retrieveCountId().value();

    this->_client.setCurrentPacketCountId(serverCountId);
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

} // namespace fge::net
