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

#include "FastEngine/C_clock.hpp"
#include "FastEngine/C_packetLZ4.hpp"
#include "FastEngine/C_server.hpp"
#include "FastEngine/manager/anim_manager.hpp"
#include "FastEngine/manager/audio_manager.hpp"
#include "FastEngine/manager/font_manager.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include "FastEngine/manager/reg_manager.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
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

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    if (!fge::net::Socket::initSocket())
    {
        std::cout << "can't init socket system !" << std::endl;
        return -1;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window =
            SDL_CreateWindow(("Life simulator client, a FastEngine example by Guillaume Guillet - version " +
                              std::to_string(LIFESIM_VERSION))
                                     .c_str(),
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LIFESIM_MAP_WIDTH, LIFESIM_MAP_HEIGHT,
                             SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    // Check that the window was successfully created
    if (window == nullptr)
    {
        // In the case that the window could not be made...
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    fge::vulkan::Context vulkanContext{};
    fge::vulkan::Context::initVolk();
    fge::vulkan::Context::enumerateExtensions();
    vulkanContext.initVulkan(window);

    fge::vulkan::SetActiveContext(vulkanContext);

    vulkanContext._garbageCollector.enable(true);

    fge::shader::Init();
    fge::shader::LoadFromFile(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX, "resources/shaders/objShapeInstances_vertex.vert",
                              fge::vulkan::Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::LoadFromFile(FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT, "resources/shaders/objSpriteBatches_fragment.frag",
                              fge::vulkan::Shader::Type::SHADER_FRAGMENT, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::LoadFromFile(FGE_OBJSPRITEBATCHES_SHADER_VERTEX, "resources/shaders/objSpriteBatches_vertex.vert",
                              fge::vulkan::Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);

    fge::RenderWindow renderWindow(vulkanContext);
    renderWindow.setClearColor(fge::Color::White);

    fge::Event event;
    fge::GuiElementHandler guiElementHandler{event, renderWindow};
    guiElementHandler.setEventCallback(event);

    //Creating the scene
    std::unique_ptr<fge::Scene> mainScene = std::make_unique<fge::Scene>();

    mainScene->setLinkedRenderTarget(&renderWindow);
    mainScene->setCallbackContext({&event, &guiElementHandler});

    //Creating the client side server
    fge::net::ClientSideNetUdp server;

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
        valid &= fge::reg::RegisterNewClass(std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Creature>()));
        valid &= fge::reg::RegisterNewClass(std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Food>()));
        valid &= fge::reg::RegisterNewClass(std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<ls::Drink>()));
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
    char const* latencyTextFormat = "clock offset: %s, latency CTOS: %d, latency STOC: %d, ping: %d";
    auto* latencyText = mainScene
                                ->newObject(FGE_NEWOBJECT(fge::ObjText, latencyTextFormat, "default", {}),
                                            FGE_SCENE_PLAN_HIGH_TOP, FGE_SCENE_BAD_SID, fge::ObjectType::TYPE_GUI)
                                ->getObject<fge::ObjText>();
    latencyText->setFillColor(fge::Color::Black);

    bool connectionValid = false;
    bool connectionTimeoutCheck = false;
    fge::Clock connectionTimeout;

    //Lambda that create a GUI window for connection
    auto createConnectionWindow = [&]() {
        //Creating window
        auto* window = mainScene
                               ->newObject(FGE_NEWOBJECT(fge::ObjWindow), FGE_SCENE_PLAN_HIGH_TOP, FGE_SCENE_BAD_SID,
                                           fge::ObjectType::TYPE_GUI)
                               ->getObject<fge::ObjWindow>();
        window->setTextureClose("close");
        window->setTextureMinimize("minimize");
        window->setTextureResize("resize");
        window->setTexture("window");
        window->move({100.0f, 100.0f});
        window->setSize(window->getSize() + fge::Vector2f{40.0f, 0.0});
        window->showExitButton(false);

        //Creating a text input box for the IP
        auto* textInputBoxIp = window->_windowScene.newObject(FGE_NEWOBJECT(fge::ObjTextInputBox, "default", 20))
                                       ->getObject<fge::ObjTextInputBox>();
        textInputBoxIp->move({20.0f, 20.0f});
        textInputBoxIp->setMaxLength(15);
        textInputBoxIp->setString("127.0.0.1");

        //Creating a button for validation
        auto* buttonValid = window->_windowScene.newObject(FGE_NEWOBJECT(fge::ObjButton, "button_2", "button_1"))
                                    ->getObject<fge::ObjButton>();
        buttonValid->move({20.0f, 100.0f});

        //Handle button pressing
        buttonValid->_onButtonPressed.add(
                new fge::CallbackLambda<fge::ObjButton*>{[&, textInputBoxIp]([[maybe_unused]] fge::ObjButton* button) {
            if (connectionTimeoutCheck)
            {
                return;
            }

            fge::net::IpAddress remoteIp{textInputBoxIp->getString().c_str()};

            //We try to connect to the server
            if (!server.start<fge::net::PacketLZ4>(LIFESIM_CLIENT_PORT, fge::net::IpAddress::Any, LIFESIM_SERVER_PORT,
                                                   remoteIp))
            {
                std::cout << "can't connect the server !" << std::endl;
                return;
            }

            auto transmissionPacket = fge::net::TransmissionPacket::create<fge::net::PacketLZ4>();
            fge::net::SetHeader(transmissionPacket->packet(), ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME)
                    << LIFESIM_CONNECTION_TEXT1 << LIFESIM_CONNECTION_TEXT2;

            //Ask the server thread to automatically update the timestamp just before sending it
            server._client._latencyPlanner.pack(transmissionPacket);

            server._client.pushPacket(std::move(transmissionPacket));

            connectionTimeoutCheck = true;
            connectionTimeout.restart();
        }});
    };

    createConnectionWindow();

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

            auto transmissionPacket = fge::net::TransmissionPacket::create<fge::net::PacketLZ4>();

            fge::net::SetHeader(transmissionPacket->packet(), ls::LS_PROTOCOL_C_UPDATE);

            //The packet is mostly composed of timestamp and latency information to limit bandwidth of packets.
            //The LatencyPlanner class will do all the work for that.
            server._client._latencyPlanner.pack(transmissionPacket);
            server._client.pushPacket(std::move(transmissionPacket));
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
        for (std::size_t i = 0; i < pckSize; ++i)
        {
            //Popping the next packet
            auto fluxPacket = server.popNextPacket();

            //Prepare a sending packet (not used in this example)
            //auto transmissionPacket = fge::net::TransmissionPacket::create<fge::net::PacketLZ4>();

            //Retrieve the packet header
            switch (fge::net::GetHeader(fluxPacket->_packet))
            {
            case ls::LS_PROTOCOL_ALL_GOODBYE:
                server.stop();
                pckSize = 0;
                mainScene->delAllObject(true);
                connectionValid = false;
                createConnectionWindow();
                break;
            case ls::LS_PROTOCOL_C_PLEASE_CONNECT_ME:
            {
                bool valid = false;
                fluxPacket->_packet >> valid;

                if (fluxPacket->_packet && valid)
                {
                    //Get latency
                    server._client._latencyPlanner.unpack(fluxPacket.get(), server._client);
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
                //Get latency
                server._client._latencyPlanner.unpack(fluxPacket.get(), server._client);
                if (auto latency = server._client._latencyPlanner.getLatency())
                {
                    server._client.setCTOSLatency_ms(latency.value());
                }
                if (auto latency = server._client._latencyPlanner.getOtherSideLatency())
                {
                    server._client.setSTOCLatency_ms(latency.value());
                }

                //Updating the latencyText
                int size = sprintf(latencyTextBuffer, latencyTextFormat,
                                   fge::string::ToStr(server._client._latencyPlanner.getClockOffset()).c_str(),
                                   server._client.getCTOSLatency_ms(), server._client.getSTOCLatency_ms(),
                                   server._client.getPing_ms());
                latencyText->setString(tiny_utf8::string(latencyTextBuffer, size));

                //And then unpack all modification made by the server scene
                mainScene->unpackModification(fluxPacket->_packet);
                //And unpack all watched events
                mainScene->unpackWatchedEvent(fluxPacket->_packet);
            }
            break;
            case ls::LS_PROTOCOL_S_UPDATE_ALL:
                //Do a full scene update
                mainScene->unpack(fluxPacket->_packet);
                break;
            default:
                break;
            }
        }

        //Update scene
        mainScene->update(renderWindow, event,
                          std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime.restart()));

        //Drawing
        auto imageIndex = renderWindow.prepareNextFrame(nullptr);
        if (imageIndex != FGE_RENDERTARGET_BAD_IMAGE_INDEX)
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

    fge::audio::Uninit();
    fge::shader::Uninit();
    fge::font::Uninit();
    fge::anim::Uninit();
    fge::texture::Uninit();

    renderWindow.destroy();

    vulkanContext.destroy();

    SDL_DestroyWindow(window);

    fge::net::Socket::uninitSocket();

    SDL_Quit();

    return 0;
}
