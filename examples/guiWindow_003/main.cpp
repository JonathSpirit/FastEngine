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
#include "FastEngine/C_random.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/graphic/C_renderWindow.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objSlider.hpp"
#include "FastEngine/object/C_objTextList.hpp"
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

        //Set default callback context
        this->setCallbackContext({&event, &guiElementHandler});

        //Init texture manager
        fge::texture::gManager.initialize();
        //Init font manager
        fge::font::gManager.initialize();

        //Load texture
        fge::texture::gManager.loadFromFile("close", "resources/images/window/close.png");
        fge::texture::gManager.loadFromFile("minimize", "resources/images/window/minimize.png");
        fge::texture::gManager.loadFromFile("resize", "resources/images/window/resize.png");
        fge::texture::gManager.loadFromFile("window", "resources/images/window/window.png");

        //Load font
        fge::font::gManager.loadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto* explainText = this->newObject<fge::ObjText>({FGE_SCENE_PLAN_HIGH_TOP + 1},
                                                          "Use your mouse to play with a window\n"
                                                          "Use space in order to duplicate the window",
                                                          "base", fge::Vector2f{}, 18);
        explainText->setFillColor(fge::Color::Black);

        //Create the window
        auto* objWindow = this->newObject<fge::ObjWindow>({FGE_SCENE_PLAN_HIGH_TOP});
        objWindow->setTextureClose("close");
        objWindow->setTextureMinimize("minimize");
        objWindow->setTextureResize("resize");
        objWindow->setTexture("window");
        objWindow->setSize({200.0f, 200.0f});
        objWindow->showExitButton(false);

        //Create a text list object
        auto* objTextList = objWindow->_windowScene.newObject<fge::ObjTextList>();
        objTextList->addText("this is a text");
        objTextList->addText("hello world");
        objTextList->addText("good morning");
        objTextList->addText("yes and no");
        objTextList->setFont("base");
        objTextList->move({100.0f, 100.0f});
        objTextList->setTextScrollRatio(0.0f);
        objTextList->setBoxSize({{0.0f, 0.0f},
                                 {fge::DynamicSize::SizeModes::SIZE_DEFAULT, fge::DynamicSize::SizeModes::SIZE_DEFAULT},
                                 {-20.0f, 0.0f}});

        //Create a slider object
        auto* objSlider = objWindow->_windowScene.newObject<fge::ObjSlider>();
        objSlider->setSize({{10.0f, 0.0f},
                            {fge::DynamicSize::SizeModes::SIZE_FIXED, fge::DynamicSize::SizeModes::SIZE_AUTO},
                            {0.0f, -50.0f}});
        objSlider->setAnchor(fge::Anchor::Types::ANCHOR_UPRIGHT_CORNER,
                             {fge::Anchor::Shifts::SHIFT_NEGATIVE_BOUNDS, fge::Anchor::Shifts::SHIFT_NONE});
        objSlider->needAnchorUpdate(false);

        //Linking the slide ratio with the text list scroll ratio
        objSlider->_onSlide.addObjectFunctor(&fge::ObjTextList::setTextScrollRatio, objTextList, objTextList);
        objSlider->setScrollInversion(true);

        //Create a slider object for the scaling
        auto* objSlider2 = this->newObject<fge::ObjSlider>();
        objSlider2->setSize(
                {{10.0f, 0.0f}, {fge::DynamicSize::SizeModes::SIZE_FIXED, fge::DynamicSize::SizeModes::SIZE_AUTO}});
        objSlider2->setAnchor(fge::Anchor::Types::ANCHOR_UPLEFT_CORNER,
                              {fge::Anchor::Shifts::SHIFT_NONE, fge::Anchor::Shifts::SHIFT_NONE});
        objSlider2->needAnchorUpdate(false);

        objSlider2->_onSlide.addLambda(
                [](float ratio) { fge::GuiElement::setGlobalGuiScale({2.0f * ratio + 0.5f, 2.0f * ratio + 0.5f}); });

        objSlider2->setCursorRatio(0.25f);

        //Add sliding when cursor is on the window but not on the slider
        objWindow->_onGuiMouseWheelScrolled.addLambda([objSlider, objWindow]([[maybe_unused]] fge::Event const& evt,
                                                                             SDL_MouseWheelEvent const& arg,
                                                                             fge::GuiElementContext& context) {
            if (context._prioritizedElement == objWindow)
            {
                //The _onGuiMouseWheelScrolled on a window is always called even if another element is prioritized
                //So we need to check if the window is the prioritized element. (happens on recursive gui element)
                objSlider->scroll(static_cast<float>(arg.y) * FGE_OBJSLIDER_SCROLL_RATIO_DEFAULT);
            }
        });

        fge::GuiElement::setGlobalGuiScale({1.0f, 1.0f});

        //Add a callback to duplicate the window
        event._onKeyDown.addLambda([&]([[maybe_unused]] fge::Event const& event, SDL_KeyboardEvent const& keyEvent) {
            if (keyEvent.keysym.sym == SDLK_SPACE)
            {
                auto* newWindow =
                        this->duplicateObject(objWindow->_myObjectData.lock()->getSid())->getObject<fge::ObjWindow>();
                newWindow->showExitButton(true);
                newWindow->move({20.0f, 20.0f});

                //Linking the slide ratio with the text list scroll ratio
                auto* newSlider = newWindow->_windowScene.getFirstObj_ByClass(FGE_OBJSLIDER_CLASSNAME)
                                          ->getObject<fge::ObjSlider>();
                auto* newTextList = newWindow->_windowScene.getFirstObj_ByClass(FGE_OBJTEXTLIST_CLASSNAME)
                                            ->getObject<fge::ObjTextList>();
                newSlider->_onSlide.addObjectFunctor(&fge::ObjTextList::setTextScrollRatio, newTextList, newTextList);
                newSlider->setScrollInversion(true);

                newWindow->_onGuiMouseWheelScrolled.addLambda(
                        [newSlider, newWindow]([[maybe_unused]] fge::Event const& evt, SDL_MouseWheelEvent const& arg,
                                               fge::GuiElementContext& context) {
                    if (context._prioritizedElement == newWindow)
                    {
                        //The _onGuiMouseWheelScrolled on a window is always called even if another element is prioritized
                        //So we need to check if the window is the prioritized element. (happens on recursive gui element)
                        newSlider->scroll(static_cast<float>(arg.y) * FGE_OBJSLIDER_SCROLL_RATIO_DEFAULT);
                    }
                });
            }
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
            if (event.isEventType(SDL_WINDOWEVENT))
            {
                auto view = renderWindow.getView();
                view.resizeFixCenter({event.getWindowSize().x, event.getWindowSize().y});
                renderWindow.setView(view);
                guiElementHandler.checkViewSize();
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

    auto instance = Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 003: guiWindow");
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
    fge::shader::gManager.loadFromFile(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX,
                                       "resources/shaders/objShapeInstances_vertex.vert", Shader::Type::SHADER_VERTEX,
                                       fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::gManager.loadFromFile(FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT,
                                       "resources/shaders/objSpriteBatches_fragment.frag",
                                       Shader::Type::SHADER_FRAGMENT, fge::shader::ShaderInputTypes::SHADER_GLSL);
    fge::shader::gManager.loadFromFile(FGE_OBJSPRITEBATCHES_SHADER_VERTEX,
                                       "resources/shaders/objSpriteBatches_vertex.vert", Shader::Type::SHADER_VERTEX,
                                       fge::shader::ShaderInputTypes::SHADER_GLSL);

    fge::RenderWindow renderWindow(vulkanContext, window);
    renderWindow.setClearColor(fge::Color::White);

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