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
#include "FastEngine/C_random.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/graphic/C_drawable.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objRectangleShape.hpp"
#include "FastEngine/object/C_objSlider.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "SDL.h"
#include <cmath>
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

        //Load font
        fge::font::LoadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto explainText =
                this->newObject(FGE_NEWOBJECT(fge::ObjText, "Use the slider to change the frequency", "base", {}, 18),
                                FGE_SCENE_PLAN_HIGH_TOP + 1);
        explainText->getObject<fge::ObjText>()->setFillColor(fge::Color::Black);

        //Create a text object that display frequency
        auto* frequencyText =
                this->newObject(FGE_NEWOBJECT(fge::ObjText, "", "base", {}, 18), FGE_SCENE_PLAN_HIGH_TOP + 1)
                        ->getObject<fge::ObjText>();
        frequencyText->setFillColor(fge::Color::Black);
        frequencyText->setPosition({40.0f, 300.0f});

        //Add a text with characters that will be moved
        auto* movingText = this->newObject(FGE_NEWOBJECT(fge::ObjText, "hello world, I'm a moving text !\ttab\nnewLine",
                                                         "base", {200.0f, 200.0f}))
                                   ->getObject<fge::ObjText>();
        movingText->setFillColor(fge::Color::Black);
        movingText->setOutlineThickness(2.0f);
        movingText->setOutlineColor(fge::Color::Yellow);
        movingText->setStyle(fge::ObjText::Style::Italic | fge::ObjText::Style::StrikeThrough |
                             fge::ObjText::Style::Bold | fge::ObjText::Style::Underlined);

        float math_t = 0.0f; //total time
        float math_f = 0.1f; //frequency
        float amp = 30.0f;   //amplitude

        frequencyText->setString(fge::string::ToStr(math_f) + "Hz");

        //Create a slider object for the frequency
        auto* objSliderFreq = this->newObject(FGE_NEWOBJECT(fge::ObjSlider))->getObject<fge::ObjSlider>();
        objSliderFreq->setSize(
                {{10.0f, 0.0f}, {fge::DynamicSize::SizeModes::SIZE_FIXED, fge::DynamicSize::SizeModes::SIZE_AUTO}});
        objSliderFreq->setAnchor(fge::Anchor::Types::ANCHOR_UPLEFT_CORNER,
                                 {fge::Anchor::Shifts::SHIFT_NONE, fge::Anchor::Shifts::SHIFT_NONE});
        objSliderFreq->needAnchorUpdate(false);

        objSliderFreq->_onSlide.add(new fge::CallbackLambda<float>{[&](float ratio) {
            math_f = std::clamp(3.0f * ratio, 0.1f, 3.0f);
            frequencyText->setString(fge::string::ToStr(math_f) + "Hz");
        }});

        //Add a rectangle representing the bounds of the moving text
        auto* rectText = this->newObject(FGE_NEWOBJECT(fge::ObjRectangleShape))->getObject<fge::ObjRectangleShape>();

        auto rect = movingText->getGlobalBounds();
        rectText->setPosition(rect.getPosition());
        rectText->setSize(rect.getSize());
        rectText->setFillColor(fge::Color::Transparent);
        rectText->setOutlineColor(fge::Color::Red);
        rectText->setOutlineThickness(2.0f);

        fge::Clock changeTextColorClock;

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

            //Update moving text characters
            auto& characters = movingText->getCharacters();
            float math_tShift = 0.0f;
            for (auto& c: characters)
            {
                if (changeTextColorClock.reached(std::chrono::milliseconds{500}))
                {
                    c.setFillColor(fge::_random.randColor());
                    c.setOutlineColor(fge::_random.randColor());
                }

                c.setOrigin({0.0f,
                             amp * std::sin(2.0f * static_cast<float>(FGE_MATH_PI) * math_f * (math_t + math_tShift))});
                math_tShift += (1.0f / math_f) / static_cast<float>(characters.size());
            }
            math_t += fge::DurationToSecondFloat(deltaTick);
            math_t = fmodf(math_t, 1.0f / math_f);

            //std::cout << 1.0f / fge::DurationToSecondFloat(deltaTick) << std::endl;

            if (changeTextColorClock.reached(std::chrono::milliseconds{500}))
            {
                changeTextColorClock.restart();
            }

            //Drawing
            auto imageIndex = renderWindow.prepareNextFrame(nullptr);
            if (imageIndex != FGE_RENDERTARGET_BAD_IMAGE_INDEX)
            {
                fge::vulkan::GlobalContext->_garbageCollector.setCurrentFrame(renderWindow.getCurrentFrame());

                renderWindow.beginRenderPass(imageIndex);

                this->draw(renderWindow);

                renderWindow.endRenderPass();

                renderWindow.display(imageIndex);
            }

            //SDL_Delay(17);
        }

        fge::vulkan::GlobalContext->waitIdle();

        fge::vulkan::GlobalContext->_garbageCollector.enable(false);
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window = SDL_CreateWindow("example 005: deepText", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800,
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
    fge::shader::LoadFromFile(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX, "resources/shaders/objShapeInstances_vertex.spv",
                              fge::vulkan::Shader::Type::SHADER_VERTEX);

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