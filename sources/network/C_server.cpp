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

#include "FastEngine/network/C_server.hpp"

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
    std::queue<FluxPacketPtr>().swap(this->_g_packets);
}
bool NetFluxUdp::pushPacket(FluxPacketPtr&& fluxPck)
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    if (this->_g_packets.size() >= this->g_maxPackets)
    {
        return false;
    }
    this->_g_packets.push(std::move(fluxPck));
    return true;
}
void NetFluxUdp::forcePushPacket(FluxPacketPtr fluxPck)
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    this->_g_packets.push(std::move(fluxPck));
}

FluxPacketPtr NetFluxUdp::popNextPacket()
{
    std::scoped_lock<std::mutex> const lock(this->_g_mutexFlux);
    if (!this->_g_packets.empty())
    {
        FluxPacketPtr tmpPck = std::move(this->_g_packets.front());
        this->_g_packets.pop();
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

//ServerUdp
ServerSideNetUdp::ServerSideNetUdp() :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
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

    this->g_fluxes.push_back(std::make_unique<fge::net::ServerNetFluxUdp>());
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
fge::net::Socket::Error ServerSideNetUdp::sendTo(fge::net::TransmissionPacketPtr& pck,
                                                 fge::net::Client const& client,
                                                 fge::net::Identity const& id)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    pck->applyOptions(client);
    return this->g_socket.sendTo(pck->packet(), id._ip, id._port);
}
fge::net::Socket::Error ServerSideNetUdp::sendTo(fge::net::TransmissionPacketPtr& pck, fge::net::Identity const& id)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    pck->applyOptions();
    return this->g_socket.sendTo(pck->packet(), id._ip, id._port);
}

bool ServerSideNetUdp::isRunning() const
{
    return this->g_running;
}

void ServerSideNetUdp::threadTransmission()
{
    std::unique_lock<std::mutex> lckServer(this->g_mutexServer);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        //Flux
        for (std::size_t i = 0; i < this->g_fluxes.size() + 1; ++i)
        {
            fge::net::ClientList* clients{nullptr};
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
                if (itClient->second->isPendingPacketsEmpty())
                {
                    continue;
                }

                if (itClient->second->getLastPacketElapsedTime() < itClient->second->getSTOCLatency_ms())
                {
                    continue;
                }

                auto transmissionPacket = itClient->second->popPacket();

                if (!transmissionPacket->packet())
                { //Last verification of the packet
                    continue;
                }

                //Applying options
                transmissionPacket->applyOptions(*itClient->second);

                //Sending the packet
                this->sendTo(transmissionPacket->packet(), itClient->first);
                itClient->second->resetLastPacketTimePoint();
            }
        }
    }
}

//ServerClientSideUdp
ClientSideNetUdp::ClientSideNetUdp() :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
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

bool ClientSideNetUdp::isRunning() const
{
    return this->g_running;
}

fge::net::Identity const& ClientSideNetUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
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

void ClientSideNetUdp::threadTransmission()
{
    std::unique_lock<std::mutex> lckServer(this->_g_mutexFlux);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        //Flux
        if (!this->_client.isPendingPacketsEmpty())
        {
            if (this->_client.getLastPacketElapsedTime() >= this->_client.getCTOSLatency_ms())
            { //Ready to send !
                auto transmissionPacket = this->_client.popPacket();

                if (!transmissionPacket->packet())
                { //Last verification of the packet
                    continue;
                }

                //Applying options
                transmissionPacket->applyOptions(this->_client);

                //Sending the packet
                this->send(transmissionPacket->packet());
                this->_client.resetLastPacketTimePoint();
            }
        }
    }
}

} // namespace fge::net
