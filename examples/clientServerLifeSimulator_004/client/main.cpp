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

#include "FastEngine/C_clock.hpp"
#include "FastEngine/graphic/C_renderWindow.hpp"
#include "FastEngine/manager/anim_manager.hpp"
#include "FastEngine/manager/audio_manager.hpp"
#include "FastEngine/manager/font_manager.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/manager/reg_manager.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/manager/timer_manager.hpp"
#include "FastEngine/network/C_packetLZ4.hpp"
#include "FastEngine/network/C_server.hpp"
#include "FastEngine/object/C_objButton.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_objTextinputbox.hpp"
#include "FastEngine/object/C_objWindow.hpp"
#include "SDL.h"
#include "definition.hpp"

#include "C_creature.hpp"
#include "C_drink.hpp"
#include "C_food.hpp"

#include <iostream>
#include <sstream>

#define MAX_BAD_PACKET 20

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    if (!fge::net::Socket::initSocket())
    {
        std::cout << "can't init socket system !" << std::endl;
        return -1;
    }

    using namespace fge::vulkan;

    auto instance =
            Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 004: life simulator client", LIFESIM_VERSION);
    Context::enumerateExtensions();

    SurfaceSDLWindow window(instance, FGE_WINDOWPOS_CENTERED, {LIFESIM_MAP_WIDTH, LIFESIM_MAP_HEIGHT},
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    // Check that the window was successfully created
    if (!window.isCreated())
    {
        // In the case that the window could not be made...
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    Context vulkanContext(window);
    vulkanContext._garbageCollector.enable(true);

    fge::shader::gManager.initialize();
    fge::shader::gManager.loadFromFile(
            FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX, "resources/shaders/objShapeInstances_vertex.vert",
            fge::vulkan::Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::gManager.loadFromFile(
            FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT, "resources/shaders/objSpriteBatches_fragment.frag",
            fge::vulkan::Shader::Type::SHADER_FRAGMENT, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::gManager.loadFromFile(
            FGE_OBJSPRITEBATCHES_SHADER_VERTEX, "resources/shaders/objSpriteBatches_vertex.vert",
            fge::vulkan::Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);

    fge::RenderWindow renderWindow(vulkanContext, window);
    renderWindow.setClearColor(fge::Color::White);

    fge::Event event;
    fge::GuiElementHandler guiElementHandler{event, renderWindow};
    guiElementHandler.setEventCallback();

    //Creating the scene
    std::unique_ptr<fge::Scene> mainScene = std::make_unique<fge::Scene>();

    mainScene->setLinkedRenderTarget(&renderWindow);
    mainScene->setCallbackContext({&event, &guiElementHandler});

    //Creating the client side server
    //Here you can swap for ipv6 if you want
    fge::net::ClientSideNetUdp server(fge::net::IpAddress::Types::Ipv4);

    //Texture
    fge::texture::gManager.initialize();
    fge::texture::gManager.loadFromFile("close", "resources/images/window/close.png");
    fge::texture::gManager.loadFromFile("minimize", "resources/images/window/minimize.png");
    fge::texture::gManager.loadFromFile("resize", "resources/images/window/resize.png");
    fge::texture::gManager.loadFromFile("window", "resources/images/window/window.png");

    fge::texture::gManager.loadFromFile("button_1", "resources/images/button_1.png");
    fge::texture::gManager.loadFromFile("button_2", "resources/images/button_2.png");

    //Font
    fge::font::gManager.initialize();
    fge::font::gManager.loadFromFile("default", "resources/fonts/SourceSansPro-Regular.ttf");

    //Animation
    fge::anim::gManager.initialize();
    fge::anim::gManager.loadFromFile("ugandan", "resources/animations/ugandan_1/ugandan.json");

    //Audio
    fge::audio::gManager.initialize();
    fge::audio::gManager.loadFromFile("ugandan1", "resources/audio/ugandan1.ogg");
    fge::audio::gManager.loadFromFile("ugandan2", "resources/audio/ugandan2.ogg");

    //Timer
    fge::timer::Init();
    //Prepare a timer for timeout
    fge::timer::TimerShared timerTimeout =
            std::make_shared<fge::Timer>(LIFESIM_TIME_CONNECTION_TIMEOUT, "timeout", true);
    fge::timer::Create(timerTimeout);
    fge::timer::TimerShared timerConnectionTimeout =
            std::make_shared<fge::Timer>(LIFESIM_TIME_CONNECTION_TIMEOUT, "connectionTimeout", true);
    fge::timer::Create(timerConnectionTimeout);

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
            return -1;
        }
        std::cout << "OK !" << std::endl;
    }

    //Clock
    fge::Clock clockUpdate;
    fge::Clock deltaTime;

    //Create a latency text
    std::ostringstream latencyTextStream;

    auto* latencyText =
            mainScene->newObject<fge::ObjText>({FGE_SCENE_PLAN_HIGH_TOP, FGE_SCENE_BAD_SID, fge::ObjectType::TYPE_GUI},
                                               "waiting for server", "default", fge::Vector2f{}, 15);
    latencyText->setFillColor(fge::Color::Black);

    bool connectionValid = false;

    //Lambda that create a GUI window for connection
    auto createConnectionWindow = [&]() {
        //First check if the window already exists
        if (mainScene->getFirstObj_ByClass(FGE_OBJWINDOW_CLASSNAME))
        {
            return;
        }

        //Creating window
        auto* window = mainScene->newObject<fge::ObjWindow>(
                {FGE_SCENE_PLAN_HIGH_TOP, FGE_SCENE_BAD_SID, fge::ObjectType::TYPE_GUI});
        window->setTextureClose("close");
        window->setTextureMinimize("minimize");
        window->setTextureResize("resize");
        window->setTexture("window");
        window->move({100.0f, 100.0f});
        window->setSize(window->getSize() + fge::Vector2f{40.0f, 0.0});
        window->showExitButton(false);

        //Creating a text input box for the IP
        auto* textInputBoxIp = window->_windowScene.newObject<fge::ObjTextInputBox>("default", 20);
        textInputBoxIp->move({20.0f, 20.0f});
        textInputBoxIp->setMaxLength(15);
        textInputBoxIp->setString("127.0.0.1");

        //Creating a button for validation
        auto* buttonValid = window->_windowScene.newObject<fge::ObjButton>("button_2", "button_1");
        buttonValid->move({20.0f, 100.0f});

        //Handle button pressing
        buttonValid->_onButtonPressed.addLambda([&, textInputBoxIp]([[maybe_unused]] fge::ObjButton* button) {
            if (!timerConnectionTimeout->isPaused())
            { //Timer is running, that means we are already trying to connect
                return;
            }

            fge::net::IpAddress remoteIp{textInputBoxIp->getString().c_str()};

            //We try to connect to the server
            if (!server.start<fge::net::PacketLZ4>(LIFESIM_CLIENT_PORT,
                                                   fge::net::IpAddress::Any(server.getAddressType()),
                                                   LIFESIM_SERVER_PORT, remoteIp))
            {
                std::cout << "can't connect the server !" << std::endl;
                return;
            }

            auto transmissionPacket = fge::net::TransmissionPacket::create(ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME);
            transmissionPacket->packet() << LIFESIM_CONNECTION_TEXT1 << LIFESIM_CONNECTION_TEXT2;

            //Ask the server thread to automatically update the timestamp just before sending it
            server._client._latencyPlanner.pack(transmissionPacket);

            server._client.pushPacket(std::move(transmissionPacket));

            timerConnectionTimeout->restart();
            timerConnectionTimeout->resume();
        });
    };

    createConnectionWindow();

    server._client._onThresholdLostPacket.addLambda([&]([[maybe_unused]] fge::net::Client& client) {
        //Here we consider that the scene is lost and we ask the server a full update
        auto transmissionPacket = fge::net::TransmissionPacket::create(ls::LS_PROTOCOL_C_ASK_FULL_UPDATE);
        transmissionPacket->doNotDiscard();
        server._client.pushPacket(std::move(transmissionPacket));
        server.notifyTransmission();
    });

    //Begin loop
    bool running = true;
    while (running)
    {
        //Process events
        event.process();
        if (event.isEventType(SDL_QUIT))
        {
            running = false;
        }

        //Send an update packet to the server
        if (clockUpdate.reached(LIFESIM_TIME_CLIENT_UPDATE) && connectionValid)
        {
            clockUpdate.restart();

            auto transmissionPacket = fge::net::TransmissionPacket::create(ls::LS_PROTOCOL_C_UPDATE);

            //The packet is mostly composed of timestamp and latency information to limit bandwidth of packets.
            //The LatencyPlanner class will do all the work for that.
            server._client._latencyPlanner.pack(transmissionPacket);
            server._client.pushPacket(std::move(transmissionPacket));
        }

        //Check if the connection is unsuccessful
        if (timerConnectionTimeout->goalReached())
        {
            std::cout << "no response (timeout) !" << std::endl;

            connectionValid = false;

            server.stop();
            mainScene->delAllObject(true);
            createConnectionWindow();

            timerConnectionTimeout->pause();
            timerConnectionTimeout->restart();
            fge::timer::Create(timerConnectionTimeout); ///TODO: timer system need a refactor
        }

        //Check if the connection is lost
        if (timerTimeout->goalReached())
        {
            std::cout << "connection lost !" << std::endl;

            connectionValid = false;

            server.stop();
            mainScene->delAllObject(true);
            createConnectionWindow();

            timerTimeout->pause();
            timerTimeout->restart();
            fge::timer::Create(timerTimeout); ///TODO: timer system need a refactor
        }

        //Handling server packets
        fge::net::ProtocolPacketPtr packet;
        while (server.process(packet) == fge::net::FluxProcessResults::RETRIEVABLE)
        {
            //Prepare a sending packet
            auto transmissionPacket = fge::net::TransmissionPacket::create();

            //Retrieve the packet header
            switch (packet->retrieveHeaderId().value())
            {
            case ls::LS_PROTOCOL_ALL_GOODBYE:
                std::cout << "goodbye from server !" << std::endl;

                server.stop();
                mainScene->delAllObject(true);
                createConnectionWindow();

                timerConnectionTimeout->pause();
                timerConnectionTimeout->restart();

                timerTimeout->pause();
                timerTimeout->restart();
                break;
            case ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME:
            {
                bool valid = false;
                *packet >> valid;

                if (packet->isValid() && valid)
                {
                    //Get latency
                    server._client._latencyPlanner.unpack(packet.get(), server._client);
                    if (auto latency = server._client._latencyPlanner.getLatency())
                    {
                        server._client.setCTOSLatency_ms(latency.value());
                    }
                    if (auto latency = server._client._latencyPlanner.getOtherSideLatency())
                    {
                        server._client.setSTOCLatency_ms(latency.value());
                    }

                    //We are connected, we can destroy the window
                    auto windowObject = mainScene->getFirstObj_ByClass(FGE_OBJWINDOW_CLASSNAME);
                    if (windowObject)
                    {
                        mainScene->delObject(windowObject->getSid());
                    }
                    connectionValid = true;
                    clockUpdate.restart();

                    timerConnectionTimeout->pause();
                    timerConnectionTimeout->restart();

                    timerTimeout->restart();
                    timerTimeout->resume();

                    std::cout << "connected to server !" << std::endl;
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
                timerTimeout->restart();

                //Get latency
                server._client._latencyPlanner.unpack(packet.get(), server._client);
                if (auto latency = server._client._latencyPlanner.getLatency())
                {
                    server._client.setCTOSLatency_ms(latency.value());
                }
                if (auto latency = server._client._latencyPlanner.getOtherSideLatency())
                {
                    server._client.setSTOCLatency_ms(latency.value());
                }

                //Updating the latencyText
                latencyTextStream.str(std::string{});
                latencyTextStream << "clock offset: "
                                  << fge::string::ToStr(server._client._latencyPlanner.getClockOffset()) << '\n'
                                  << "latency CTOS: " << server._client.getCTOSLatency_ms() << '\n'
                                  << "latency STOC: " << server._client.getSTOCLatency_ms() << '\n'
                                  << "ping: " << server._client.getPing_ms() << '\n'
                                  << "RTT: " << fge::string::ToStr(server._client._latencyPlanner.getRoundTripTime())
                                  << '\n'
                                  << "Update count: " << mainScene->getUpdateCount() << '\n'
                                  << "Lost packets: " << server._client.getLostPacketCount() << '\n'
                                  << "Realm: " << static_cast<unsigned int>(server._client.getCurrentRealm())
                                  << ", CurrentCounter: " << server._client.getCurrentPacketCounter()
                                  << ", ClientCounter: " << server._client.getClientPacketCounter();

                latencyText->setString(tiny_utf8::string(latencyTextStream.str()));

                //And then unpack all modification made by the server scene
                fge::Scene::UpdateCountRange updateRange{};
                auto err = mainScene->unpackModification(*packet, updateRange);
                if (err)
                {
                    server._client.advanceLostPacketCount();

                    err->dump(std::cout);
                    std::cout << "\tclient[" << mainScene->getUpdateCount() << "] "
                              << "server[" << updateRange._last << " -> " << updateRange._now << "]\n"
                              << std::endl;
                }
                else
                {
                    //And unpack all watched events
                    err = mainScene->unpackWatchedEvent(*packet);
                    if (err)
                    {
                        server._client.advanceLostPacketCount();
                    }
                }
            }
            break;
            case ls::LS_PROTOCOL_S_UPDATE_ALL:
                //Do a full scene update
                mainScene->unpack(*packet);

                std::cout << "received full scene update [" << mainScene->getUpdateCount() << "]" << std::endl;
                break;
            default:
                break;
            }
        }

        //Update scene
        mainScene->update(renderWindow, event,
                          std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime.restart()));

        //Drawing
        auto imageIndex = renderWindow.prepareNextFrame(nullptr, FGE_RENDER_TIMEOUT_BLOCKING);
        if (imageIndex != FGE_RENDER_BAD_IMAGE_INDEX)
        {
            fge::vulkan::GetActiveContext()._garbageCollector.setCurrentFrame(renderWindow.getCurrentFrame());

            renderWindow.beginRenderPass(imageIndex);

            mainScene->draw(renderWindow);

            renderWindow.endRenderPass();

            renderWindow.display(imageIndex);
        }
    }

    fge::vulkan::GetActiveContext().waitIdle();

    fge::vulkan::GetActiveContext()._garbageCollector.enable(false);

    mainScene.reset();

    fge::timer::Uninit();
    fge::audio::gManager.uninitialize();
    fge::shader::gManager.uninitialize();
    fge::font::gManager.uninitialize();
    fge::anim::gManager.uninitialize();
    fge::texture::gManager.uninitialize();

    renderWindow.destroy();

    vulkanContext.destroy();

    window.destroy();
    instance.destroy();
    fge::net::Socket::uninitSocket();
    SDL_Quit();

    return 0;
}
