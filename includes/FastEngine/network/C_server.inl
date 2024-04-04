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
        _packet(pck),
        g_id(id),
        g_timestamp(fge::net::Client::getTimestamp_ms()),
        g_fluxIndex(fluxIndex),
        g_fluxCount(fluxCount)
{}
inline FluxPacket::FluxPacket(fge::net::Packet&& pck,
                              fge::net::Identity const& id,
                              std::size_t fluxIndex,
                              std::size_t fluxCount) :
        _packet(std::move(pck)),
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
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission, this);

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
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission, this);

        return true;
    }
    return false;
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
                if (this->g_fluxes.empty())
                {
                    this->g_defaultFlux.pushPacket(
                            std::make_unique<fge::net::FluxPacket>(std::move(pckReceive), idReceive));
                    continue;
                }

                pushingIndex = (pushingIndex + 1) % this->g_fluxes.size();
                //Try to push packet in a flux
                for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
                {
                    pushingIndex = (pushingIndex + 1) % this->g_fluxes.size();
                    if (this->g_fluxes[pushingIndex]->pushPacket(
                                std::make_unique<fge::net::FluxPacket>(std::move(pckReceive), idReceive, pushingIndex)))
                    {
                        //Packet is correctly pushed
                        break;
                    }
                }
                //If every flux is busy, the new packet is dismissed
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
            this->g_threadTransmission = std::make_unique<std::thread>(&ClientSideNetUdp::threadTransmission, this);

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

                this->pushPacket(std::make_unique<fge::net::FluxPacket>(std::move(pckReceive), this->g_clientIdentity));
                this->g_receptionNotifier.notify_all();
            }
        }
    }
}

} // namespace fge::net
