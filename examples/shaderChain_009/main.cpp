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
#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objButton.hpp"
#include "FastEngine/object/C_objShaderChain.hpp"
#include "FastEngine/object/C_objShape.hpp"
#include "FastEngine/object/C_objSprite.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_objTextinputbox.hpp"
#include "FastEngine/object/C_objWindow.hpp"
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

        this->setLinkedRenderTarget(&renderWindow);

        this->setCallbackContext({&event, &guiElementHandler});

        //Init texture manager
        fge::texture::Init();
        //Init font manager
        fge::font::Init();

        //Load texture
        fge::texture::LoadFromFile("close", "resources/images/window/close.png");
        fge::texture::LoadFromFile("minimize", "resources/images/window/minimize.png");
        fge::texture::LoadFromFile("resize", "resources/images/window/resize.png");
        fge::texture::LoadFromFile("window", "resources/images/window/window.png");

        fge::texture::LoadFromFile("arrow", "resources/images/arrow_1.png");

        //Load font
        fge::font::LoadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto explainText = this->newObject(FGE_NEWOBJECT(fge::ObjText,
                                                         "Use WASD/Arrow keys to move the view around\n"
                                                         "Use the mouse wheel to zoom in and out",
                                                         "base", {}, 18),
                                           FGE_SCENE_PLAN_HIGH_TOP);
        explainText->getObject<fge::ObjText>()->setFillColor(fge::Color::Black);

        //Create a window to select and load geometry, vertex and fragment shaders
        auto* windowShaders =
                this->newObject(FGE_NEWOBJECT(fge::ObjWindow), FGE_SCENE_PLAN_GUI)->getObject<fge::ObjWindow>();
        windowShaders->setTextureClose("close");
        windowShaders->setTextureMinimize("minimize");
        windowShaders->setTextureResize("resize");
        windowShaders->setTexture("window");
        windowShaders->setSize({300.0f, 400.0f});
        windowShaders->showExitButton(false);

        //Prepare window content
        auto* textInputGeometryPath =
                windowShaders->_windowScene
                        .newObject(FGE_NEWOBJECT(fge::ObjTextInputBox, "base", 30), FGE_SCENE_PLAN_DEFAULT)
                        ->getObject<fge::ObjTextInputBox>();
        textInputGeometryPath->setBoxSize({200.0f, 20.0f});
        textInputGeometryPath->move({10.0f, 50.0f});
        textInputGeometryPath->setString("./shaders/shader.geom");

        auto* textInputVertexPath =
                windowShaders->_windowScene
                        .newObject(FGE_NEWOBJECT(fge::ObjTextInputBox, "base", 30), FGE_SCENE_PLAN_DEFAULT)
                        ->getObject<fge::ObjTextInputBox>();
        textInputVertexPath->setBoxSize({200.0f, 20.0f});
        textInputVertexPath->move({10.0f, 100.0f});
        textInputVertexPath->setString("./shaders/shader.vert");

        auto* textInputFragmentPath =
                windowShaders->_windowScene
                        .newObject(FGE_NEWOBJECT(fge::ObjTextInputBox, "base", 30), FGE_SCENE_PLAN_DEFAULT)
                        ->getObject<fge::ObjTextInputBox>();
        textInputFragmentPath->setBoxSize({200.0f, 20.0f});
        textInputFragmentPath->move({10.0f, 150.0f});
        textInputFragmentPath->setString("./shaders/shader.frag");

        auto* buttonLoadGeometry =
                windowShaders->_windowScene
                        .newObject(FGE_NEWOBJECT(fge::ObjButton, "arrow", "arrow"), FGE_SCENE_PLAN_DEFAULT)
                        ->getObject<fge::ObjButton>();
        buttonLoadGeometry->move({10.0f, textInputGeometryPath->getPosition().y - 20.0f});

        auto* buttonLoadVertex =
                windowShaders->_windowScene
                        .newObject(FGE_NEWOBJECT(fge::ObjButton, "arrow", "arrow"), FGE_SCENE_PLAN_DEFAULT)
                        ->getObject<fge::ObjButton>();
        buttonLoadVertex->move({10.0f, textInputVertexPath->getPosition().y - 20.0f});

        auto* buttonLoadFragment =
                windowShaders->_windowScene
                        .newObject(FGE_NEWOBJECT(fge::ObjButton, "arrow", "arrow"), FGE_SCENE_PLAN_DEFAULT)
                        ->getObject<fge::ObjButton>();
        buttonLoadFragment->move({10.0f, textInputFragmentPath->getPosition().y - 20.0f});

        auto* textOutput = windowShaders->_windowScene
                                   .newObject(FGE_NEWOBJECT(fge::ObjText, "", "base", {}, 18), FGE_SCENE_PLAN_DEFAULT)
                                   ->getObject<fge::ObjText>();
        textOutput->setFillColor(fge::Color::Red);
        textOutput->setOutlineColor(fge::Color::Black);
        textOutput->setOutlineThickness(1.0f);
        textOutput->move({10.0f, textInputFragmentPath->getPosition().y + 40.0f});

        //Create callback for loading shaders
        auto recreateObject = [&]() {
            if (auto obj = this->getFirstObj_ByTag("chain"))
            {
                this->delObject(obj->getSid());
            }

            auto* obj = this->newObject(FGE_NEWOBJECT(fge::ObjShaderChain), FGE_SCENE_PLAN_DEFAULT)
                                ->getObject<fge::ObjShaderChain>();
            obj->setGeometryShader("custom_geometry");
            obj->setVertexShader("custom_vertex");
            obj->setFragmentShader("custom_fragment");
            obj->_tags.add("chain");

            if (!obj->getGeometryShader()->_valid || !obj->getVertexShader()->_valid ||
                !obj->getFragmentShader()->_valid)
            {
                this->delObject(obj->_myObjectData.lock()->getSid());
            }
        };

        buttonLoadGeometry->_onButtonPressed.addLambda([&](auto* button) {
            fge::shader::Unload("custom_geometry");

            auto output = fge::shader::LoadFromFile("custom_geometry", textInputGeometryPath->getString().cpp_str(),
                                                    fge::vulkan::Shader::Type::SHADER_GEOMETRY,
                                                    fge::shader::ShaderInputTypes::SHADER_GLSL, true);

            if (output)
            {
                textOutput->setString("Geometry shader loaded successfully");
                recreateObject();
            }
            else
            {
                textOutput->setString("Failed to load geometry shader");
            }
        });

        buttonLoadVertex->_onButtonPressed.addLambda([&](auto* button) {
            fge::shader::Unload("custom_vertex");

            auto output = fge::shader::LoadFromFile("custom_vertex", textInputVertexPath->getString().cpp_str(),
                                                    fge::vulkan::Shader::Type::SHADER_VERTEX,
                                                    fge::shader::ShaderInputTypes::SHADER_GLSL, true);

            if (output)
            {
                textOutput->setString("Vertex shader loaded successfully");
                recreateObject();
            }
            else
            {
                textOutput->setString("Failed to load vertex shader");
            }
        });

        buttonLoadFragment->_onButtonPressed.addLambda([&](auto* button) {
            fge::shader::Unload("custom_fragment");

            auto output = fge::shader::LoadFromFile("custom_fragment", textInputFragmentPath->getString().cpp_str(),
                                                    fge::vulkan::Shader::Type::SHADER_FRAGMENT,
                                                    fge::shader::ShaderInputTypes::SHADER_GLSL, true);

            if (output)
            {
                textOutput->setString("Fragment shader loaded successfully");
                recreateObject();
            }
            else
            {
                textOutput->setString("Failed to load fragment shader");
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

    auto instance = Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 009: shader chain");
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
                              Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::LoadFromFile(FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT, "resources/shaders/objSpriteBatches_fragment.frag",
                              Shader::Type::SHADER_FRAGMENT, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::LoadFromFile(FGE_OBJSPRITEBATCHES_SHADER_VERTEX, "resources/shaders/objSpriteBatches_vertex.vert",
                              Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);

    fge::RenderWindow renderWindow(vulkanContext, window);
    renderWindow.setClearColor(fge::Color::White);
    renderWindow.setPresentMode(VK_PRESENT_MODE_FIFO_KHR);

    std::unique_ptr<MainScene> scene = std::make_unique<MainScene>();
    scene->start(renderWindow);
    scene.reset();

    fge::texture::Uninit();
    fge::font::Uninit();
    fge::shader::Uninit();

    renderWindow.destroy();

    vulkanContext.destroy();

    window.destroy();
    instance.destroy();
    SDL_Quit();

    return 0;
}