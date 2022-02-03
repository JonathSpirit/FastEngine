namespace fge
{
namespace net
{

template<typename Tpacket>
bool ServerUdp::start(fge::net::Port port, const fge::net::IpAddress& ip)
{
    if ( this->g_running )
    {
        return false;
    }
    if ( this->g_socket.bind(port, ip) == fge::net::Socket::ERR_NOERROR )
    {
        this->g_running = true;

        this->g_threadReception = new std::thread(&ServerUdp::serverThreadReception<Tpacket>, this);
        this->g_threadTransmission = new std::thread(&ServerUdp::serverThreadTransmission, this);

        return true;
    }
    return false;
}

template<typename Tpacket>
void ServerUdp::serverThreadReception()
{
    fge::net::Identity idReceive;
    Tpacket pckReceive;
    std::size_t pushingIndex = 0;

    while ( this->g_running )
    {
        if ( this->g_socket.select(true, 500) == fge::net::Socket::ERR_NOERROR )
        {
            if ( this->g_socket.receiveFrom(pckReceive, idReceive._ip, idReceive._port) == fge::net::Socket::ERR_NOERROR )
            {
                std::lock_guard<std::mutex> lck(this->g_mutexServer);
                if ( !this->g_flux.size() )
                {
                    this->g_defaultFlux.pushPacket( std::make_shared<fge::net::FluxPacket>(std::move(pckReceive), idReceive) );
                    continue;
                }

                pushingIndex = (pushingIndex+1) % this->g_flux.size();
                //Try to push packet in a flux
                for (std::size_t i=0; i<this->g_flux.size(); ++i)
                {
                    pushingIndex = (pushingIndex+1) % this->g_flux.size();
                    if ( this->g_flux[pushingIndex]->pushPacket( std::make_shared<fge::net::FluxPacket>(std::move(pckReceive), idReceive, pushingIndex) ) )
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

}//end net
}//end fge
