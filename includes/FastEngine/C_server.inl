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

namespace fge::net
{

//ServerUdp

template<class TPacket>
bool ServerUdp::start(fge::net::Port bindPort, fge::net::IpAddress const& bindIp)
{
    if (this->g_running)
    {
        return false;
    }
    if (this->g_socket.bind(bindPort, bindIp) == fge::net::Socket::ERR_NOERROR)
    {
        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerUdp::serverThreadReception<TPacket>, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerUdp::serverThreadTransmission, this);

        return true;
    }
    return false;
}
template<class TPacket>
bool ServerUdp::start()
{
    if (this->g_running)
    {
        return false;
    }
    if (this->g_socket.isValid())
    {
        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerUdp::serverThreadReception<TPacket>, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerUdp::serverThreadTransmission, this);

        return true;
    }
    return false;
}

template<class TPacket>
void ServerUdp::serverThreadReception()
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
                std::lock_guard<std::mutex> lck(this->g_mutexServer);
                if (this->g_fluxes.empty())
                {
                    this->g_defaultFlux.pushPacket(
                            std::make_shared<fge::net::FluxPacket>(std::move(pckReceive), idReceive));
                    continue;
                }

                pushingIndex = (pushingIndex + 1) % this->g_fluxes.size();
                //Try to push packet in a flux
                for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
                {
                    pushingIndex = (pushingIndex + 1) % this->g_fluxes.size();
                    if (this->g_fluxes[pushingIndex]->pushPacket(
                                std::make_shared<fge::net::FluxPacket>(std::move(pckReceive), idReceive, pushingIndex)))
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

//ServerClientSideUdp

template<class TPacket>
bool ServerClientSideUdp::start(fge::net::Port bindPort,
                                fge::net::IpAddress const& bindIp,
                                fge::net::Port connectRemotePort,
                                fge::net::IpAddress const& connectRemoteAddress)
{
    if (this->g_running)
    {
        return false;
    }
    if (this->g_socket.bind(bindPort, bindIp) == fge::net::Socket::ERR_NOERROR)
    {
        if (this->g_socket.connect(connectRemoteAddress, connectRemotePort) == fge::net::Socket::ERR_NOERROR)
        {
            this->g_clientIdentity._ip = connectRemoteAddress;
            this->g_clientIdentity._port = connectRemotePort;

            this->g_running = true;

            this->g_threadReception =
                    std::make_unique<std::thread>(&ServerClientSideUdp::serverThreadReception<TPacket>, this);
            this->g_threadTransmission =
                    std::make_unique<std::thread>(&ServerClientSideUdp::serverThreadTransmission, this);

            return true;
        }
    }
    this->g_socket.close();
    return false;
}

template<class TPacket>
void ServerClientSideUdp::serverThreadReception()
{
    TPacket pckReceive;

    while (this->g_running)
    {
        if (this->g_socket.select(true, 500) == fge::net::Socket::ERR_NOERROR)
        {
            if (this->g_socket.receive(pckReceive) == fge::net::Socket::ERR_NOERROR)
            {
                this->pushPacket(std::make_shared<fge::net::FluxPacket>(std::move(pckReceive), this->g_clientIdentity));
                this->g_receptionNotifier.notify_all();
            }
        }
    }
}

} // namespace fge::net
