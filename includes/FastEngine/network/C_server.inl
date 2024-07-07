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

//FluxPacket

inline FluxPacket::FluxPacket(fge::net::Packet const& pck,
                              fge::net::Identity const& id,
                              std::size_t fluxIndex,
                              std::size_t fluxCount) :
        ProtocolPacket(pck),
        g_id(id),
        g_timestamp(fge::net::Client::getTimestamp_ms()),
        g_fluxIndex(fluxIndex),
        g_fluxCount(fluxCount)
{}
inline FluxPacket::FluxPacket(fge::net::Packet&& pck,
                              fge::net::Identity const& id,
                              std::size_t fluxIndex,
                              std::size_t fluxCount) :
        ProtocolPacket(std::move(pck)),
        g_id(id),
        g_timestamp(fge::net::Client::getTimestamp_ms()),
        g_fluxIndex(fluxIndex),
        g_fluxCount(fluxCount)
{}

inline fge::net::Timestamp FluxPacket::getTimeStamp() const
{
    return this->g_timestamp;
}
inline fge::net::Identity const& FluxPacket::getIdentity() const
{
    return this->g_id;
}

//ServerSideNetUdp

template<class TPacket>
bool ServerSideNetUdp::start(fge::net::Port bindPort, fge::net::IpAddress const& bindIp, IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }
    this->g_socket.setAddressType(addressType);
    if (this->g_socket.bind(bindPort, bindIp) == fge::net::Socket::ERR_NOERROR)
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
Socket::Error ServerSideNetUdp::sendTo(TransmissionPacketPtr& pck, Client const& client, Identity const& id)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    pck->applyOptions(client);
    pck->packet().addHeaderFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);
    TPacket packet(pck->packet());
    return this->g_socket.sendTo(packet, id._ip, id._port);
}
template<class TPacket>
Socket::Error ServerSideNetUdp::sendTo(TransmissionPacketPtr& pck, Identity const& id)
{
    std::scoped_lock<std::mutex> const lock(this->g_mutexTransmission);
    pck->applyOptions();
    pck->packet().addHeaderFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);
    TPacket packet(pck->packet());
    return this->g_socket.sendTo(packet, id._ip, id._port);
}

template<class TPacket>
void ServerSideNetUdp::threadReception()
{
    fge::net::Identity idReceive;
    TPacket pckReceive;
    std::size_t pushingIndex = 0;

    while (this->g_running)
    {
        if (this->g_socket.select(true, 500) == fge::net::Socket::ERR_NOERROR)
        {
            if (this->g_socket.receiveFrom(pckReceive, idReceive._ip, idReceive._port) == fge::net::Socket::ERR_NOERROR)
            {
#ifdef FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST
                if (fge::_random.range(0, 1000) <= 10)
                {
                    continue;
                }
#endif

                std::scoped_lock<std::mutex> const lck(this->g_mutexServer);

                if (pckReceive.getDataSize() < ProtocolPacket::HeaderSize)
                { //Bad header, packet is dismissed
                    continue;
                }

                //Skip the header for reading
                pckReceive.skip(ProtocolPacket::HeaderSize);
                auto fluxPacket = std::make_unique<FluxPacket>(std::move(pckReceive), idReceive);

                //Verify headerId
                auto const header = fluxPacket->retrieveHeader().value();
                if ((header & ~FGE_NET_HEADER_FLAGS_MASK) == FGE_NET_BAD_HEADERID ||
                    (header & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) > 0)
                { //Bad headerId, packet is dismissed
                    continue;
                }

                //Realm and countId is verified by the flux who have the corresponding client

                if (this->g_fluxes.empty())
                {
                    this->g_defaultFlux.pushPacket(std::move(fluxPacket));
                    continue;
                }

                //Try to push packet in a flux
                for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
                {
                    pushingIndex = (pushingIndex + 1) % this->g_fluxes.size();
                    fluxPacket->g_fluxIndex = pushingIndex;
                    if (this->g_fluxes[pushingIndex]->pushPacket(std::move(fluxPacket)))
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
    std::unique_lock<std::mutex> lckServer(this->g_mutexServer);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        //Flux
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

                auto const header = transmissionPacket->packet().retrieveHeader().value();
                if ((header & FGE_NET_HEADER_DO_NOT_REORDER_FLAG) == 0)
                {
                    transmissionPacket->packet().setCountId(itClient->second->advanceCurrentPacketCountId());
                }

                //Sending the packet
                TPacket packet(transmissionPacket->packet());
                this->sendTo(packet, itClient->first);
                itClient->second->resetLastPacketTimePoint();
            }
        }
    }
}

//ClientSideNetUdp

template<class TPacket>
bool ClientSideNetUdp::start(fge::net::Port bindPort,
                             fge::net::IpAddress const& bindIp,
                             fge::net::Port connectRemotePort,
                             fge::net::IpAddress const& connectRemoteAddress,
                             IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }
    this->g_socket.setAddressType(addressType);
    if (this->g_socket.bind(bindPort, bindIp) == fge::net::Socket::ERR_NOERROR)
    {
        if (this->g_socket.connect(connectRemoteAddress, connectRemotePort) == fge::net::Socket::ERR_NOERROR)
        {
            this->g_clientIdentity._ip = connectRemoteAddress;
            this->g_clientIdentity._port = connectRemotePort;

            this->g_running = true;

            this->g_threadReception = std::make_unique<std::thread>(&ClientSideNetUdp::threadReception<TPacket>, this);
            this->g_threadTransmission =
                    std::make_unique<std::thread>(&ClientSideNetUdp::threadTransmission<TPacket>, this);

            return true;
        }
    }
    this->g_socket.close();
    return false;
}

template<class TPacket>
void ClientSideNetUdp::threadReception()
{
    TPacket pckReceive;

    while (this->g_running)
    {
        if (this->g_socket.select(true, 500) == fge::net::Socket::ERR_NOERROR)
        {
            if (this->g_socket.receive(pckReceive) == fge::net::Socket::ERR_NOERROR)
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
                auto fluxPacket = std::make_unique<FluxPacket>(std::move(pckReceive), this->g_clientIdentity);

                //Verify headerId
                auto const header = fluxPacket->retrieveHeader().value();
                if ((header & ~FGE_NET_HEADER_FLAGS_MASK) == FGE_NET_BAD_HEADERID ||
                    (header & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) > 0)
                { //Bad headerId, packet is dismissed
                    continue;
                }

                this->pushPacket(std::move(fluxPacket));
                this->g_receptionNotifier.notify_all();
            }
        }
    }
}
template<class TPacket>
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

                if (!transmissionPacket->packet() || !transmissionPacket->packet().haveCorrectHeaderSize())
                { //Last verification of the packet
                    continue;
                }

                //Applying options
                transmissionPacket->applyOptions(this->_client);

                auto const header = transmissionPacket->packet().retrieveHeader().value();
                if ((header & FGE_NET_HEADER_DO_NOT_REORDER_FLAG) == 0)
                {
                    transmissionPacket->packet().setCountId(this->_client.advanceClientPacketCountId());
                }

                //Sending the packet
                TPacket packet = transmissionPacket->packet();
                this->g_socket.send(packet);
                this->_client.resetLastPacketTimePoint();
            }
        }
    }
}

} // namespace fge::net
