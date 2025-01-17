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

namespace fge::net
{

//ServerSideNetUdp

template<class TPacket>
bool ServerSideNetUdp::start(Port bindPort, IpAddress const& bindIp, IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }
    this->g_socket.setAddressType(addressType);
    if (this->g_socket.bind(bindPort, bindIp) == Socket::Errors::ERR_NOERROR)
    {
        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerSideNetUdp::threadReception<TPacket>, this);
        this->g_threadTransmission =
                std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission<TPacket>, this);

        return true;
    }
    return false;
}
template<class TPacket>
bool ServerSideNetUdp::start(IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }
    this->g_socket.setAddressType(addressType);
    if (this->g_socket.isValid())
    {
        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerSideNetUdp::threadReception<TPacket>, this);
        this->g_threadTransmission =
                std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission<TPacket>, this);

        return true;
    }
    return false;
}

template<class TPacket>
void ServerSideNetUdp::threadReception()
{
    Identity idReceive;
    TPacket pckReceive;
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
template<class TPacket>
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
                if (itClient->second->isPendingPacketsEmpty())
                {
                    continue;
                }

                if (itClient->second->getLastPacketElapsedTime() < itClient->second->getSTOCLatency_ms())
                {
                    continue;
                }

                auto transmissionPacket = itClient->second->popPacket();

                if (!transmissionPacket->packet() || !transmissionPacket->packet().haveCorrectHeaderSize())
                { //Last verification of the packet
                    continue;
                }

                //Applying options
                transmissionPacket->applyOptions(*itClient->second);

                //Sending the packet
                TPacket packet(transmissionPacket->packet());
                this->g_socket.sendTo(packet, itClient->first._ip, itClient->first._port);
                itClient->second->resetLastPacketTimePoint();
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
            TPacket packet(data.first->packet());
            this->g_socket.sendTo(packet, data.second._ip, data.second._port);
        }
    }
}

//ClientSideNetUdp

template<class TPacket>
bool ClientSideNetUdp::start(Port bindPort,
                             IpAddress const& bindIp,
                             Port connectRemotePort,
                             IpAddress const& connectRemoteAddress,
                             IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }
    this->g_socket.setAddressType(addressType);
    if (addressType == IpAddress::Types::Ipv6)
    {
        this->g_socket.setIpv6Only(false);
    }
    else
    {
        this->g_socket.setDontFragment(true);
    }

    if (this->g_socket.bind(bindPort, bindIp) == Socket::Errors::ERR_NOERROR)
    {
        if (this->g_socket.connect(connectRemoteAddress, connectRemotePort) != Socket::Errors::ERR_NOERROR)
        {
            this->g_socket.close();
            return false;
        }

        this->g_clientIdentity._ip = connectRemoteAddress;
        this->g_clientIdentity._port = connectRemotePort;

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ClientSideNetUdp::threadReception<TPacket>, this);
        this->g_threadTransmission =
                std::make_unique<std::thread>(&ClientSideNetUdp::threadTransmission<TPacket>, this);

        return true;
    }
    this->g_socket.close();
    return false;
}

template<class TPacket>
void ClientSideNetUdp::sendTo(TransmissionPacketPtr& pck, Identity const& id)
{ ///TODO: have a transmission queue ?
    if (!pck->packet() || !pck->packet().haveCorrectHeaderSize())
    { //Last verification of the packet
        return;
    }

    pck->doNotReorder();

    std::scoped_lock const lock(this->_g_mutexFlux);
    TPacket packet = pck->packet();
    this->g_socket.sendTo(packet, id._ip, id._port);
}

template<class TPacket>
void ClientSideNetUdp::threadReception()
{
    TPacket pckReceive;
    auto lastTimePoint = std::chrono::steady_clock::now();

    while (this->g_running)
    {
        if (this->g_socket.select(true, FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS) == Socket::Errors::ERR_NOERROR)
        {
            if (this->g_socket.receive(pckReceive) == Socket::Errors::ERR_NOERROR)
            {
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
template<class TPacket>
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
                    TPacket packet = possiblePacket->packet();
                    this->g_socket.send(packet);
                    continue;
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

            if (!transmissionPacket->packet() || !transmissionPacket->packet().haveCorrectHeaderSize())
            { //Last verification of the packet
                continue;
            }

            //Applying options
            transmissionPacket->applyOptions(this->_client);

            //Sending the packet
            TPacket packet = transmissionPacket->packet();
            this->g_socket.send(packet);
            this->_client.resetLastPacketTimePoint();
        }
    }
}

} // namespace fge::net
