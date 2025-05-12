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

#include "FastEngine/C_clock.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/fge_version.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/manager/reg_manager.hpp"
#include "FastEngine/network/C_packetLZ4.hpp"
#include "FastEngine/network/C_server.hpp"
#include "definition.hpp"

#include "C_creature.hpp"
#include "C_drink.hpp"
#include "C_food.hpp"

#include <csignal>
#include <iostream>

#define TERMINUS(returnValue_)                                                                                         \
    fge::Sleep(std::chrono::seconds(2));                                                                               \
    return returnValue_

namespace
{

bool gRunning = true;

} // namespace

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
    std::cout << "Life simulator server, a FastEngine example by Guillaume Guillet - version " << LIFESIM_VERSION
              << std::endl
              << std::endl;

    if (std::signal(SIGINT, signalCallbackHandler) == SIG_ERR)
    {
        std::cout << "can't set the signal handler ! (continuing anyway)" << std::endl;
    }

    if (!fge::net::Socket::initSocket())
    {
        std::cout << "can't init socket system !" << std::endl;
        TERMINUS(-1);
    }

    //Enable virtual terminal sequence support
    std::cout << "virtual terminal sequence support : " << std::boolalpha << fge::SetVirtualTerminalSequenceSupport()
              << std::endl;

    std::string title = "Life simulator server, FastEngine " + std::string{FGE_VERSION_FULL_WITHTAG_STRING};
    fge::SetConsoleCmdTitle(title.c_str());

    //Creating the server
    //Here you can choose between Ipv4 or Ipv6
    fge::net::ServerSideNetUdp server(fge::net::IpAddress::Types::Ipv4);

    //Starting the server with an LZ4 compression
    std::cout << "starting the server on port " << LIFESIM_SERVER_PORT << " ..." << std::endl;
    if (!server.start(LIFESIM_SERVER_PORT, fge::net::IpAddress::Any(server.getAddressType())))
    {
        std::cout << "can't start the server on this port !" << std::endl;
        TERMINUS(-1);
    }
    std::cout << "OK !" << std::endl << std::endl;

    auto* serverFlux = server.getDefaultFlux();
    fge::net::ClientList& clients = serverFlux->_clients;

    clients.watchEvent(true);

    //We must register all classes that we will use
    std::cout << "registering all classes ..." << std::endl;
    {
        bool valid = true;
        valid &= fge::reg::RegisterNewClass<ls::Creature>();
        valid &= fge::reg::RegisterNewClass<ls::Food>();
        valid &= fge::reg::RegisterNewClass<ls::Drink>();
        if (!valid)
        {
            std::cout << "error during class registrations !" << std::endl;
            TERMINUS(-1);
        }
        std::cout << "OK !" << std::endl;
    }

    //Creating the scene
    fge::Scene mainScene;

    //Ask the scene to watch network events
    mainScene.watchEvent(true);

    //Adding some creatures
    for (std::size_t i = 0; i < LIFESIM_START_CREATURES_COUNT; ++i)
    {
        mainScene.newObject(FGE_NEWOBJECT(ls::Creature, ls::GetRandomPosition()), FGE_SCENE_PLAN_MIDDLE);
    }

    //Prepare some clocks
    fge::Clock clockNewFood;
    fge::Clock clockWorldUpdate;
    fge::Clock deltaTime;

    //fge::Event is not used in this application, but required
    fge::Event event;

    //Handling clients timeout
    serverFlux->_onClientTimeout.addLambda([](fge::net::ClientSharedPtr client, fge::net::Identity const& id) {
        std::cout << "user : " << id.toString() << " disconnected (timeout) !" << std::endl;
    });
    serverFlux->_onClientDisconnected.addLambda([](fge::net::ClientSharedPtr client, fge::net::Identity const& id) {
        std::cout << "user : " << id.toString() << " disconnected !" << std::endl;
    });

    //Handling clients connection
    serverFlux->_onClientConnected.addLambda([](fge::net::ClientSharedPtr const& client, fge::net::Identity const& id) {
        client->getStatus().setTimeout(LIFESIM_TIME_TIMEOUT);
    });

    //Handling clients return packet
    serverFlux->_onClientReturnPacket.addLambda([](fge::net::ClientSharedPtr const& client, fge::net::Identity id,
                                                   fge::net::ReceivedPacketPtr const& packet) {
        std::cout << "received update from : " << packet->getIdentity().toString() << std::endl;

        //We reset the timeout
        client->getStatus().resetTimeout();
    });

    while (gRunning)
    {
        //Spawn some new foods
        if (clockNewFood.reached(LIFESIM_TIME_NEW_FOODS))
        {
            clockNewFood.restart();

            for (std::size_t i = 0; i < LIFESIM_NEW_FOOD_COUNT; ++i)
            {
                mainScene.newObject(FGE_NEWOBJECT(ls::Food, ls::GetRandomPosition()), FGE_SCENE_PLAN_BACK);
            }
            for (std::size_t i = 0; i < LIFESIM_NEW_DRINK_COUNT; ++i)
            {
                mainScene.newObject(FGE_NEWOBJECT(ls::Drink, ls::GetRandomPosition()), FGE_SCENE_PLAN_BACK);
            }
        }

        //Checking for a world update
        if (clockWorldUpdate.reached(LIFESIM_TIME_WORLDTICK))
        {
            clockWorldUpdate.restart();

            std::size_t creatureCount = 0;
            for (auto it = mainScene.begin(); it != mainScene.end();)
            {
                //Avoid cast on non-CustomObject by only looking for object type
                if ((*it)->getType() == fge::ObjectTypes::OBJECT)
                {
                    auto* object = (*it)->getObject<ls::CustomObject>();
                    if (object->worldTick())
                    { //This object must be destroyed
                        fge::ObjectSid sid = (*it)->getSid();
                        ++it;
                        mainScene.delObject(sid);
                    }
                    else
                    {
                        if (std::strcmp(object->getClassName(), "LS:OBJ:CREATURE") == 0)
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
                for (std::size_t i = 0; i < LIFESIM_START_CREATURES_COUNT; ++i)
                {
                    mainScene.newObject(FGE_NEWOBJECT(ls::Creature, ls::GetRandomPosition()), FGE_SCENE_PLAN_MIDDLE);
                }
            }
        }

        //Handling clients packets
        fge::net::ClientSharedPtr client;
        fge::net::ReceivedPacketPtr packet;
        fge::net::FluxProcessResults processResult;
        do {
            processResult = serverFlux->process(client, packet, true);
            if (processResult != fge::net::FluxProcessResults::USER_RETRIEVABLE)
            {
                continue;
            }

            //Prepare a sending packet
            auto transmissionPacket = fge::net::CreatePacket();

            //Retrieve the packet header
            switch (packet->retrieveHeaderId().value())
            {
            case ls::LS_PROTOCOL_ALL_PING:
                transmissionPacket->setHeaderId(ls::LS_PROTOCOL_ALL_PONG);

                if (client)
                {
                    transmissionPacket->doNotReorder();
                    client->pushPacket(std::move(transmissionPacket));
                }
                else
                {
                    server.sendTo(transmissionPacket, packet->getIdentity());
                }
                break;
            case ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME:
            {
                transmissionPacket->setHeaderId(ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME);
                transmissionPacket->addFlags(FGE_NET_HEADER_DO_NOT_REORDER_FLAG);

                if (client->getStatus().getNetworkStatus() == fge::net::ClientStatus::NetworkStatus::AUTHENTICATED)
                {
                    //The client is already connected, so we just send "true"
                    transmissionPacket->packet() << true;
                    client->pushPacket(std::move(transmissionPacket));
                }
                else
                { //The potential client is not connected

                    using namespace fge::net::rules;

                    //We extract 2 "really super secret" strings for validating the connection
                    std::string connectionText1;
                    std::string connectionText2;

                    //Before extracting a string from the packet, we must be sure that the string
                    //will have a valid size range.
                    RValid(RSizeMustEqual<std::string>(sizeof(LIFESIM_CONNECTION_TEXT1) - 1, packet->packet(),
                                                       &connectionText1))
                            .and_then([&](auto& chain) {
                        return RValid(RSizeMustEqual<std::string>(sizeof(LIFESIM_CONNECTION_TEXT2) - 1, chain,
                                                                  &connectionText2));
                    })
                            .and_then([&](auto& chain) {
                        //At this point, every extraction as been successful, so we can continue
                        //Check if those text is respected
                        if (connectionText1 == LIFESIM_CONNECTION_TEXT1 && connectionText2 == LIFESIM_CONNECTION_TEXT2)
                        {
                            //The client is valid, we can connect him
                            transmissionPacket->packet() << true;
                            transmissionPacket->doNotReorder();

                            std::cout << "new user : " << packet->getIdentity().toString() << " connected !"
                                      << std::endl;

                            //Create the new client with the packet identity
                            //client = std::make_shared<fge::net::Client>();
                            //clients.add(packet->getIdentity(), client);
                            client->getStatus().setNetworkStatus(fge::net::ClientStatus::NetworkStatus::AUTHENTICATED);

                            //Pack data required by the LatencyPlanner in order to compute latency
                            client->_latencyPlanner.pack(transmissionPacket);

                            //Ask the server thread to automatically update the timestamp just before sending it
                            client->pushPacket(std::move(transmissionPacket));

                            //We will send a full scene update to the client too
                            transmissionPacket = fge::net::CreatePacket(ls::LS_PROTOCOL_S_UPDATE_ALL);
                            transmissionPacket->doNotDiscard();
                            mainScene.pack(transmissionPacket->packet(), packet->getIdentity());

                            std::cout << "transmitting full update with scene update : " << mainScene.getUpdateCount()
                                      << std::endl;

                            client->pushPacket(std::move(transmissionPacket));
                        }

                        return chain;
                    }).on_error([&]([[maybe_unused]] auto& chain) {
                        //Something is not right, we will send "false" to the potential client
                        transmissionPacket->packet() << false;
                        server.sendTo(transmissionPacket, packet->getIdentity());
                    });
                }
            }
            break;
            case ls::LS_PROTOCOL_C_ASK_FULL_UPDATE:
                if (client)
                {
                    transmissionPacket->setHeaderId(ls::LS_PROTOCOL_S_UPDATE_ALL);
                    transmissionPacket->doNotDiscard();
                    mainScene.pack(transmissionPacket->packet(), packet->getIdentity());
                    client->advanceCurrentRealm();
                    client->pushPacket(std::move(transmissionPacket));
                    server.notifyTransmission();
                }
                break;
            default:
                break;
            }
        } while (processResult != fge::net::FluxProcessResults::NONE_AVAILABLE);

        //Scene update
        mainScene.update(event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime.restart()));

        //Sending scene update to clients
        {
            //We do a client checkup, in order to prepare network data for new clients or destroying old clients
            mainScene.clientsCheckup(clients);

            //Client event must be manually cleared after use
            clients.clearClientEvent();

            auto clientsLock = clients.acquireLock();
            for (auto it = clients.begin(clientsLock); it != clients.end(clientsLock); ++it)
            {
                if (it->second._client->getStatus().getNetworkStatus() !=
                    fge::net::ClientStatus::NetworkStatus::AUTHENTICATED)
                {
                    continue;
                }

                //Make sure that the client is not busy with another packet
                if (!it->second._client->isPendingPacketsEmpty())
                {
                    continue;
                }

                auto transmissionPacket = fge::net::CreatePacket();
                transmissionPacket->setHeaderId(ls::LS_PROTOCOL_S_UPDATE);

                //Pack data required by the LatencyPlanner in order to compute latency
                it->second._client->_latencyPlanner.pack(transmissionPacket);

                //We can now push all scene modification by clients
                mainScene.packModification(transmissionPacket->packet(), it->first);
                //And push all scene events by clients
                mainScene.packWatchedEvent(transmissionPacket->packet(), it->first);

                //We send the packet to the client with the QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP option for the server thread
                it->second._client->pushPacket(std::move(transmissionPacket));

                //Notify the server that a packet as been pushed
                server.notifyTransmission();
            }
        }

        //Server tick
        fge::Sleep(std::chrono::milliseconds(LIFESIM_SERVER_TICK));
    }

    std::cout << "disconnecting clients" << std::endl;
    serverFlux->disconnectAllClients(std::chrono::milliseconds(2000));

    std::cout << "shutdown ..." << std::endl;
    server.stop();

    //Saving the scene
    mainScene.saveInFile("lifeSimulatorScene.json");

    fge::net::Socket::uninitSocket();

    TERMINUS(0);
}
