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

#include "FastEngine/C_server.hpp"

namespace fge::net
{

//ServerFluxUdp
ServerFluxUdp::~ServerFluxUdp()
{
    this->clearPackets();
}

void ServerFluxUdp::clearPackets()
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexLocal);
    std::queue<FluxPacketSharedPtr>().swap(this->g_packets);
}
bool ServerFluxUdp::pushPacket(FluxPacketSharedPtr const& fluxPck)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexLocal);
    if (this->g_packets.size() >= this->g_maxPackets)
    {
        return false;
    }
    this->g_packets.push(fluxPck);
    return true;
}
void ServerFluxUdp::forcePushPacket(FluxPacketSharedPtr fluxPck)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexLocal);
    this->g_packets.push(std::move(fluxPck));
}

FluxPacketSharedPtr ServerFluxUdp::popNextPacket()
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexLocal);
    if (!this->g_packets.empty())
    {
        FluxPacketSharedPtr tmpPck = std::move(this->g_packets.front());
        this->g_packets.pop();
        return tmpPck;
    }
    return nullptr;
}
std::size_t ServerFluxUdp::getPacketsSize() const
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexLocal);
    return this->g_packets.size();
}
bool ServerFluxUdp::isEmpty() const
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexLocal);
    return this->g_packets.empty();
}

void ServerFluxUdp::setMaxPackets(std::size_t n)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexLocal);
    this->g_maxPackets = n;
}
std::size_t ServerFluxUdp::getMaxPackets() const
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexLocal);
    return this->g_maxPackets;
}

//ServerUdp
ServerUdp::ServerUdp() :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_running(false)
{}
ServerUdp::~ServerUdp()
{
    this->stop();
}

void ServerUdp::stop()
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

fge::net::ServerFluxUdp* ServerUdp::newFlux()
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);

    this->g_fluxes.push_back(std::make_unique<fge::net::ServerFluxUdp>());
    return this->g_fluxes.back().get();
}
fge::net::ServerFluxUdp* ServerUdp::getFlux(std::size_t index)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);

    if (index >= this->g_fluxes.size())
    {
        return nullptr;
    }
    return this->g_fluxes[index].get();
}
fge::net::ServerFluxUdp* ServerUdp::getDefaultFlux()
{
    return &this->g_defaultFlux;
}
std::size_t ServerUdp::getFluxSize() const
{
    return this->g_fluxes.size();
}
void ServerUdp::closeFlux(fge::net::ServerFluxUdp* flux)
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
void ServerUdp::closeAllFlux()
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);
    this->g_fluxes.clear();
}

void ServerUdp::repushPacket(FluxPacketSharedPtr&& fluxPck)
{
    if ((++fluxPck->_fluxCount) >= this->g_fluxes.size())
    {
        this->g_defaultFlux.pushPacket(fluxPck);
        return;
    }
    auto newIndex = (fluxPck->_fluxIndex + 1) % this->g_fluxes.size();
    fluxPck->_fluxIndex = newIndex;
    this->g_fluxes[newIndex]->forcePushPacket(std::move(fluxPck));
}

void ServerUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

fge::net::Socket::Error ServerUdp::sendTo(fge::net::Packet& pck, fge::net::IpAddress const& ip, fge::net::Port port)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    return this->g_socket.sendTo(pck, ip, port);
}
fge::net::Socket::Error ServerUdp::sendTo(fge::net::Packet& pck, fge::net::Identity const& id)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    return this->g_socket.sendTo(pck, id._ip, id._port);
}

bool ServerUdp::isRunning() const
{
    return this->g_running;
}

void ServerUdp::serverThreadTransmission()
{
    std::unique_lock<std::mutex> lckServer(this->g_mutexServer, std::defer_lock);

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

                auto buffPck = itClient->second->popPacket();
                if (!buffPck._pck)
                { //Last verification of the packet
                    continue;
                }

                //Applying options
                for (auto const& option: buffPck._options)
                {
                    if (option._option == fge::net::SendQueuePacket::Options::UPDATE_TIMESTAMP)
                    {
                        fge::net::Timestamp updatedTimestamp = fge::net::Client::getTimestamp_ms();
                        buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                    }
                    else if (option._option == fge::net::SendQueuePacket::Options::UPDATE_FULL_TIMESTAMP)
                    {
                        fge::net::FullTimestamp updatedTimestamp = fge::net::Client::getFullTimestamp_ms();
                        buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                    }
                    else if (option._option == fge::net::SendQueuePacket::Options::UPDATE_CORRECTION_LATENCY)
                    {
                        fge::net::Latency_ms correctorLatency =
                                itClient->second->getCorrectorLatency().value_or(FGE_NET_BAD_LATENCY);
                        buffPck._pck->pack(option._argument, &correctorLatency, sizeof(correctorLatency));
                    }
                }

                //Sending the packet
                this->sendTo(*buffPck._pck, itClient->first);
                itClient->second->resetLastPacketTimePoint();
            }
        }
    }
}

//ServerClientSideUdp
ServerClientSideUdp::ServerClientSideUdp() :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_running(false)
{}
ServerClientSideUdp::~ServerClientSideUdp()
{
    this->stop();
}

void ServerClientSideUdp::stop()
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

void ServerClientSideUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

fge::net::Socket::Error ServerClientSideUdp::send(fge::net::Packet& pck)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    return this->g_socket.send(pck);
}

bool ServerClientSideUdp::isRunning() const
{
    return this->g_running;
}

FluxPacketSharedPtr ServerClientSideUdp::popNextPacket()
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);
    if (!this->g_packets.empty())
    {
        FluxPacketSharedPtr tmpPck = this->g_packets.front();
        this->g_packets.pop();
        return tmpPck;
    }
    return nullptr;
}

std::size_t ServerClientSideUdp::getPacketsSize() const
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);
    return this->g_packets.size();
}
bool ServerClientSideUdp::isEmpty() const
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);
    return this->g_packets.empty();
}

void ServerClientSideUdp::setMaxPackets(std::size_t n)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);
    this->g_maxPackets = n;
}
std::size_t ServerClientSideUdp::getMaxPackets() const
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);
    return this->g_maxPackets;
}

fge::net::Identity const& ServerClientSideUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}

std::size_t ServerClientSideUdp::waitForPackets(std::chrono::milliseconds const& ms)
{
    std::unique_lock<std::mutex> lock(this->g_mutexServer);
    auto packetSize = this->g_packets.size();
    if (packetSize > 0)
    {
        return packetSize;
    }

    this->g_receptionNotifier.wait_for(lock, ms);
    return this->g_packets.size();
}

bool ServerClientSideUdp::pushPacket(FluxPacketSharedPtr const& fluxPck)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexServer);
    if (this->g_packets.size() >= this->g_maxPackets)
    {
        return false;
    }
    this->g_packets.push(fluxPck);
    return true;
}

void ServerClientSideUdp::serverThreadTransmission()
{
    std::unique_lock<std::mutex> lckServer(this->g_mutexServer, std::defer_lock);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        //Flux
        if (!this->_client.isPendingPacketsEmpty())
        {
            if (this->_client.getLastPacketElapsedTime() >= this->_client.getCTOSLatency_ms())
            { //Ready to send !
                auto buffPck = this->_client.popPacket();
                if (buffPck._pck)
                { //Last verification of the packet

                    //Applying options
                    for (auto const& option: buffPck._options)
                    {
                        if (option._option == fge::net::SendQueuePacket::Options::UPDATE_TIMESTAMP)
                        {
                            fge::net::Timestamp updatedTimestamp = fge::net::Client::getTimestamp_ms();
                            buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                        }
                        else if (option._option == fge::net::SendQueuePacket::Options::UPDATE_FULL_TIMESTAMP)
                        {
                            fge::net::FullTimestamp updatedTimestamp = fge::net::Client::getFullTimestamp_ms();
                            buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                        }
                        else if (option._option == fge::net::SendQueuePacket::Options::UPDATE_CORRECTION_LATENCY)
                        {
                            fge::net::Latency_ms correctorLatency =
                                    this->_client.getCorrectorLatency().value_or(FGE_NET_BAD_LATENCY);
                            buffPck._pck->pack(option._argument, &correctorLatency, sizeof(correctorLatency));
                        }
                    }

                    //Sending the packet
                    this->send(*buffPck._pck);
                    this->_client.resetLastPacketTimePoint();
                }
            }
        }
    }
}

} // namespace fge::net
