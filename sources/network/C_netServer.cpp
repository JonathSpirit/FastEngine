/*
 * Copyright 2025 Guillaume Guillet
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

#include "FastEngine/C_compressorLZ4.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/network/C_server.hpp"
#include "private/fge_crypt.hpp"
#include "private/fge_debug.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>

using namespace fge::priv;

namespace fge::net
{

//ServerSideNetUdp
ServerSideNetUdp::ServerSideNetUdp(IpAddress::Types type) :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_defaultFlux(*this, true),
        g_socket(type),
        g_running(false)
{}
ServerSideNetUdp::~ServerSideNetUdp()
{
    this->stop();
}

void ServerSideNetUdp::setVersioningString(std::string_view versioningString)
{
    std::scoped_lock const lock{this->g_mutexServer};
    this->g_versioningString = versioningString;
}
std::string const& ServerSideNetUdp::getVersioningString() const
{
    return this->g_versioningString;
}

bool ServerSideNetUdp::start(Port bindPort, IpAddress const& bindIp, IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }

    this->g_socket.setAddressType(addressType);
    if (this->g_socket.bind(bindPort, bindIp) == Socket::Errors::ERR_NOERROR)
    {
        if (!CryptServerInit(this->g_crypt_ctx))
        {
            this->g_socket.close();
            return false;
        }

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerSideNetUdp::threadReception, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission, this);

        return true;
    }
    return false;
}
bool ServerSideNetUdp::start(IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }
    this->g_socket.setAddressType(addressType);
    if (this->g_socket.isValid())
    {
        if (!CryptServerInit(this->g_crypt_ctx))
        {
            this->g_socket.close();
            return false;
        }

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerSideNetUdp::threadReception, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission, this);

        return true;
    }
    return false;
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

        CryptUninit(this->g_crypt_ctx);
    }
}

ServerNetFluxUdp* ServerSideNetUdp::newFlux()
{
    std::scoped_lock const lock(this->g_mutexServer);

    this->g_fluxes.push_back(std::make_unique<ServerNetFluxUdp>(*this, false));
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

void ServerSideNetUdp::repushPacket(ReceivedPacketPtr&& packet)
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

[[nodiscard]] bool ServerSideNetUdp::announceNewClient(Identity const& identity, ClientSharedPtr const& client)
{
    std::scoped_lock const lock(this->g_mutexServer);

    auto result = this->g_clientsMap.emplace(identity, client);
    if (!result.second)
    {
        if (!result.first->second.expired())
        {
            return result.first->second.lock() == client;
        }
        result.first->second = client;
    }
    return true;
}

void ServerSideNetUdp::sendTo(TransmitPacketPtr& pck, Client const& client, Identity const& id)
{
    pck->applyOptions(client);
    pck->doNotReorder();

    {
        std::scoped_lock const lock(this->g_mutexServer);
        this->g_transmissionQueue.emplace(std::move(pck), id);
    }
    this->g_transmissionNotifier.notify_one();
}
void ServerSideNetUdp::sendTo(TransmitPacketPtr& pck, Identity const& id)
{
    pck->applyOptions();
    pck->doNotReorder();

    {
        std::scoped_lock const lock(this->g_mutexServer);
        this->g_transmissionQueue.emplace(std::move(pck), id);
    }
    this->g_transmissionNotifier.notify_one();
}

void* ServerSideNetUdp::getCryptContext() const
{
    return this->g_crypt_ctx;
}

void ServerSideNetUdp::threadReception()
{
    Identity idReceive;
    Packet pckReceive;
    std::size_t pushingIndex = 0;
    auto gcClientsMap = std::chrono::steady_clock::now();

    CompressorLZ4 compressor;

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

                auto packet = std::make_unique<ProtocolPacket>(std::move(pckReceive), idReceive);
                packet->setTimestamp(Client::getTimestamp_ms());

                std::scoped_lock const lck(this->g_mutexServer);

                auto itClient = this->g_clientsMap.find(idReceive);
                if (itClient != this->g_clientsMap.end())
                {
                    auto client = itClient->second.lock();
                    if (!client)
                    { //bad client
                        this->g_clientsMap.erase(itClient);
                    }
                    else
                    {
                        //Check if the packet is encrypted
                        if (client->getStatus().isInEncryptedState())
                        {
                            if (!CryptDecrypt(*client, *packet))
                            {
                                continue;
                            }
                        }
                    }
                }

                //Here we consider that the packet is not encrypted
                if (!packet->haveCorrectHeader())
                {
                    continue;
                }
                //Skip the header for reading
                packet->skip(ProtocolPacket::HeaderSize);

                //Decompress the packet if needed
                if (!packet->decompress(compressor))
                {
                    continue;
                }

                //Realm and countId is verified by the flux

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

        //"Garbage collection" of the clients map
        auto const now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - gcClientsMap) >=
            std::chrono::milliseconds{FGE_SERVER_CLIENTS_MAP_GC_DELAY_MS})
        {
            gcClientsMap = now;

            for (auto it = this->g_clientsMap.begin(); it != this->g_clientsMap.end();)
            {
                if (it->second.expired())
                {
                    it = this->g_clientsMap.erase(it);
                    continue;
                }
                ++it;
            }
        }
    }
}
void ServerSideNetUdp::threadTransmission()
{
    std::unique_lock lckServer(this->g_mutexServer);
    CompressorLZ4 compressor;
    std::chrono::steady_clock::time_point timePoint;

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

            timePoint = std::chrono::steady_clock::now();
            auto clientLock = clients->acquireLock();

            for (auto itClient = clients->begin(clientLock); itClient != clients->end(clientLock); ++itClient)
            {
                auto& client = itClient->second;

                //check cache
                client->_context._cache.process(timePoint, *client);

                if (client->isPendingPacketsEmpty())
                {
                    continue;
                }

                if (client->getLastPacketLatency() < client->getSTOCLatency_ms())
                {
                    continue;
                }

                auto transmissionPacket = client->popPacket();

                if (!transmissionPacket->isMarkedAsCached())
                {
                    //Compression and applying options
                    transmissionPacket->applyOptions(*client);
                    if (!transmissionPacket->isFragmented())
                    {
                        if (client->getStatus().isInEncryptedState())
                        {
                            if (!transmissionPacket->compress(compressor))
                            {
                                continue;
                            }
                        }
                        client->_context._cache.push(transmissionPacket);
                    }
                }

                //MTU check
                if (!transmissionPacket->isFragmented() &&
                    !transmissionPacket->checkFlags(FGE_NET_HEADER_DO_NOT_FRAGMENT_FLAG))
                {
                    auto const mtu = client->getMTU();

                    //Packet is not fragmented, we have to check is size
                    if (mtu == 0)
                    { //We don't know the MTU yet
                        goto mtu_check_skip;
                    }

                    auto fragments = transmissionPacket->fragment(mtu);
#ifdef FGE_ENABLE_PACKET_DEBUG_VERBOSE
                    auto const fragmentCount = fragments.size();
                    if (fragmentCount > 1)
                    {
                        auto const originalSize = transmissionPacket->getDataSize();
                        FGE_DEBUG_PRINT("Fragmenting packet of size {} into {} fragments", originalSize, fragmentCount);
                    }
#endif
                    for (std::size_t iFragment = 0; iFragment < fragments.size(); ++iFragment)
                    {
                        if (iFragment == 0)
                        {
                            transmissionPacket = std::move(fragments[iFragment]);
                        }
                        else
                        {
                            client->pushForcedFrontPacket(std::move(fragments[iFragment]));
                        }
                    }
                }
            mtu_check_skip:

                if (!transmissionPacket->packet() || !transmissionPacket->haveCorrectHeaderSize())
                { //Last verification of the packet
                    continue;
                }

                //Check if the packet must be encrypted
                if (transmissionPacket->isMarkedForEncryption())
                {
                    if (!CryptEncrypt(*client, *transmissionPacket))
                    {
                        continue;
                    }
                }

                //Sending the packet
                this->g_socket.sendTo(transmissionPacket->packet(), itClient->first._ip, itClient->first._port);
                client->resetLastPacketTimePoint();
            }
        }

        //Checking isolated transmission queue TODO: maybe remove all that
        while (!this->g_transmissionQueue.empty())
        {
            auto data = std::move(this->g_transmissionQueue.front());
            this->g_transmissionQueue.pop();

            if (!data.first->packet() || !data.first->haveCorrectHeaderSize())
            { //Last verification of the packet
                continue;
            }

            //Sending the packet
            this->g_socket.sendTo(data.first->packet(), data.second._ip, data.second._port);
        }
    }
}

} // namespace fge::net
