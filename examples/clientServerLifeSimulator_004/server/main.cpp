#include <definition.hpp>
#include <FastEngine/C_server.hpp>
#include <FastEngine/manager/network_manager.hpp>
#include <FastEngine/manager/reg_manager.hpp>
#include <FastEngine/C_packetLZ4.hpp>
#include <FastEngine/C_clock.hpp>

#include <C_creature.hpp>
#include <C_food.hpp>
#include <C_drink.hpp>

#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::cout << "Life simulator server, a FastEngine example by Guillaume Guillet - version " << LIFESIM_VERSION << std::endl << std::endl;

    if (!fge::net::Socket::initSocket())
    {
        std::cout << "Can't init socket system !" << std::endl;
        return -1;
    }

    bool running = true;

    fge::net::ServerUdp server;
    std::shared_ptr<fge::net::PacketLZ4> packetSend;

    std::cout << "Starting the server on port " << LIFESIM_SERVER_PORT << " ..." << std::endl;
    if ( !server.start<fge::net::PacketLZ4>(LIFESIM_SERVER_PORT) )
    {
        std::cout << "Can't start the server on this port !" << std::endl;
        sf::sleep(sf::seconds(2));
        return 1;
    }
    std::cout << "OK !" << std::endl << std::endl;

    fge::net::ServerFluxUdp* serverFlux = server.getDefaultFlux();
    fge::net::ClientList& clients = serverFlux->_clients;

    clients.watchEvent(true);

    //We must register all classes that we will use
    std::cout << "registering all classes ..." << std::endl;
    std::cout << fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Creature>()) ) << std::endl;
    std::cout << fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Food>()) ) << std::endl;
    std::cout << fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Drink>()) ) << std::endl;

    //Creating the scene
    fge::Scene mainScene;

    //Adding some new creatures
    for (std::size_t i=0; i<20; ++i)
    {
        mainScene.newObject(FGE_NEWOBJECT(ls::Creature, ls::GetRandomPosition()), FGE_SCENE_PLAN_MIDDLE );
    }

    mainScene.watchEvent(true);

    fge::Clock clockNewFood;
    fge::Clock clockStatUpdate;
    fge::Clock clockTimeout;
    fge::Clock deltaTime;

    fge::Clock endClock; //TODO

    fge::Event event;

    while (running)
    {
        if (endClock.reached(std::chrono::seconds{60}))
        {
            running = false;
        }

        ///TIMER
        if ( clockNewFood.reached(std::chrono::milliseconds{10000}) )
        {
            clockNewFood.restart();

            for (std::size_t i=0; i<10; ++i)
            {
                mainScene.newObject(FGE_NEWOBJECT(ls::Food, ls::GetRandomPosition()), FGE_SCENE_PLAN_BACK );
            }
            for (std::size_t i=0; i<10; ++i)
            {
                mainScene.newObject(FGE_NEWOBJECT(ls::Drink, ls::GetRandomPosition()), FGE_SCENE_PLAN_BACK );
            }
        }
        if ( clockStatUpdate.reached(std::chrono::milliseconds{20000}) )
        {///WORLD TICK
            clockStatUpdate.restart();

            for (auto it=mainScene.begin(); it!=mainScene.end();)
            {
                auto* tmpObj = reinterpret_cast<ls::CustomObject*>( (*it)->getObject() );
                if ( tmpObj->worldTick() )
                {//dead object
                    fge::ObjectSid sid = (*it)->getSid();
                    ++it;
                    mainScene.delObject(sid);
                }
                else
                {
                    ++it;
                }
            }
        }

        ///Handling clients timeout
        if (clockTimeout.reached(std::chrono::milliseconds{20}))
        {
            clockTimeout.restart();
            auto clientsLock = clients.acquireLock();
            for (auto it=clients.begin(clientsLock); it!=clients.end(clientsLock);)
            {
                auto timeout = (*it).second->_data[LIFESIM_CLIENTDATA_TIMEOUT].get<fge::PuintType>().value_or(0) + 1;
                if (timeout >= LIFESIM_TIMEOUT)
                {
                    std::cout << "User : " << (*it).first._ip.toString() << " disconnected (timeout) !" << std::endl;

                    auto packet = std::make_shared<fge::net::PacketLZ4>();

                    fge::net::SetHeader(*packet, ls::LS_PROTOCOL_ALL_GOODBYE) << "timeout";
                    server.sendTo(*packet, (*it).first);

                    it = clients.remove(it, clientsLock);
                }
                else
                {
                    (*it).second->_data[LIFESIM_CLIENTDATA_TIMEOUT].set(timeout);
                    ++it;
                }
            }
        }

        ///Handling packets
        std::size_t pckSize = serverFlux->getPacketsSize();
        for (std::size_t i=0; i<pckSize; ++i)
        {
            auto fluxPacket = serverFlux->popNextPacket();
            auto client = clients.get(fluxPacket->_id);
            packetSend = std::make_shared<fge::net::PacketLZ4>();

            switch ( fge::net::GetHeader(fluxPacket->_pck) )
            {
            case ls::LS_PROTOCOL_ALL_PING:
                fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_ALL_PING);
                if (client)
                {
                    client->_data.setProperty(LIFESIM_CLIENTDATA_TIMEOUT, 0);
                    client->pushPacket({packetSend});
                }
                else
                {
                    server.sendTo(*packetSend, fluxPacket->_id);
                }
                break;
            case ls::LS_PROTOCOL_C_UPDATE:
                if (client)
                {
                    fge::net::Client::Timestamp timestampCTOS;
                    fge::net::Client::Latency_ms latencySTOC;
                    fluxPacket->_pck >> timestampCTOS >> latencySTOC;

                    auto latencyCTOS = fge::net::Client::computeLatency_ms(timestampCTOS, fluxPacket->_timestamp);

                    client->setLatency_ms(latencySTOC);
                    client->_data.setProperty(LIFESIM_CLIENTDATA_LATENCY, latencyCTOS);
                    client->_data.setProperty(LIFESIM_CLIENTDATA_TIMEOUT, 0);
                }
                break;
            case ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME:
                fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME);
                if (client != nullptr)
                {
                    *packetSend << true;
                    client->pushPacket({packetSend});
                }
                else
                {
                    std::string connectionText1;
                    std::string connectionText2;
                    fluxPacket->_pck >> connectionText1 >> connectionText2;
                    if (connectionText1 == "Hello" && connectionText2 == LIFESIM_CONNECTION_TEXT)
                    {
                        *packetSend << true;
                        std::cout << "New user : " << fluxPacket->_id._ip.toString() << " connected !" << std::endl;
                        client = std::make_shared<fge::net::Client>();
                        clients.add(fluxPacket->_id, client);

                        client->pushPacket({packetSend});

                        packetSend = std::make_shared<fge::net::PacketLZ4>();
                        fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_S_UPDATE_ALL);
                        mainScene.pack(*packetSend);

                        client->pushPacket({packetSend});
                    }
                    else
                    {
                        *packetSend << false;
                        server.sendTo(*packetSend, fluxPacket->_id);
                    }
                }
                break;
            default:
                break;
            }
        }

        ///Scene update
        mainScene.update(event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime.restart()));

        ///Sending scene update to clients
        {
            mainScene.clientsCheckup(clients);
            mainScene.clientsCheckupEvent(clients);

            clients.clearClientEvent();

            auto clientsLock = clients.acquireLock();
            for (auto it=clients.begin(clientsLock); it!=clients.end(clientsLock); ++it)
            {
                packetSend = std::make_shared<fge::net::PacketLZ4>();
                fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_S_UPDATE);

                //Timestamp TIMESTAMP_STOC_SERVER
                //Latency_ms LATENCY_CTOS
                packetSend->append(sizeof(fge::net::Client::Timestamp));
                auto latencyCTOS = (*it).second->_data[LIFESIM_CLIENTDATA_LATENCY].get<fge::net::Client::Latency_ms>().value_or(0);
                *packetSend << latencyCTOS;

                mainScene.packModification(*packetSend, (*it).first);
                mainScene.packWatchedEvent(*packetSend, (*it).first);

                (*it).second->pushPacket({packetSend, fge::net::ClientSendQueuePacketOptions::QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP,
                                          sizeof(fge::net::PacketHeader)});
            }
        }

        ///Tick de 10ms
        sf::sleep(sf::milliseconds(LIFESIM_SERVER_TICK));
    }

    std::cout << "Shutdown ..." << std::endl;
    server.stop();

    mainScene.saveInFile("lifeSimulatorScene.json");

    fge::net::Socket::uninitSocket();

    sf::sleep(sf::seconds(2));

    return 0;
}

