#include "FastEngine/C_server.hpp"

#include <memory>
#include "FastEngine/C_clientList.hpp"

namespace fge
{
namespace net
{

///ServerFluxUdp
FGE_API ServerFluxUdp::~ServerFluxUdp()
{
    this->clear();
}

void FGE_API ServerFluxUdp::clear()
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    std::size_t qsize = this->g_packets.size();
    for (std::size_t i=0; i<qsize; ++i)
    {
        this->g_packets.pop();
    }
}

bool FGE_API ServerFluxUdp::pushPacket(const FluxPacketSharedPtr& fluxPck)
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    if ( this->g_packets.size() >= this->g_maxPackets )
    {
        return false;
    }
    this->g_packets.push(fluxPck);
    return true;
}
void FGE_API ServerFluxUdp::forcePushPacket(const FluxPacketSharedPtr& fluxPck)
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    this->g_packets.push(fluxPck);
}

FluxPacketSharedPtr FGE_API ServerFluxUdp::popNextPacket()
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
std::size_t FGE_API ServerFluxUdp::getPacketsSize() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    return this->g_packets.size();
}
bool FGE_API ServerFluxUdp::isEmpty() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    return this->g_packets.empty();
}

void FGE_API ServerFluxUdp::setMaxPackets(std::size_t n)
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    this->g_maxPackets = n;
}
std::size_t FGE_API ServerFluxUdp::getMaxPackets() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexLocal);
    return this->g_maxPackets;
}

///ServerUdp
FGE_API ServerUdp::ServerUdp() :
    g_threadReception(nullptr),
    g_threadTransmission(nullptr),
    g_running(false)
{
}
FGE_API ServerUdp::~ServerUdp()
{
    this->stop();
}

void FGE_API ServerUdp::stop()
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

fge::net::ServerFluxUdp* FGE_API ServerUdp::newFlux()
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);

    this->g_flux.push_back( std::make_unique<fge::net::ServerFluxUdp>() );
    return this->g_flux.back().get();
}
fge::net::ServerFluxUdp* FGE_API ServerUdp::getFlux(std::size_t index)
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);

    if ( index >= this->g_flux.size() )
    {
        return nullptr;
    }
    return this->g_flux[index].get();
}
fge::net::ServerFluxUdp* FGE_API ServerUdp::getDefaultFlux()
{
    return &this->g_defaultFlux;
}
std::size_t FGE_API ServerUdp::getFluxSize() const
{
    return this->g_flux.size();
}
void FGE_API ServerUdp::delFlux(fge::net::ServerFluxUdp* flux)
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
void FGE_API ServerUdp::delAllFlux()
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    this->g_flux.clear();
}

void FGE_API ServerUdp::repushPacket(const FluxPacketSharedPtr& fluxPck)
{
    if ( (++fluxPck->_fluxCount) >= this->g_flux.size() )
    {
        this->g_defaultFlux.pushPacket(fluxPck);
        return;
    }
    fluxPck->_fluxIndex = (fluxPck->_fluxIndex+1) % this->g_flux.size();
    this->g_flux[fluxPck->_fluxIndex]->forcePushPacket(fluxPck);
}

const fge::net::SocketUdp& FGE_API ServerUdp::getSocket() const
{
    return this->g_socket;
}
fge::net::SocketUdp& FGE_API ServerUdp::getSocket()
{
    return this->g_socket;
}

void FGE_API ServerUdp::notify()
{
    this->g_cv.notify_one();
}
std::mutex& FGE_API ServerUdp::getSendMutex()
{
    return this->g_mutexSend;
}

fge::net::Socket::Error FGE_API ServerUdp::sendTo(fge::net::Packet& pck, const fge::net::IpAddress& ip, fge::net::Port port)
{
    std::lock_guard<std::mutex> lock(this->g_mutexSend);
    return this->g_socket.sendTo(pck, ip, port);
}
fge::net::Socket::Error FGE_API ServerUdp::sendTo(fge::net::Packet& pck, const fge::net::Identity& id)
{
    std::lock_guard<std::mutex> lock(this->g_mutexSend);
    return this->g_socket.sendTo(pck, id._ip, id._port);
}

bool FGE_API ServerUdp::isRunning() const
{
    return this->g_running;
}

void FGE_API ServerUdp::serverThreadTransmission()
{
    std::unique_lock<std::mutex> lckServer(this->g_mutexServer);

    while ( this->g_running )
    {
        this->g_cv.wait_for(lckServer, std::chrono::milliseconds(10));

        //Flux
        for (std::size_t i=0; i<this->g_flux.size(); ++i)
        {
            std::lock_guard<std::mutex> lck(this->g_flux[i]->_clients.getMutex());
            for (auto& client : this->g_flux[i]->_clients)
            {
                if ( !client.second->isPendingPacketsEmpty() )
                {
                    if ( client.second->getLastPacketElapsedTime() >= client.second->getLatency_ms() )
                    {//Ready to send !
                        fge::net::ClientSendQueuePacket buffPck = client.second->popPacket();
                        if (buffPck._pck)
                        {//Last verification of the packet
                            if (buffPck._option == fge::net::QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP)
                            {
                                fge::net::Client::Timestamp tmpTimestamp = fge::net::Client::getTimestamp_ms();
                                buffPck._pck->pack(buffPck._optionArg, &tmpTimestamp, sizeof(fge::net::Client::Timestamp));
                            }
                            this->sendTo(*buffPck._pck, client.first);
                            client.second->resetLastPacketTimePoint();
                        }
                    }
                }
            }
        }
        //Default flux
        std::lock_guard<std::mutex> lck(this->g_defaultFlux._clients.getMutex());
        for (auto& client : this->g_defaultFlux._clients)
        {
            if ( !client.second->isPendingPacketsEmpty() )
            {
                if ( client.second->getLastPacketElapsedTime() >= client.second->getLatency_ms() )
                {//Ready to send !
                    fge::net::ClientSendQueuePacket buffPck = client.second->popPacket();
                    if (buffPck._pck)
                    {//Last verification of the packet
                        if (buffPck._option == fge::net::QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP)
                        {
                            fge::net::Client::Timestamp tmpTimestamp = fge::net::Client::getTimestamp_ms();
                            buffPck._pck->pack(buffPck._optionArg, &tmpTimestamp, sizeof(fge::net::Client::Timestamp));
                        }
                        this->sendTo(*buffPck._pck, client.first);
                        client.second->resetLastPacketTimePoint();
                    }
                }
            }
        }
    }
}

///ServerClientSideUdp
FGE_API ServerClientSideUdp::ServerClientSideUdp() :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_running(false)
{
}
FGE_API ServerClientSideUdp::~ServerClientSideUdp()
{
    this->stop();
}

void FGE_API ServerClientSideUdp::stop()
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

const fge::net::SocketUdp& FGE_API ServerClientSideUdp::getSocket() const
{
    return this->g_socket;
}
fge::net::SocketUdp& FGE_API ServerClientSideUdp::getSocket()
{
    return this->g_socket;
}

void FGE_API ServerClientSideUdp::notify()
{
    this->g_cv.notify_one();
}
std::mutex& FGE_API ServerClientSideUdp::getSendMutex()
{
    return this->g_mutexSend;
}

fge::net::Socket::Error FGE_API ServerClientSideUdp::send(fge::net::Packet& pck)
{
    std::lock_guard<std::mutex> lock(this->g_mutexSend);
    return this->g_socket.send(pck);
}

bool FGE_API ServerClientSideUdp::isRunning() const
{
    return this->g_running;
}

FluxPacketSharedPtr FGE_API ServerClientSideUdp::popNextPacket()
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

std::size_t FGE_API ServerClientSideUdp::getPacketsSize() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    return this->g_packets.size();
}
bool FGE_API ServerClientSideUdp::isEmpty() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    return this->g_packets.empty();
}

void FGE_API ServerClientSideUdp::setMaxPackets(std::size_t n)
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    this->g_maxPackets = n;
}
std::size_t FGE_API ServerClientSideUdp::getMaxPackets() const
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    return this->g_maxPackets;
}

const fge::net::Identity& FGE_API ServerClientSideUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}

bool FGE_API ServerClientSideUdp::waitForPackets(const std::chrono::milliseconds& ms)
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

bool FGE_API ServerClientSideUdp::pushPacket(const FluxPacketSharedPtr& fluxPck)
{
    std::lock_guard<std::mutex> lock(this->g_mutexServer);
    if ( this->g_packets.size() >= this->g_maxPackets )
    {
        return false;
    }
    this->g_packets.push(fluxPck);
    return true;
}

void FGE_API ServerClientSideUdp::serverThreadTransmission()
{
    std::unique_lock<std::mutex> lckServer(this->g_mutexServer);

    while ( this->g_running )
    {
        this->g_cv.wait_for(lckServer, std::chrono::milliseconds(10));

        //Flux
        if ( !this->_client.isPendingPacketsEmpty() )
        {
            if ( this->_client.getLastPacketElapsedTime() >= this->_client.getLatency_ms() )
            {//Ready to send !
                fge::net::ClientSendQueuePacket buffPck = this->_client.popPacket();
                if (buffPck._pck)
                {//Last verification of the packet
                    if (buffPck._option == fge::net::QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP)
                    {
                        fge::net::Client::Timestamp tmpTimestamp = fge::net::Client::getTimestamp_ms();
                        buffPck._pck->pack(buffPck._optionArg, &tmpTimestamp, sizeof(fge::net::Client::Timestamp));
                    }
                    this->send(*buffPck._pck);
                    this->_client.resetLastPacketTimePoint();
                }
            }
        }
    }
}

}//end net
}//end fge
