/*
 * Copyright 2022 Guillaume Guillet
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
        guiElementHandler.setEventCallback(event);

        this->setLinkedRenderTarget(&renderWindow);

        //Set default callback context
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

        //Load font
        fge::font::LoadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto explainText = this->newObject(FGE_NEWOBJECT(fge::ObjText,
                                                         "Use your mouse to play with a window\n"
                                                         "Use space in order to duplicate the window",
                                                         "base", {}, 18),
                                           FGE_SCENE_PLAN_HIGH_TOP + 1);
        explainText->getObject<fge::ObjText>()->setFillColor(fge::Color::Black);

        //Create the window
        auto* objWindow =
                this->newObject(FGE_NEWOBJECT(fge::ObjWindow), FGE_SCENE_PLAN_HIGH_TOP)->getObject<fge::ObjWindow>();
        objWindow->setTextureClose("close");
        objWindow->setTextureMinimize("minimize");
        objWindow->setTextureResize("resize");
        objWindow->setTexture("window");
        objWindow->setSize({200.0f, 200.0f});
        objWindow->showExitButton(false);

        //Create a text list object
        auto* objTextList =
                objWindow->_windowScene.newObject(FGE_NEWOBJECT(fge::ObjTextList))->getObject<fge::ObjTextList>();
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
        auto* objSlider = objWindow->_windowScene.newObject(FGE_NEWOBJECT(fge::ObjSlider))->getObject<fge::ObjSlider>();
        objSlider->setSize({{10.0f, 0.0f},
                            {fge::DynamicSize::SizeModes::SIZE_FIXED, fge::DynamicSize::SizeModes::SIZE_AUTO},
                            {0.0f, -50.0f}});
        objSlider->setAnchor(fge::Anchor::Types::ANCHOR_UPRIGHT_CORNER,
                             {fge::Anchor::Shifts::SHIFT_NEGATIVE_BOUNDS, fge::Anchor::Shifts::SHIFT_NONE});
        objSlider->needAnchorUpdate(false);

        //Linking the slide ratio with the text list scroll ratio
        objSlider->_onSlide.add(new fge::CallbackFunctorObject(&fge::ObjTextList::setTextScrollRatio, objTextList),
                                objTextList);

        //Create a slider object for the scaling
        auto* objSlider2 = this->newObject(FGE_NEWOBJECT(fge::ObjSlider))->getObject<fge::ObjSlider>();
        objSlider2->setSize(
                {{10.0f, 0.0f}, {fge::DynamicSize::SizeModes::SIZE_FIXED, fge::DynamicSize::SizeModes::SIZE_AUTO}});
        objSlider2->setAnchor(fge::Anchor::Types::ANCHOR_UPLEFT_CORNER,
                              {fge::Anchor::Shifts::SHIFT_NONE, fge::Anchor::Shifts::SHIFT_NONE});
        objSlider2->needAnchorUpdate(false);

        objSlider2->_onSlide.add(new fge::CallbackLambda<float>{[](float ratio) {
            fge::GuiElement::setGlobalGuiScale({2.0f * ratio + 0.5f, 2.0f * ratio + 0.5f});
        }});

        fge::GuiElement::setGlobalGuiScale({1.0f, 1.0f});

        //Add a callback to duplicate the window
        event._onKeyDown.add(new fge::CallbackLambda<const fge::Event&, const SDL_KeyboardEvent&>(
                [&]([[maybe_unused]] const fge::Event& event, const SDL_KeyboardEvent& keyEvent) {
            if (keyEvent.keysym.sym == SDLK_SPACE)
            {
                auto newObject = this->duplicateObject(objWindow->_myObjectData.lock()->getSid());
                newObject->getObject<fge::ObjWindow>()->showExitButton(true);
                newObject->getObject()->move({20.0f, 20.0f});

                //Linking the slide ratio with the text list scroll ratio
                auto* newSlider = newObject->getObject<fge::ObjWindow>()
                                          ->_windowScene.getFirstObj_ByClass(FGE_OBJSLIDER_CLASSNAME)
                                          ->getObject<fge::ObjSlider>();
                auto* newTextList = newObject->getObject<fge::ObjWindow>()
                                            ->_windowScene.getFirstObj_ByClass(FGE_OBJTEXTLIST_CLASSNAME)
                                            ->getObject<fge::ObjTextList>();
                newSlider->_onSlide.add(
                        new fge::CallbackFunctorObject(&fge::ObjTextList::setTextScrollRatio, newTextList),
                        newTextList);
            }
        }));

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
            auto imageIndex = renderWindow.prepareNextFrame(nullptr);
            if (imageIndex != BAD_IMAGE_INDEX)
            {
                fge::vulkan::GlobalContext->_garbageCollector.setCurrentFrame(renderWindow.getCurrentFrame());

                renderWindow.beginRenderPass(imageIndex);

                this->draw(renderWindow);

                renderWindow.endRenderPass();

                renderWindow.display(imageIndex);
            }
        }

        fge::vulkan::GlobalContext->waitIdle();

        fge::vulkan::GlobalContext->_garbageCollector.enable(false);
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window = SDL_CreateWindow("example 003: guiWindow", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800,
                                          600, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

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

    fge::vulkan::GlobalContext = &vulkanContext;

    fge::vulkan::GlobalContext->_garbageCollector.enable(true);

    fge::shader::Init("resources/shaders/vertex.spv", "resources/shaders/fragment.spv",
                      "resources/shaders/fragmentTexture.spv");

    fge::RenderWindow renderWindow(vulkanContext);
    renderWindow.setClearColor(fge::Color::White);

    std::unique_ptr<MainScene> scene = std::make_unique<MainScene>();
    scene->start(renderWindow);
    scene.reset();

    fge::texture::Uninit();
    fge::font::Uninit();
    fge::shader::Uninit();

    renderWindow.destroy();

    vulkanContext.destroy();

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}