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

#include "FastEngine/C_server.hpp"

#include <memory>
#include "FastEngine/C_clientList.hpp"

namespace fge
{
namespace net
{

///ServerFluxUdp
ServerFluxUdp::~ServerFluxUdp()
{
    this->clear();
}

void ServerFluxUdp::clear()
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    std::size_t qsize = this->g_packets.size();
    for (std::size_t i=0; i<qsize; ++i)
    {
        this->g_packets.pop();
    }
}

bool ServerFluxUdp::pushPacket(const FluxPacketSharedPtr& fluxPck)
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    if ( this->g_packets.size() >= this->g_maxPackets )
    {
        return false;
    }
    this->g_packets.push(fluxPck);
    return true;
}
void ServerFluxUdp::forcePushPacket(const FluxPacketSharedPtr& fluxPck)
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    this->g_packets.push(fluxPck);
}

FluxPacketSharedPtr ServerFluxUdp::popNextPacket()
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    if ( !this->g_packets.empty() )
    {
        FluxPacketSharedPtr tmpPck = this->g_packets.front();
        this->g_packets.pop();
        return tmpPck;
    }
    return nullptr;
}
std::size_t ServerFluxUdp::getPacketsSize() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    return this->g_packets.size();
}
bool ServerFluxUdp::isEmpty() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    return this->g_packets.empty();
}

void ServerFluxUdp::setMaxPackets(std::size_t n)
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    this->g_maxPackets = n;
}
std::size_t ServerFluxUdp::getMaxPackets() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    return this->g_maxPackets;
}

///ServerUdp
ServerUdp::ServerUdp() :
    g_threadReception(nullptr),
    g_threadTransmission(nullptr),
    g_running(false)
{
}
ServerUdp::~ServerUdp()
{
    this->stop();
}

void ServerUdp::stop()
{
    if ( this->g_running )
    {
        this->g_running = false;

        this->g_threadReception->join();
        this->g_threadTransmission->join();

        delete this->g_threadReception;
        delete this->g_threadTransmission;

        this->g_threadReception = nullptr;
        this->g_threadTransmission = nullptr;

        this->g_socket.close();
    }
}

fge::net::ServerFluxUdp* ServerUdp::newFlux()
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);

    this->g_flux.push_back( std::make_unique<fge::net::ServerFluxUdp>() );
    return this->g_flux.back().get();
}
fge::net::ServerFluxUdp* ServerUdp::getFlux(std::size_t index)
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);

    if ( index >= this->g_flux.size() )
    {
        return nullptr;
    }
    return this->g_flux[index].get();
}
fge::net::ServerFluxUdp* ServerUdp::getDefaultFlux()
{
    return &this->g_defaultFlux;
}
std::size_t ServerUdp::getFluxSize() const
{
    return this->g_flux.size();
}
void ServerUdp::delFlux(fge::net::ServerFluxUdp* flux)
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);

    for (std::size_t i=0; i<this->g_flux.size(); ++i)
    {
        if (this->g_flux[i].get() == flux)
        {
            this->g_flux.erase(this->g_flux.begin() + i);
            break;
        }
    }
}
void ServerUdp::delAllFlux()
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    this->g_flux.clear();
}

void ServerUdp::repushPacket(const FluxPacketSharedPtr& fluxPck)
{
    if ( (++fluxPck->_fluxCount) >= this->g_flux.size() )
    {
        this->g_defaultFlux.pushPacket(fluxPck);
        return;
    }
    fluxPck->_fluxIndex = (fluxPck->_fluxIndex+1) % this->g_flux.size();
    this->g_flux[fluxPck->_fluxIndex]->forcePushPacket(fluxPck);
}

const fge::net::SocketUdp& ServerUdp::getSocket() const
{
    return this->g_socket;
}
fge::net::SocketUdp& ServerUdp::getSocket()
{
    return this->g_socket;
}

void ServerUdp::notify()
{
    this->g_cv.notify_one();
}
std::mutex& ServerUdp::getSendMutex()
{
    return this->g_mutexSend;
}

fge::net::Socket::Error ServerUdp::sendTo(fge::net::Packet& pck, const fge::net::IpAddress& ip, fge::net::Port port)
{
    std::lock_guard<std::mutex> lock(this->g_mutexSend);
    return this->g_socket.sendTo(pck, ip, port);
}
fge::net::Socket::Error ServerUdp::sendTo(fge::net::Packet& pck, const fge::net::Identity& id)
{
    std::lock_guard<std::mutex> lock(this->g_mutexSend);
    return this->g_socket.sendTo(pck, id._ip, id._port);
}

bool ServerUdp::isRunning() const
{
    return this->g_running;
}

void ServerUdp::serverThreadTransmission()
{
    std::unique_lock<std::mutex> lckServer(this->g_mutexServer);

    while ( this->g_running )
    {
        this->g_cv.wait_for(lckServer, std::chrono::milliseconds(10));

        //Flux
        for (std::size_t i=0; i<this->g_flux.size(); ++i)
        {
            fge::net::ClientList& clients = this->g_flux[i]->_clients;
            std::unique_lock<std::recursive_mutex> lck{clients.acquireLock()};

            for (auto itClient=clients.begin(lck); itClient!=clients.end(lck); ++itClient)
            {
                if ( !itClient->second->isPendingPacketsEmpty() )
                {
                    if ( itClient->second->getLastPacketElapsedTime() >= itClient->second->getSTOCLatency_ms() )
                    {//Ready to send !
                        fge::net::ClientSendQueuePacket buffPck = itClient->second->popPacket();
                        if (buffPck._pck)
                        {//Last verification of the packet

                            //Applying options
                            for (const auto& option : buffPck._options)
                            {
                                if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_TIMESTAMP)
                                {
                                    fge::net::Timestamp updatedTimestamp = fge::net::Client::getTimestamp_ms();
                                    buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                                }
                                else if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_FULL_TIMESTAMP)
                                {
                                    fge::net::FullTimestamp updatedTimestamp = fge::net::Client::getFullTimestamp_ms();
                                    buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                                }
                                else if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_CORRECTION_LATENCY)
                                {
                                    fge::net::Latency_ms correctorLatency = itClient->second->getCorrectorLatency().value_or(FGE_NET_BAD_LATENCY);
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
        }
        //Default flux TODO: redundant code
        std::unique_lock<std::recursive_mutex> lck{this->g_defaultFlux._clients.acquireLock()};

        for (auto itClient=this->g_defaultFlux._clients.begin(lck); itClient!=this->g_defaultFlux._clients.end(lck); ++itClient)
        {
            if ( !itClient->second->isPendingPacketsEmpty() )
            {
                if ( itClient->second->getLastPacketElapsedTime() >= itClient->second->getSTOCLatency_ms() )
                {//Ready to send !
                    fge::net::ClientSendQueuePacket buffPck = itClient->second->popPacket();
                    if (buffPck._pck)
                    {//Last verification of the packet

                        //Applying options
                        for (const auto& option : buffPck._options)
                        {
                            if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_TIMESTAMP)
                            {
                                fge::net::Timestamp updatedTimestamp = fge::net::Client::getTimestamp_ms();
                                buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                            }
                            else if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_FULL_TIMESTAMP)
                            {
                                fge::net::FullTimestamp updatedTimestamp = fge::net::Client::getFullTimestamp_ms();
                                buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                            }
                            else if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_CORRECTION_LATENCY)
                            {
                                fge::net::Latency_ms correctorLatency = itClient->second->getCorrectorLatency().value_or(FGE_NET_BAD_LATENCY);
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
    }
}

///ServerClientSideUdp
ServerClientSideUdp::ServerClientSideUdp() :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_running(false)
{
}
ServerClientSideUdp::~ServerClientSideUdp()
{
    this->stop();
}

void ServerClientSideUdp::stop()
{
    if ( this->g_running )
    {
        this->g_running = false;

        this->g_threadReception->join();
        this->g_threadTransmission->join();

        delete this->g_threadReception;
        delete this->g_threadTransmission;

        this->g_threadReception = nullptr;
        this->g_threadTransmission = nullptr;

        this->g_socket.close();
    }
}

const fge::net::SocketUdp& ServerClientSideUdp::getSocket() const
{
    return this->g_socket;
}
fge::net::SocketUdp& ServerClientSideUdp::getSocket()
{
    return this->g_socket;
}

void ServerClientSideUdp::notify()
{
    this->g_cv.notify_one();
}
std::mutex& ServerClientSideUdp::getSendMutex()
{
    return this->g_mutexSend;
}

fge::net::Socket::Error ServerClientSideUdp::send(fge::net::Packet& pck)
{
    std::lock_guard<std::mutex> lock(this->g_mutexSend);
    return this->g_socket.send(pck);
}

bool ServerClientSideUdp::isRunning() const
{
    return this->g_running;
}

FluxPacketSharedPtr ServerClientSideUdp::popNextPacket()
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    if ( !this->g_packets.empty() )
    {
        FluxPacketSharedPtr tmpPck = this->g_packets.front();
        this->g_packets.pop();
        return tmpPck;
    }
    return nullptr;
}

std::size_t ServerClientSideUdp::getPacketsSize() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    return this->g_packets.size();
}
bool ServerClientSideUdp::isEmpty() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    return this->g_packets.empty();
}

void ServerClientSideUdp::setMaxPackets(std::size_t n)
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    this->g_maxPackets = n;
}
std::size_t ServerClientSideUdp::getMaxPackets() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    return this->g_maxPackets;
}

const fge::net::Identity& ServerClientSideUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}

bool ServerClientSideUdp::waitForPackets(const std::chrono::milliseconds& ms)
{
    if (this->getPacketsSize() > 0)
    {
        return true;
    }
    std::unique_lock<std::mutex> lock(this->g_mutexServer);
    this->g_cvReceiveNotifier.wait_for(lock, ms);
    if ( !this->g_packets.empty() )
    {
        return true;
    }
    return false;
}

bool ServerClientSideUdp::pushPacket(const FluxPacketSharedPtr& fluxPck)
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    if ( this->g_packets.size() >= this->g_maxPackets )
    {
        return false;
    }
    this->g_packets.push(fluxPck);
    return true;
}

void ServerClientSideUdp::serverThreadTransmission()
{
    std::unique_lock<std::mutex> lckServer(this->g_mutexServer);

    while ( this->g_running )
    {
        this->g_cv.wait_for(lckServer, std::chrono::milliseconds(10));

        //Flux
        if ( !this->_client.isPendingPacketsEmpty() )
        {
            if ( this->_client.getLastPacketElapsedTime() >= this->_client.getCTOSLatency_ms() )
            {//Ready to send !
                fge::net::ClientSendQueuePacket buffPck = this->_client.popPacket();
                if (buffPck._pck)
                {//Last verification of the packet

                    //Applying options
                    for (const auto& option : buffPck._options)
                    {
                        if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_TIMESTAMP)
                        {
                            fge::net::Timestamp updatedTimestamp = fge::net::Client::getTimestamp_ms();
                            buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                        }
                        else if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_FULL_TIMESTAMP)
                        {
                            fge::net::FullTimestamp updatedTimestamp = fge::net::Client::getFullTimestamp_ms();
                            buffPck._pck->pack(option._argument, &updatedTimestamp, sizeof(updatedTimestamp));
                        }
                        else if (option._option == fge::net::ClientSendQueuePacket::Options::UPDATE_CORRECTION_LATENCY)
                        {
                            fge::net::Latency_ms correctorLatency = this->_client.getCorrectorLatency().value_or(FGE_NET_BAD_LATENCY);
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

}//end net
}//end fge
