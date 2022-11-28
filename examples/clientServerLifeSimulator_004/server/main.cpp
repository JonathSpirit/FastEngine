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
#include <csignal>

#define TERMINUS(returnValue_) sf::sleep(sf::seconds(2)); return returnValue_;

namespace
{

bool gRunning = true;

}//end

void signalCallbackHandler(int signum)
{
    if (signum == SIGINT)
    {
        std::cout << "received external interrupt signal !" << std::endl;
        gRunning = false;
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::cout << "Life simulator server, a FastEngine example by Guillaume Guillet - version " << LIFESIM_VERSION << std::endl << std::endl;

    if (std::signal(SIGINT, signalCallbackHandler) == SIG_ERR)
    {
        std::cout << "can't set the signal handler ! (continuing anyway)" << std::endl;
    }

    if (!fge::net::Socket::initSocket())
    {
        std::cout << "can't init socket system !" << std::endl;
        TERMINUS(-1)
    }

    fge::net::ServerUdp server;
    std::shared_ptr<fge::net::PacketLZ4> packetSend;
    ///TODO: fge::net::ServerUdp should have a function to create a packet for us. \
    /// This will save some time if the user want to re-change the packet type.

    //Starting the server with an LZ4 compression
    std::cout << "starting the server on port " << LIFESIM_SERVER_PORT << " ..." << std::endl;
    if ( !server.start<fge::net::PacketLZ4>(LIFESIM_SERVER_PORT) )
    {
        std::cout << "can't start the server on this port !" << std::endl;
        TERMINUS(-1)
    }
    std::cout << "OK !" << std::endl << std::endl;

    fge::net::ServerFluxUdp* serverFlux = server.getDefaultFlux();
    fge::net::ClientList& clients = serverFlux->_clients;

    clients.watchEvent(true);

    //We must register all classes that we will use
    std::cout << "registering all classes ..." << std::endl;
    {
        bool valid = true;
        valid &= fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Creature>()) );
        valid &= fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Food>()) );
        valid &= fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Drink>()) );
        if (!valid)
        {
            std::cout << "error during class registrations !" << std::endl;
            TERMINUS(-1)
        }
        std::cout << "OK !" << std::endl;
    }

    //Creating the scene
    fge::Scene mainScene;

    //Ask the scene to watch network events
    mainScene.watchEvent(true);

    //Adding some creatures
    for (std::size_t i=0; i<LIFESIM_START_CREATURES_COUNT; ++i)
    {
        mainScene.newObject(FGE_NEWOBJECT(ls::Creature, ls::GetRandomPosition()), FGE_SCENE_PLAN_MIDDLE);
    }

    //Prepare some clocks
    fge::Clock clockNewFood;
    fge::Clock clockWorldUpdate;
    fge::Clock clockTimeout;
    fge::Clock deltaTime;

    //fge::Event is not used in this application, but required
    fge::Event event;

    while (gRunning)
    {
        //Spawn some new foods
        if (clockNewFood.reached(LIFESIM_TIME_NEW_FOODS))
        {
            clockNewFood.restart();

            for (std::size_t i=0; i<LIFESIM_NEW_FOOD_COUNT; ++i)
            {
                mainScene.newObject(FGE_NEWOBJECT(ls::Food, ls::GetRandomPosition()), FGE_SCENE_PLAN_BACK);
            }
            for (std::size_t i=0; i<LIFESIM_NEW_DRINK_COUNT; ++i)
            {
                mainScene.newObject(FGE_NEWOBJECT(ls::Drink, ls::GetRandomPosition()), FGE_SCENE_PLAN_BACK);
            }
        }

        //Checking for a world update
        if (clockWorldUpdate.reached(LIFESIM_TIME_WORLDTICK))
        {
            clockWorldUpdate.restart();

            std::size_t creatureCount = 0;
            for (auto it=mainScene.begin(); it!=mainScene.end();)
            {
                //Avoid cast on non-CustomObject by only looking for object type
                if ((*it)->getType() == fge::ObjectType::TYPE_OBJECT)
                {
                    auto* object = (*it)->getObject<ls::CustomObject>();
                    if ( object->worldTick() )
                    {//This object must be destroyed
                        fge::ObjectSid sid = (*it)->getSid();
                        ++it;
                        mainScene.delObject(sid);
                    }
                    else
                    {
                        if ( std::strcmp(object->getClassName(), "LS:OBJ:CREATURE") == 0 )
                        {
                            ++creatureCount;
                        }
                        ++it;
                    }
                }
            }

            //Check if there is still a creature alive
            if (creatureCount == 0)
            {
                //We destroy everything and add new creatures
                mainScene.delAllObject(true);
                for (std::size_t i=0; i<LIFESIM_START_CREATURES_COUNT; ++i)
                {
                    mainScene.newObject(FGE_NEWOBJECT(ls::Creature, ls::GetRandomPosition()), FGE_SCENE_PLAN_MIDDLE);
                }
            }
        }

        //Handling clients timeout
        if (clockTimeout.reached(LIFESIM_TIME_TIMEOUT))
        {
            clockTimeout.restart();

            //In order to iterate through all clients, we have to acquire a lock
            auto clientsLock = clients.acquireLock();

            for (auto it=clients.begin(clientsLock); it!=clients.end(clientsLock);)
            {
                auto timeout = (*it).second->_data[LIFESIM_CLIENTDATA_TIMEOUT].get<fge::PuintType>().value_or(0) + 1;
                if (timeout >= LIFESIM_TIMEOUT_COUNT)
                {
                    std::cout << "user : " << (*it).first._ip.toString() << " disconnected (timeout) !" << std::endl;

                    auto packet = std::make_shared<fge::net::PacketLZ4>();

                    fge::net::SetHeader(*packet, ls::LS_PROTOCOL_ALL_GOODBYE) << "timeout";
                    server.sendTo(*packet, (*it).first); ///TODO: we have to stop using .sendTo method and let the server thread handle all packets

                    it = clients.remove(it, clientsLock);
                }
                else
                {
                    (*it).second->_data[LIFESIM_CLIENTDATA_TIMEOUT].set(timeout);
                    ++it;
                }
            }
        }

        //Handling clients packets
        std::size_t pckSize = serverFlux->getPacketsSize();
        for (std::size_t i=0; i<pckSize; ++i)
        {
            //Popping the next packet
            auto fluxPacket = serverFlux->popNextPacket();

            //Check if we already know the packet identity
            auto client = clients.get(fluxPacket->_id);

            //Prepare a sending packet
            packetSend = std::make_shared<fge::net::PacketLZ4>();

            //Retrieve the packet header
            switch (fge::net::GetHeader(fluxPacket->_pck))
            {
            case ls::LS_PROTOCOL_ALL_PING:
                fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_ALL_PONG);

                if (client)
                {
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
                    //Retrieving "Client To Server" timestamp
                    //and the "Server To Client" latency
                    fge::net::Client::Timestamp timestampCTOS;
                    fge::net::Client::Timestamp timestampSTOC;
                    fge::net::Client::Latency_ms latencyCTOS;
                    fge::net::Client::Latency_ms latencyCorrectorSTOC;
                    fluxPacket->_pck >> timestampCTOS >> timestampSTOC >> latencyCTOS >> latencyCorrectorSTOC;

                    //We can compute the "Server To Client" latency with the provided server timestamp and the packet timestamp
                    if (latencyCorrectorSTOC != FGE_NET_BAD_LATENCY)
                    {
                        auto latencySTOC = fge::net::Client::computeLatency_ms(timestampSTOC, fluxPacket->_timestamp);
                        latencySTOC = latencyCorrectorSTOC>latencySTOC ? 0 : latencySTOC-latencyCorrectorSTOC;
                        client->setSTOCLatency_ms(latencySTOC/2);
                    }

                    std::cout << latencyCorrectorSTOC << std::endl;

                    //We can set the required latency of the server
                    client->setCTOSLatency_ms(latencyCTOS);
                    if (!client->getCorrectorLatency().has_value())
                    {
                        client->setCorrectorTimestamp(fluxPacket->_timestamp);
                    }
                    client->_data.setProperty(LIFESIM_CLIENTDATA_TIMESTAMP, timestampCTOS);
                    client->_data.setProperty(LIFESIM_CLIENTDATA_TIMEOUT, 0);
                }
                break;
            case ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME:
                fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME);

                if (client != nullptr)
                {
                    //The client is already connected, so we just send "true"
                    *packetSend << true;
                    client->pushPacket({packetSend});
                }
                else
                {
                    //The potential client is not connected

                    //We extract 2 "really super secret" strings for validating the connection
                    std::string connectionText1;
                    std::string connectionText2;

                    //Before extracting a string from the packet, we must be sure that the string
                    //will have a valid size range.
                    FGE_NET_RULES_START
                        fge::net::rules::RSizeMustEqual<std::string>(sizeof(LIFESIM_CONNECTION_TEXT1), {fluxPacket->_pck});
                    FGE_NET_RULES_AFFECT_END_ELSE(connectionText1,)

                    FGE_NET_RULES_START
                        fge::net::rules::RSizeMustEqual<std::string>(sizeof(LIFESIM_CONNECTION_TEXT2), {fluxPacket->_pck});
                    FGE_NET_RULES_AFFECT_END_ELSE(connectionText2,)

                    fge::net::Client::Timestamp timestampCTOS;
                    fluxPacket->_pck >> timestampCTOS;

                    //Check if the packet is still valid after extraction and/or rules
                    if (fluxPacket->_pck)
                    {
                        //Check if those text is respected
                        if (connectionText1 == LIFESIM_CONNECTION_TEXT1 && connectionText2 == LIFESIM_CONNECTION_TEXT2)
                        {
                            //The client is valid, we can connect him
                            *packetSend << true << timestampCTOS;
                            const std::size_t timestampPos = packetSend->getDataSize();
                            packetSend->append(sizeof(fge::net::Client::Timestamp));

                            const std::size_t latencyCorrectorPos = packetSend->getDataSize();
                            packetSend->append(sizeof(fge::net::Client::Latency_ms));

                            std::cout << "new user : " << fluxPacket->_id._ip.toString() << " connected !" << std::endl;

                            //Create the new client with the packet identity
                            client = std::make_shared<fge::net::Client>();
                            clients.add(fluxPacket->_id, client);

                            //Storing the client timestamp
                            client->_data.setProperty(LIFESIM_CLIENTDATA_TIMESTAMP, timestampCTOS);
                            client->setCorrectorTimestamp(fluxPacket->_timestamp);

                            //Ask the server thread to automatically update the timestamp just before sending it
                            client->pushPacket({packetSend, {{fge::net::ClientSendQueuePacket::Options::UPDATE_TIMESTAMP, timestampPos},
                                                             {fge::net::ClientSendQueuePacket::Options::UPDATE_CORRECTION_LATENCY, latencyCorrectorPos}}});

                            //We will send a full scene update to the client too
                            packetSend = std::make_shared<fge::net::PacketLZ4>();
                            fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_S_UPDATE_ALL);
                            mainScene.pack(*packetSend);

                            client->pushPacket({packetSend});
                            break;
                        }
                    }

                    //Something is not right, we will send "false" to the potential client
                    *packetSend << false;
                    server.sendTo(*packetSend, fluxPacket->_id);
                }
                break;
            default:
                break;
            }
        }

        //Scene update
        mainScene.update(event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime.restart()));

        //Sending scene update to clients
        {
            //We do a client checkup for every object, in order to prepare data for new clients or destroying old clients
            mainScene.clientsCheckup(clients);
            //Same with the scene
            mainScene.clientsCheckupEvent(clients);

            //Client event must be manually cleared after use
            clients.clearClientEvent();

            auto clientsLock = clients.acquireLock();
            for (auto it=clients.begin(clientsLock); it!=clients.end(clientsLock); ++it)
            {
                //Make sure that the server thread is not busy with another packet
                if ( !(*it).second->isPendingPacketsEmpty() )
                {
                    continue;
                }

                packetSend = std::make_shared<fge::net::PacketLZ4>();
                fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_S_UPDATE);

                //We append the byte size of a timestamp here cause the server thread will automatically update it
                //just before sending the packet (more precise timestamp).
                packetSend->append(sizeof(fge::net::Client::Timestamp));

                //We retrieve the stored "Client To Server" timestamp and re-sent it to the client as it is not useful for us.
                auto timestampCTOS = (*it).second->_data[LIFESIM_CLIENTDATA_TIMESTAMP].get<fge::net::Client::Timestamp>().value_or(0);
                *packetSend << timestampCTOS << (*it).second->getSTOCLatency_ms();

                const std::size_t latencyCorrectorPos = packetSend->getDataSize();
                packetSend->append(sizeof(fge::net::Client::Latency_ms));

                //We can now push all scene modification by clients
                mainScene.packModification(*packetSend, (*it).first);
                //And push all scene events by clients
                mainScene.packWatchedEvent(*packetSend, (*it).first);

                //We send the packet to the client with the QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP option for the server thread
                (*it).second->pushPacket({packetSend, {{fge::net::ClientSendQueuePacket::Options::UPDATE_TIMESTAMP, sizeof(fge::net::PacketHeader)},
                                                       {fge::net::ClientSendQueuePacket::Options::UPDATE_CORRECTION_LATENCY, latencyCorrectorPos}}});

                //Notify the server that a packet as been pushed
                server.notify();
            }
        }

        //Server tick
        sf::sleep(sf::milliseconds(LIFESIM_SERVER_TICK));
    }

    std::cout << "shutdown ..." << std::endl;
    server.stop();

    //Saving the scene
    mainScene.saveInFile("lifeSimulatorScene.json");

    fge::net::Socket::uninitSocket();

    TERMINUS(0)
}

