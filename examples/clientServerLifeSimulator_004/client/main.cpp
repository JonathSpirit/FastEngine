#include <definition.hpp>
#include <FastEngine/C_server.hpp>
#include <FastEngine/manager/network_manager.hpp>
#include <FastEngine/manager/reg_manager.hpp>
#include <FastEngine/manager/texture_manager.hpp>
#include <FastEngine/manager/font_manager.hpp>
#include <FastEngine/manager/anim_manager.hpp>
#include <FastEngine/manager/audio_manager.hpp>
#include <FastEngine/object/C_objText.hpp>
#include <FastEngine/object/C_objWindow.hpp>
#include <FastEngine/object/C_objTextinputbox.hpp>
#include <FastEngine/object/C_objButton.hpp>
#include <FastEngine/C_packetLZ4.hpp>
#include <FastEngine/C_clock.hpp>

#include <C_creature.hpp>
#include <C_food.hpp>
#include <C_drink.hpp>

#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    if (!fge::net::Socket::initSocket())
    {
        std::cout << "can't init socket system !" << std::endl;
        return -1;
    }

    //Creating the window
    sf::RenderWindow screen(LIFESIM_VIDEOMODE, "Life simulator client, a FastEngine example by Guillaume Guillet - version "+std::to_string(LIFESIM_VERSION), sf::Style::Default);
    screen.setFramerateLimit(LIFESIM_FRAMERATE);

    fge::Event event;
    fge::GuiElementHandler guiElementHandler{event, screen};
    guiElementHandler.setEventCallback(event);

    //Creating the scene
    fge::Scene mainScene;

    mainScene.setLinkedRenderTarget(&screen);
    mainScene.setCallbackContext({&event, &guiElementHandler});

    //Creating the client side server
    fge::net::ServerClientSideUdp server;
    auto& serverSocket = server.getSocket();

    std::shared_ptr<fge::net::PacketLZ4> packetSend;

    //Texture
    fge::texture::Init();
    fge::texture::LoadFromFile("close", "resources/images/window/close.png");
    fge::texture::LoadFromFile("minimize", "resources/images/window/minimize.png");
    fge::texture::LoadFromFile("resize", "resources/images/window/resize.png");
    fge::texture::LoadFromFile("window", "resources/images/window/window.png");

    fge::texture::LoadFromFile("button_1", "resources/images/button_1.png");
    fge::texture::LoadFromFile("button_2", "resources/images/button_2.png");

    //Font
    fge::font::Init();
    fge::font::LoadFromFile("default", "resources/fonts/SourceSansPro-Regular.ttf");

    //Animation
    fge::anim::Init();
    fge::anim::LoadFromFile("ugandan", "resources/animations/ugandan_1/ugandan.json");

    //Audio
    fge::audio::Init();
    fge::audio::LoadFromFile("ugandan1", "resources/audio/ugandan1.ogg");
    fge::audio::LoadFromFile("ugandan2", "resources/audio/ugandan2.ogg");

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
            return -1;
        }
        std::cout << "OK !" << std::endl;
    }

    //Clock
    fge::Clock clockUpdate;
    fge::Clock deltaTime;

    //Create a latency text
    char latencyTextBuffer[200];
    const char* latencyTextFormat = "latency CTOS: %d, STOC: %d, ping: %d";
    auto* latencyText = mainScene.newObject(FGE_NEWOBJECT(fge::ObjText, latencyTextFormat, "default", {}),
                                            FGE_SCENE_PLAN_HIGH_TOP, FGE_SCENE_BAD_SID, fge::ObjectType::TYPE_GUI)->getObject<fge::ObjText>();
    latencyText->setFillColor(sf::Color::Black);

    fge::net::Client::Latency_ms latencySTOC = 0;
    fge::net::Client::Latency_ms ping = 0;

    bool connectionValid = false;
    bool connectionTimeoutCheck = false;
    fge::Clock connectionTimeout;

    //Lambda that create a GUI window for connection
    auto createConnectionWindow = [&](){
        //Creating window
        auto* window = mainScene.newObject(FGE_NEWOBJECT(fge::ObjWindow), FGE_SCENE_PLAN_HIGH_TOP, FGE_SCENE_BAD_SID, fge::ObjectType::TYPE_GUI)->getObject<fge::ObjWindow>();
        window->setTextureClose("close");
        window->setTextureMinimize("minimize");
        window->setTextureResize("resize");
        window->getTileSet().setTexture("window");
        window->move({100.0f, 100.0f});
        window->setSize(window->getSize() + sf::Vector2f{40.0f, 0.0});
        window->showExitButton(false);

        //Creating a text input box for the IP
        auto* textInputBoxIp = window->_windowScene.newObject(FGE_NEWOBJECT(fge::ObjTextInputBox, "default", 20))->getObject<fge::ObjTextInputBox>();
        textInputBoxIp->move({20.0f, 20.0f});
        textInputBoxIp->setMaxLength(15);
        textInputBoxIp->setString("127.0.0.1");

        //Creating a button for validation
        auto* buttonValid = window->_windowScene.newObject(FGE_NEWOBJECT(fge::ObjButton, "button_2", "button_1"))->getObject<fge::ObjButton>();
        buttonValid->move({20.0f, 100.0f});

        //Handle button pressing
        buttonValid->_onButtonPressed.add(new fge::CallbackLambda<fge::ObjButton*>{[&, textInputBoxIp]([[maybe_unused]] fge::ObjButton* button){
            if (connectionTimeoutCheck)
            {
                return;
            }

            fge::net::IpAddress ip{textInputBoxIp->getString().c_str()};

            if (serverSocket.bind(LIFESIM_CLIENT_PORT) != fge::net::Socket::Error::ERR_DONE)
            {
                std::cout << "can't bind to socket " << LIFESIM_CLIENT_PORT << " !" << std::endl;
                return;
            }

            if (serverSocket.connect(ip, LIFESIM_SERVER_PORT) != fge::net::Socket::Error::ERR_DONE)
            {
                serverSocket.close();
                std::cout << "can't connect the socket !" << std::endl;
                return;
            }

            //After the socket is bound and connected we start the server
            if (!server.start<fge::net::PacketLZ4>())
            {
                serverSocket.close();
                std::cout << "can't start the server !" << std::endl;
                return;
            }

            packetSend = std::make_shared<fge::net::PacketLZ4>();
            fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME) << LIFESIM_CONNECTION_TEXT1 << LIFESIM_CONNECTION_TEXT2;
            const std::size_t pckTimestampPos = packetSend->getDataSize();
            packetSend->append(sizeof(fge::net::Client::Timestamp));
            server._client.pushPacket({packetSend, fge::net::ClientSendQueuePacketOptions::QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP, pckTimestampPos});

            connectionTimeoutCheck = true;
            connectionTimeout.restart();
        }});
    };

    createConnectionWindow();

    while (screen.isOpen())
    {
        //Process events
        event.process(screen);
        if ( event.isEventType(sf::Event::Closed) )
        {
            screen.close();
        }

        //Send an update packet to the server
        if (clockUpdate.reached(LIFESIM_TIME_CLIENT_UPDATE) && connectionValid)
        {
            clockUpdate.restart();

            packetSend = std::make_shared<fge::net::PacketLZ4>();

            fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_C_UPDATE);

            //The packet is mostly composed of timestamp and latency information to limit bandwidth of packets
            *packetSend << fge::net::Client::getTimestamp_ms() << latencySTOC;
            server._client.pushPacket({packetSend, fge::net::ClientSendQueuePacketOptions::QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP,
                                       sizeof(fge::net::PacketHeader)});
        }

        //Check if the connection is unsuccessful
        if (connectionTimeoutCheck)
        {
            if (connectionTimeout.reached(LIFESIM_TIME_CONNECTION_TIMEOUT))
            {
                if (!connectionValid)
                {
                    std::cout << "no response (timeout) !" << std::endl;
                    server.stop();
                }
                connectionTimeoutCheck = false;
            }
        }

        //Handling server packets
        std::size_t pckSize = server.getPacketsSize();
        for (std::size_t i=0; i<pckSize; ++i)
        {
            //Popping the next packet
            auto fluxPacket = server.popNextPacket();

            //Prepare a sending packet
            packetSend = std::make_shared<fge::net::PacketLZ4>();

            //Retrieve the packet header
            switch ( fge::net::GetHeader(fluxPacket->_pck) )
            {
            case ls::LS_PROTOCOL_ALL_GOODBYE:
                server.stop();
                pckSize = 0;
                mainScene.delAllObject(true);
                connectionValid = false;
                createConnectionWindow();
                break;
            case ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME:
                {
                    bool valid = false;
                    fluxPacket->_pck >> valid;

                    fge::net::Client::Timestamp SyncTimestampClientStart;
                    fge::net::Client::Timestamp SyncTimestampServerStart;
                    fge::net::Client::Timestamp SyncTimestampServerStop;

                    fge::net::Client::Timestamp SyncTimestampClientStop = fluxPacket->_timestamp;

                    fluxPacket->_pck >> SyncTimestampClientStart >> SyncTimestampServerStart >> SyncTimestampServerStop;

                    if (fluxPacket->_pck && valid)
                    {
                        auto extraDelay = (fge::net::Client::computeLatency_ms(SyncTimestampClientStart, SyncTimestampClientStop) -
                                fge::net::Client::computeLatency_ms(SyncTimestampServerStart, SyncTimestampServerStop))/2;
                        SyncTimestampServerStop += extraDelay;
                        int32_t offset = SyncTimestampServerStop-SyncTimestampClientStop;
                        server._client.setSyncOffset(offset);

                        //We are connected, we can destroy the window
                        auto windowObject = mainScene.getFirstObj_ByClass(FGE_OBJWINDOW_CLASSNAME);
                        if (windowObject)
                        {
                            mainScene.delObject(windowObject->getSid());
                        }
                        connectionValid = true;
                        clockUpdate.restart();
                    }
                    else
                    {
                        server.stop();
                        std::cout << "server refused the connection or invalid packet !" << std::endl;
                    }
                }
                break;
            case ls::LS_PROTOCOL_S_UPDATE:
                {
                    //Retrieving "Client To Server" latency
                    //and the "Server To Client" timestamp
                    fge::net::Client::Latency_ms latencyCTOS;
                    fge::net::Client::Timestamp timestampServer;
                    fluxPacket->_pck >> timestampServer >> latencyCTOS;

                    //We can compute the "Server To Client" latency with the provided server timestamp and the packet timestamp
                    latencySTOC = fge::net::Client::computeLatency_ms(server._client.syncServerTimestampToClient(timestampServer), fluxPacket->_timestamp);
                    server._client.setLatency_ms(latencyCTOS);

                    //We can simply get the ping by summing the 2 latencies
                    ping = latencySTOC + latencyCTOS;

                    //Updating the latencyText
                    int size = sprintf(latencyTextBuffer, latencyTextFormat, latencyCTOS, latencySTOC, ping);
                    latencyText->setString(tiny_utf8::string(latencyTextBuffer, size));

                    //And then unpack all modification made by the server scene
                    mainScene.unpackModification(fluxPacket->_pck);
                    //And unpack all watched events
                    mainScene.unpackWatchedEvent(fluxPacket->_pck);
                }
                break;
            case ls::LS_PROTOCOL_S_UPDATE_ALL:
                //Do a full scene update
                mainScene.unpack(fluxPacket->_pck);
                break;
            default:
                break;
            }
        }

        //Update the scene
        mainScene.update(screen, event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime.restart()));

        //Draw the scene
        mainScene.draw(screen, true, sf::Color::White);
        screen.display();
    }

    fge::audio::Uninit();
    fge::font::Uninit();
    fge::anim::Uninit();
    fge::texture::Uninit();

    fge::net::Socket::uninitSocket();

    return 0;
}

