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
#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/graphic/C_renderWindow.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objSprite.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "SDL.h"
#include <iostream>

//Create the MainScene class
class MainScene : public fge::Scene
{
public:
    void start(fge::RenderWindow& renderWindow)
    {
        fge::Event event(renderWindow);
        fge::GuiElementHandler guiElementHandler(event, renderWindow);
        guiElementHandler.setEventCallback();

        this->setCallbackContext({&event, &guiElementHandler});

        //Init texture manager
        fge::texture::gManager.initialize();
        //Init font manager
        fge::font::gManager.initialize();

        //Load texture
        fge::texture::gManager.loadFromFile("texture", "resources/textures/texture.jpg");
        auto textureData = fge::texture::gManager.getElement("texture");
        textureData->_ptr->generateMipmaps(FGE_TEXTURE_IMAGE_MIPMAPS_LEVELS_AUTO);

        std::cout << "Mipmap levels : " << textureData->_ptr->getMipLevels() << std::endl;
        float mipMinLod = 0.0f;

        //Load font
        fge::font::gManager.loadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto* explainText = this->newObject<fge::ObjText>({FGE_SCENE_PLAN_HIGH_TOP},
                                                          "Use WASD/Arrow keys to move the view around\n"
                                                          "Use Q/E to increase/decrease the mipmap min value\n"
                                                          "Use the mouse wheel to zoom in and out",
                                                          "base", fge::Vector2f{}, 18);
        explainText->setFillColor(fge::Color::Black);

        //Create a select box in order to switch between test objects
        auto* sprite = this->newObject<fge::ObjSprite>({FGE_SCENE_PLAN_HIGH_TOP}, "texture");
        sprite->setOrigin(static_cast<fge::Vector2f>(sprite->getTexture().getTextureSize()) / 2.0f);
        sprite->move({400.0f, 300.0f});

        //Create event callback for moving the view
        event._onKeyDown.addLambda([&](fge::Event const&, SDL_KeyboardEvent const& keyEvent) {
            auto view = renderWindow.getView();
            if (keyEvent.keysym.sym == SDLK_LEFT || keyEvent.keysym.sym == SDLK_a)
            {
                view.move({-10, 0});
            }
            else if (keyEvent.keysym.sym == SDLK_RIGHT || keyEvent.keysym.sym == SDLK_d)
            {
                view.move({10, 0});
            }
            else if (keyEvent.keysym.sym == SDLK_UP || keyEvent.keysym.sym == SDLK_w)
            {
                view.move({0, -10});
            }
            else if (keyEvent.keysym.sym == SDLK_DOWN || keyEvent.keysym.sym == SDLK_s)
            {
                view.move({0, 10});
            }
            else if (keyEvent.keysym.sym == SDLK_q)
            {
                mipMinLod -= 1.0f;
                if (mipMinLod < 0.0f)
                {
                    mipMinLod = 0.0f;
                }
                textureData->_ptr->forceMipLod(0.0f, mipMinLod, static_cast<float>(textureData->_ptr->getMipLevels()));
                std::cout << "Mipmap min lod : " << mipMinLod << std::endl;
            }
            else if (keyEvent.keysym.sym == SDLK_e)
            {
                mipMinLod += 1.0f;
                if (mipMinLod > static_cast<float>(textureData->_ptr->getMipLevels()))
                {
                    mipMinLod = static_cast<float>(textureData->_ptr->getMipLevels());
                }
                textureData->_ptr->forceMipLod(0.0f, mipMinLod, static_cast<float>(textureData->_ptr->getMipLevels()));
                std::cout << "Mipmap min lod : " << mipMinLod << std::endl;
            }
            renderWindow.setView(view);
        });

        //Create event callback for zooming the view
        event._onMouseWheel.addLambda([&](fge::Event const&, SDL_MouseWheelEvent const& mouseWheelEvent) {
            auto view = renderWindow.getView();
            if (mouseWheelEvent.y > 0)
            {
                view.zoom(0.9f);
            }
            else
            {
                view.zoom(1.1f);
            }
            renderWindow.setView(view);
        });

        //Begin loop
        bool running = true;
        while (running)
        {
            //Update event
            event.process();
            if (event.isEventType(SDL_QUIT))
            {
                running = false;
            }

            //Update scene
            auto deltaTick = tick.restart();
            this->update(renderWindow, event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTick));

            //Drawing
            auto imageIndex = renderWindow.prepareNextFrame(nullptr, FGE_RENDER_TIMEOUT_BLOCKING);
            if (imageIndex != FGE_RENDER_BAD_IMAGE_INDEX)
            {
                fge::vulkan::GetActiveContext()._garbageCollector.setCurrentFrame(renderWindow.getCurrentFrame());

                renderWindow.beginRenderPass(imageIndex);

                this->draw(renderWindow);

                renderWindow.endRenderPass();

                renderWindow.display(imageIndex);
            }
        }

        fge::vulkan::GetActiveContext().waitIdle();

        fge::vulkan::GetActiveContext()._garbageCollector.enable(false);
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    using namespace fge::vulkan;

    auto instance = Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 007: mipmaps");
    Context::enumerateExtensions();

    SurfaceSDLWindow window(instance, FGE_WINDOWPOS_CENTERED, {800, 600}, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

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

    fge::RenderWindow renderWindow(vulkanContext, window);
    renderWindow.setClearColor(fge::Color::White);
    renderWindow.setPresentMode(VK_PRESENT_MODE_FIFO_KHR);

    std::unique_ptr<MainScene> scene = std::make_unique<MainScene>();
    scene->start(renderWindow);
    scene.reset();

    fge::texture::gManager.uninitialize();
    fge::font::gManager.uninitialize();
    fge::shader::gManager.uninitialize();

    renderWindow.destroy();

    vulkanContext.destroy();

    window.destroy();
    instance.destroy();
    SDL_Quit();

    return 0;
}