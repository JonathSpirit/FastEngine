#include <definition.hpp>
#include <FastEngine/C_server.hpp>
#include <FastEngine/manager/network_manager.hpp>
#include <FastEngine/manager/reg_manager.hpp>
#include <FastEngine/manager/texture_manager.hpp>
#include <FastEngine/manager/font_manager.hpp>
#include <FastEngine/manager/anim_manager.hpp>
#include <FastEngine/manager/audio_manager.hpp>
#include <FastEngine/object/C_objText.hpp>
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
        std::cout << "Can't init socket system !" << std::endl;
        return -1;
    }

    sf::RenderWindow screen(sf::VideoMode(1600, 900), "Life simulator client, a FastEngine example by Guillaume Guillet - version "+std::to_string(LIFESIM_VERSION), sf::Style::Default);
    screen.setFramerateLimit(60);

    fge::Scene mainScene;

    fge::net::ServerClientSideUdp server;
    auto& serverSocket = server.getSocket();
    std::shared_ptr<fge::net::PacketLZ4> packetSend;

    if (serverSocket.bind(LIFESIM_CLIENT_PORT) != fge::net::Socket::Error::ERR_DONE)
    {
        std::cout << "Can't bind to socket " << LIFESIM_CLIENT_PORT << " !" << std::endl;
        return -1;
    }

    if (serverSocket.connect(fge::net::IpAddress::LocalHost, LIFESIM_SERVER_PORT) != fge::net::Socket::Error::ERR_DONE)
    {
        std::cout << "Can't connect the socket !" << std::endl;
        return -1;
    }

    if (!server.start<fge::net::PacketLZ4>())
    {
        std::cout << "Can't start the server !" << std::endl;
        return -1;
    }

    packetSend = std::make_shared<fge::net::PacketLZ4>();
    fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME) << "Hello" << LIFESIM_CONNECTION_TEXT;
    server._client.pushPacket({packetSend});
    if (server.waitForPackets(std::chrono::milliseconds{3000}))
    {
        auto fluxPacket = server.popNextPacket();

        if ( fge::net::GetHeader(fluxPacket->_pck) == ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME )
        {
            bool tmpValid = false;
            fluxPacket->_pck >> tmpValid;
            if (!tmpValid)
            {
                server.stop();
                std::cout << "Server refused the connection !" << std::endl;
                //return -1;
            }
        }
        else
        {
            std::cout << "Bad protocol response from server !" << std::endl;
            server.stop();
            //return -1;
        }
    }
    else
    {
        std::cout << "No response (timeout) !" << std::endl;
        server.stop();
        //return -1;
    }

    ///TEXTURE
    fge::texture::Init();

    ///FONT
    fge::font::Init();
    fge::font::LoadFromFile("default", "resources/fonts/SourceSansPro-Regular.ttf");

    ///ANIM
    fge::anim::Init();
    fge::anim::LoadFromFile("ugandan", "resources/animations/ugandan_1/ugandan.json");

    ///AUDIO
    fge::audio::Init();
    fge::audio::LoadFromFile("ugandan1", "resources/audio/ugandan1.ogg");
    fge::audio::LoadFromFile("ugandan2", "resources/audio/ugandan2.ogg");

    ///REG
    std::cout << fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Creature>()) ) << std::endl;
    std::cout << fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Food>()) ) << std::endl;
    std::cout << fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Drink>()) ) << std::endl;

    ///CLOCK
    fge::Clock clockPing;
    fge::Clock deltaTime;

    ///Scene
    char latencyTextBuffer[200];
    const char* latencyTextFormat = "latency CTOS: %d, STOC: %d, ping: %d";
    auto* latencyText = mainScene.newObject(FGE_NEWOBJECT(fge::ObjText, latencyTextFormat, "default", {}),
                                            FGE_SCENE_PLAN_HIGH_TOP, FGE_SCENE_BAD_SID, fge::ObjectType::TYPE_GUI)->getObject<fge::ObjText>();
    latencyText->setFillColor(sf::Color::Black);

    fge::net::Client::Latency_ms latencySTOC;
    fge::net::Client::Latency_ms ping;

    fge::Event event;
    while (screen.isOpen())
    {
        ///Event
        event.process(screen);
        if ( event.isEventType(sf::Event::Closed) )
        {
            screen.close();
        }

        ///Timer
        if (clockPing.reached(std::chrono::milliseconds{1000}))
        {
            packetSend = std::make_shared<fge::net::PacketLZ4>();
            fge::net::SetHeader(*packetSend, ls::LS_PROTOCOL_C_UPDATE);
            *packetSend << fge::net::Client::getTimestamp_ms() << latencySTOC;
            server._client.pushPacket({packetSend, fge::net::ClientSendQueuePacketOptions::QUEUE_PACKET_OPTION_UPDATE_TIMESTAMP,
                                       sizeof(fge::net::PacketHeader)});
        }

        std::size_t pckSize = server.getPacketsSize();
        for (std::size_t i=0; i<pckSize; ++i)
        {
            auto fluxPacket = server.popNextPacket();
            packetSend = std::make_shared<fge::net::PacketLZ4>();

            switch ( fge::net::GetHeader(fluxPacket->_pck) )
            {
            case ls::LS_PROTOCOL_ALL_GOODBYE:
                server.stop();
                screen.close();
                break;
            case ls::LS_PROTOCOL_S_UPDATE:
                {
                    fge::net::Client::Latency_ms latencyCTOS;
                    fge::net::Client::Timestamp timestampSTOC;

                    fluxPacket->_pck >> timestampSTOC >> latencyCTOS;
                    latencySTOC = fge::net::Client::computeLatency_ms(timestampSTOC, fluxPacket->_timestamp);
                    server._client.setLatency_ms(latencyCTOS);
                    ping = latencySTOC + latencyCTOS;

                    int size = sprintf(latencyTextBuffer, latencyTextFormat, latencyCTOS, latencySTOC, ping);

                    latencyText->setString(tiny_utf8::string(latencyTextBuffer, size));

                    mainScene.unpackModification(fluxPacket->_pck);
                    mainScene.unpackWatchedEvent(fluxPacket->_pck);
                }
                break;
            case ls::LS_PROTOCOL_S_UPDATE_ALL:
                mainScene.unpack(fluxPacket->_pck);
                break;
            default:
                break;
            }
        }

        mainScene.update(screen, event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime.restart()));

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

