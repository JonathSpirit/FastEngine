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
#include <iostream>

namespace fge::net
{

//ServerFluxUdp
NetFluxUdp::~NetFluxUdp()
{
    this->clearPackets();
}

void NetFluxUdp::clearPackets()
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    this->_g_packets.clear();
}
bool NetFluxUdp::pushPacket(FluxPacketPtr&& fluxPck)
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    if (this->_g_packets.size() >= this->g_maxPackets)
    {
        return false;
    }
    this->_g_packets.push_back(std::move(fluxPck));
    return true;
}
void NetFluxUdp::forcePushPacket(FluxPacketPtr fluxPck)
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    this->_g_packets.push_back(std::move(fluxPck));
}
void NetFluxUdp::forcePushPacketFront(FluxPacketPtr fluxPck)
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    this->_g_packets.push_front(std::move(fluxPck));
}

FluxPacketPtr NetFluxUdp::popNextPacket()
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
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
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    return this->_g_packets.size();
}
bool NetFluxUdp::isEmpty() const
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    return this->_g_packets.empty();
}

void NetFluxUdp::setMaxPackets(std::size_t n)
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    this->g_maxPackets = n;
}
std::size_t NetFluxUdp::getMaxPackets() const
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    return this->g_maxPackets;
}

//ServerNetFluxUdp
FluxProcessResults
ServerNetFluxUdp::process(ClientSharedPtr& refClient, FluxPacketPtr& refFluxPacket, bool allowUnknownClient)
{
    refClient.reset();
    refFluxPacket.reset();

    if (this->g_clientWithRetrievableOrderedPacket)
    { //All packets cached from a client must be retrieved first
        auto const clientCountId = this->g_clientWithRetrievableOrderedPacket->getClientPacketCountId();
        if (!this->g_clientWithRetrievableOrderedPacket->getPacketReorderer().isRetrievable(
                    clientCountId, this->g_clientWithRetrievableOrderedPacket->getCurrentRealm()))
        {
            this->g_clientWithRetrievableOrderedPacket.reset();
            std::cout << "finish reordering" << std::endl;
            return FluxProcessResults::BAD_REORDER;
        }

        refClient = this->g_clientWithRetrievableOrderedPacket;
        refFluxPacket = this->g_clientWithRetrievableOrderedPacket->getPacketReorderer().pop();

        //Verify the realm
        if (!this->verifyRealm(refClient, refFluxPacket))
        {
            std::cout << "bad realm during reorder" << std::endl;
            return FluxProcessResults::BAD_REALM;
        }
        std::cout << "getting packet from reorder" << std::endl;
        auto const countId = refFluxPacket->retrieveCountId().value();
        refClient->setClientPacketCountId(countId);
        return FluxProcessResults::RETRIEVABLE;
    }

    if (this->g_remainingPackets == 0)
    {
        this->g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //Popping the next packet
    refFluxPacket = this->popNextPacket();
    --this->g_remainingPackets;

    //Verify if the client is known
    refClient = this->_clients.get(refFluxPacket->getIdentity());

    if (!refClient && !allowUnknownClient)
    {
        this->g_server->repushPacket(std::move(refFluxPacket));
        std::cout << "unknown client, repushing" << std::endl;
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    if (refClient)
    {
        auto const headerId = refFluxPacket->retrieveHeaderId().value();

        if ((headerId & FGE_NET_HEADERID_DO_NOT_REORDER_FLAG) > 0)
        {
            std::cout << "packet with do not reorder flag" << std::endl;
            return FluxProcessResults::RETRIEVABLE;
        }

        //Reorder the packet if needed
        refClient->getPacketReorderer().push(std::move(refFluxPacket));
        if (!refClient->getPacketReorderer().isRetrievable(refClient->getClientPacketCountId(),
                                                           refClient->getCurrentRealm()))
        {
            std::cout << "packet need reorder" << std::endl;
            return FluxProcessResults::NOT_RETRIEVABLE;
        }

        refFluxPacket = refClient->getPacketReorderer().pop();

        if (!refClient->getPacketReorderer().isEmpty())
        {
            std::cout << "getting into packet ordered retrieve" << std::endl;
            this->g_clientWithRetrievableOrderedPacket = refClient;
        }

        //Verify the realm
        if (!this->verifyRealm(refClient, refFluxPacket))
        {
            std::cout << "bad realm" << std::endl;
            return FluxProcessResults::BAD_REALM;
        }

        auto const countId = refFluxPacket->retrieveCountId().value();
        refClient->setClientPacketCountId(countId);
    }

    return FluxProcessResults::RETRIEVABLE;
}

bool ServerNetFluxUdp::verifyRealm(ClientSharedPtr const& refClient, FluxPacketPtr const& refFluxPacket)
{
    auto const headerId = refFluxPacket->retrieveHeaderId().value();

    if ((headerId & FGE_NET_HEADERID_DO_NOT_DISCARD_FLAG) > 0)
    {
        return true;
    }

    auto const clientProvidedRealm = refFluxPacket->retrieveRealm().value();

    if (clientProvidedRealm != refClient->getCurrentRealm() &&
        refClient->getLastRealmChangeElapsedTime() >= FGE_SERVER_MAX_TIME_DIFFERENCE_REALM)
    {
        this->_onClientBadRealm.call(refClient);
        return false;
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
    }
}

fge::net::ServerNetFluxUdp* ServerSideNetUdp::newFlux()
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);

    this->g_fluxes.push_back(std::make_unique<fge::net::ServerNetFluxUdp>(*this));
    return this->g_fluxes.back().get();
}
fge::net::ServerNetFluxUdp* ServerSideNetUdp::getFlux(std::size_t index)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);

    if (index >= this->g_fluxes.size())
    {
        return nullptr;
    }
    return this->g_fluxes[index].get();
}
fge::net::ServerNetFluxUdp* ServerSideNetUdp::getDefaultFlux()
{
    return &this->g_defaultFlux;
}
std::size_t ServerSideNetUdp::getFluxSize() const
{
    return this->g_fluxes.size();
}
fge::net::IpAddress::Types ServerSideNetUdp::getAddressType() const
{
    return this->g_socket.getAddressType();
}
void ServerSideNetUdp::closeFlux(fge::net::NetFluxUdp* flux)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);

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
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);
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

fge::net::Socket::Error ServerSideNetUdp::sendTo(fge::net::Packet& pck, fge::net::Identity const& id)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    return this->g_socket.sendTo(pck, id._ip, id._port);
}

bool ServerSideNetUdp::isRunning() const
{
    return this->g_running;
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
    }
}

void ClientSideNetUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

fge::net::Socket::Error ClientSideNetUdp::send(fge::net::Packet& pck)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    return this->g_socket.send(pck);
}
fge::net::Socket::Error ClientSideNetUdp::send(fge::net::TransmissionPacketPtr& pck)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    pck->applyOptions(this->_client);
    return this->g_socket.send(pck->packet());
}
fge::net::IpAddress::Types ClientSideNetUdp::getAddressType() const
{
    return this->g_socket.getAddressType();
}

bool ClientSideNetUdp::isRunning() const
{
    return this->g_running;
}

fge::net::Identity const& ClientSideNetUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}

FluxProcessResults ClientSideNetUdp::process(FluxPacketPtr& refFluxPacket)
{
    refFluxPacket.reset();

    if (this->g_remainingPackets == 0)
    {
        this->g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //Popping the next packet
    refFluxPacket = this->popNextPacket();
    --this->g_remainingPackets;

    auto const headerId = refFluxPacket->retrieveHeaderId().value();

    if ((headerId & FGE_NET_HEADERID_DO_NOT_REORDER_FLAG) > 0)
    {
        std::cout << "packet with do not reorder flag" << std::endl;
        return FluxProcessResults::RETRIEVABLE;
    }

    if ((headerId & FGE_NET_HEADERID_LOCAL_REORDERED_FLAG) == 0)
    {
        //Reorder the packet if needed
        if (this->_client.getPacketReorderer().isEmpty())
        {
            this->_client.getPacketReorderer().push(std::move(refFluxPacket));

            if (!this->_client.getPacketReorderer().isRetrievable(this->_client.getCurrentPacketCountId(),
                                                                  this->_client.getCurrentRealm()))
            {
                std::cout << "packet need reorder" << std::endl;
                return FluxProcessResults::NOT_RETRIEVABLE;
            }

            //If the packet is directly retrievable, we can continue the process ...
            refFluxPacket = this->_client.getPacketReorderer().pop();
        }
        else
        {
            this->_client.getPacketReorderer().push(std::move(refFluxPacket));

            std::size_t containerInversedSize = 0;
            auto* containerInversed = FGE_ALLOCA_T(FluxPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX);
            FGE_PLACE_CONSTRUCT(FluxPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);

            //We already have packets in cache, we check the retrievability with a loop
            while (this->_client.getPacketReorderer().isRetrievable(this->_client.getCurrentPacketCountId(),
                                                                    this->_client.getCurrentRealm()))
            {
                auto reorderedPacket = this->_client.getPacketReorderer().pop();

                //Add the LOCAL_REORDERED_FLAG to the headerId
                reorderedPacket->setHeaderId(reorderedPacket->retrieveHeaderId().value() |
                                             FGE_NET_HEADERID_LOCAL_REORDERED_FLAG);

                //Add it to the container (we are going to push in front of the flux queue, so we need to inverse the order)
                containerInversed[containerInversedSize++] = std::move(reorderedPacket);
            }

            if (containerInversedSize == 0)
            {
                //No packet to retrieve
                std::cout << "packet need reorder" << std::endl;
                FGE_PLACE_DESTRUCT(FluxPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);
                return FluxProcessResults::BAD_REORDER;
            }

            std::cout << "reorder complete with " << containerInversedSize << " packets" << std::endl;

            //Now we push the packets (until the last one) in the correct order in the flux queue
            for (std::size_t i = containerInversedSize; i != 1; --i)
            {
                this->forcePushPacketFront(std::move(containerInversed[i - 1]));
                ++this->g_remainingPackets;
            }

            refFluxPacket = std::move(containerInversed[0]);

            FGE_PLACE_DESTRUCT(FluxPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);
        }
    }
    else
    {
        std::cout << "getting into an reordered packet" << std::endl;
    }

    //Verify the realm
    if (!this->verifyRealm(refFluxPacket))
    {
        std::cout << "bad realm" << std::endl;
        return FluxProcessResults::BAD_REALM;
    }

    auto const serverRealm = refFluxPacket->retrieveRealm().value();
    auto const serverCountId = refFluxPacket->retrieveCountId().value();

    this->_client.setCurrentPacketCountId(serverCountId);
    this->_client.setCurrentRealm(serverRealm);

    return FluxProcessResults::RETRIEVABLE;
}
bool ClientSideNetUdp::verifyRealm(FluxPacketPtr const& refFluxPacket)
{
    auto const headerId = refFluxPacket->retrieveHeaderId().value();

    if ((headerId & FGE_NET_HEADERID_DO_NOT_DISCARD_FLAG) > 0)
    {
        return true;
    }

    auto const serverProvidedRealm = refFluxPacket->retrieveRealm().value();

    if (serverProvidedRealm < this->_client.getCurrentRealm() && this->_client.getCurrentRealm() + 1 != 0)
    {
        return false;
    }
    return true;
}

std::size_t ClientSideNetUdp::waitForPackets(std::chrono::milliseconds time_ms)
{
    std::unique_lock<std::mutex> lock(this->_g_mutexFlux);
    auto packetSize = this->_g_packets.size();
    if (packetSize > 0)
    {
        return packetSize;
    }

    this->g_receptionNotifier.wait_for(lock, time_ms);
    return this->_g_packets.size();
}

} // namespace fge::net
