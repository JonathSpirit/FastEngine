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
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/extra/extra_pathFinding.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objSelectBox.hpp"
#include "FastEngine/object/C_objSprite.hpp"
#include "FastEngine/object/C_objSpriteBatches.hpp"
#include "FastEngine/object/C_objSpriteCluster.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "SDL.h"
#include <iostream>

#define MAP_SIZE_W 200
#define MAP_SIZE_H 200
#define MULTISPRITES_OBJECT_TAG "multiSprites"

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
        fge::texture::gManager.loadFromFile("grid", "resources/images/grid_1.png");

        //Load font
        fge::font::gManager.loadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto* explainText = this->newObject<fge::ObjText>({FGE_SCENE_PLAN_HIGH_TOP},
                                                          "Use WASD/Arrow keys to move the view around\n"
                                                          "Use the mouse wheel to zoom in and out",
                                                          "base", fge::Vector2f{}, 18);
        explainText->setFillColor(fge::Color::Black);

        //Create a select box in order to switch between test objects
        auto* selectBox = this->newObject<fge::ObjSelectBox>({FGE_SCENE_PLAN_HIGH_TOP}, "base");
        selectBox->move({0.0f, 60.0f});
        selectBox->addItem("ObjSpriteBatches");
        selectBox->addItem("ObjSpriteCluster");
        selectBox->addItem("ObjSprite");
        selectBox->addItem("ObjRectangleShape");
        selectBox->addItem("None");

        selectBox->_onSelect.addLambda([&](fge::ObjSelectBox& obj, std::size_t itemIndex) {
            std::cout << "ObjSelectBox: " << itemIndex << " , " << *obj.getItem(itemIndex) << std::endl;

            //Clearing existing objects
            fge::ObjectContainer container;
            this->getAllObj_ByTag(MULTISPRITES_OBJECT_TAG, container);
            std::for_each(container.begin(), container.end(), [&](auto& object) { this->delObject(object->getSid()); });

            if (itemIndex == 0)
            { //Create a sprite batches
                auto* spriteBatches = this->newObject<fge::ObjSpriteBatches>();
                spriteBatches->resize(MAP_SIZE_W * MAP_SIZE_H);
                spriteBatches->addTexture("grid");
                spriteBatches->_tags.add(MULTISPRITES_OBJECT_TAG);

                spriteBatches->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

                fge::Vector2f const textureSize =
                        static_cast<fge::Vector2f>(spriteBatches->getTexture(0).getTextureSize());
                fge::RectInt const textureRect({}, spriteBatches->getTexture(0).getTextureSize());

                for (std::size_t y = 0; y < MAP_SIZE_H; ++y)
                {
                    for (std::size_t x = 0; x < MAP_SIZE_W; ++x)
                    {
                        std::size_t const index = x + y * MAP_SIZE_W;

                        spriteBatches->setSpriteTexture(index, 0);
                        spriteBatches->setTextureRect(index, textureRect);
                        spriteBatches->getTransformable(index)->setPosition(
                                {static_cast<float>(x) * textureSize.x, static_cast<float>(y) * textureSize.y});
                    }
                }
            }
            else if (itemIndex == 1)
            { //Create a sprite cluster
                auto* spriteCluster = this->newObject<fge::ObjSpriteCluster>();
                spriteCluster->resize(MAP_SIZE_W * MAP_SIZE_H);
                spriteCluster->setTexture("grid");
                spriteCluster->_tags.add(MULTISPRITES_OBJECT_TAG);

                spriteCluster->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

                fge::Vector2f const textureSize =
                        static_cast<fge::Vector2f>(spriteCluster->getTexture().getTextureSize());
                fge::RectInt const textureRect({}, spriteCluster->getTexture().getTextureSize());

                for (std::size_t y = 0; y < MAP_SIZE_H; ++y)
                {
                    for (std::size_t x = 0; x < MAP_SIZE_W; ++x)
                    {
                        std::size_t const index = x + y * MAP_SIZE_W;

                        spriteCluster->setTextureRect(index, textureRect);
                        spriteCluster->setOffset(
                                index, {static_cast<float>(x) * textureSize.x, static_cast<float>(y) * textureSize.y});
                    }
                }
            }
            else if (itemIndex == 2)
            { //Create a matrix of sprites
                fge::Texture const texture = "grid";
                fge::Vector2f const textureSize = static_cast<fge::Vector2f>(texture.getTextureSize());

                for (std::size_t y = 0; y < MAP_SIZE_H; ++y)
                {
                    for (std::size_t x = 0; x < MAP_SIZE_W; ++x)
                    {
                        auto* sprite = this->newObject<fge::ObjSprite>();
                        sprite->setTexture(texture);
                        sprite->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;
                        sprite->_tags.add(MULTISPRITES_OBJECT_TAG);
                        sprite->setPosition(
                                {static_cast<float>(x) * textureSize.x, static_cast<float>(y) * textureSize.y});
                    }
                }
            }
            else if (itemIndex == 3)
            { //Create a RectangleShape
                auto* rectangleShape = this->newObject<fge::ObjRectangleShape>();
                rectangleShape->setInstancesCount(MAP_SIZE_W * MAP_SIZE_H);
                rectangleShape->_tags.add(MULTISPRITES_OBJECT_TAG);
                rectangleShape->setSize({16.0f, 16.0f});
                rectangleShape->setOutlineThickness(2.0f);

                rectangleShape->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

                for (std::size_t y = 0; y < MAP_SIZE_H; ++y)
                {
                    for (std::size_t x = 0; x < MAP_SIZE_W; ++x)
                    {
                        std::size_t const index = x + y * MAP_SIZE_W;

                        rectangleShape->setOffset({static_cast<float>(x) * rectangleShape->getSize().x,
                                                   static_cast<float>(y) * rectangleShape->getSize().y},
                                                  index);
                        rectangleShape->setOutlineColor(fge::Color::Black, index);
                        rectangleShape->setFillColor(fge::Color::Transparent, index);
                    }
                }
            }
        });

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
                view.rotate(-10.0f);
            }
            else if (keyEvent.keysym.sym == SDLK_e)
            {
                view.rotate(10.0f);
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

        float fpsMean = 0.0f;
        std::size_t fpsMeanCount = 0;

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

            fpsMean += fge::DurationToSecondFloat(deltaTick);
            if (++fpsMeanCount >= 100)
            {
                std::cout << static_cast<float>(fpsMeanCount) / fpsMean << '\n';
                fpsMeanCount = 0;
                fpsMean = 0.0f;
            }

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

    InstanceLayers.clear();
    InstanceLayers.push_back("VK_LAYER_LUNARG_monitor");

    auto instance = Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 006: multipleSprites");
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

    fge::shader::Init();
    fge::shader::LoadFromFile(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX, "resources/shaders/objShapeInstances_vertex.vert",
                              fge::vulkan::Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::LoadFromFile(FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT, "resources/shaders/objSpriteBatches_fragment.frag",
                              fge::vulkan::Shader::Type::SHADER_FRAGMENT, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::LoadFromFile(FGE_OBJSPRITEBATCHES_SHADER_VERTEX, "resources/shaders/objSpriteBatches_vertex.vert",
                              fge::vulkan::Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);

    fge::RenderWindow renderWindow(vulkanContext, window);
    renderWindow.setClearColor(fge::Color::White);
    renderWindow.setPresentMode(VK_PRESENT_MODE_IMMEDIATE_KHR);

    std::unique_ptr<MainScene> scene = std::make_unique<MainScene>();
    scene->start(renderWindow);
    scene.reset();

    fge::texture::gManager.uninitialize();
    fge::font::gManager.uninitialize();
    fge::shader::Uninit();

    renderWindow.destroy();

    vulkanContext.destroy();

    window.destroy();
    instance.destroy();
    SDL_Quit();

    return 0;
}