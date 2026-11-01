/*
 * Copyright 2026 Guillaume Guillet
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
#include "FastEngine/extensions/C_objChart.hpp"
#include "FastEngine/graphic/C_renderWindow.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objShape.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_objWindow.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <cmath>
#include <iostream>

#define CHART_POINT_COUNT 25
#define CHART_MOVE_INTERVAL std::chrono::milliseconds(2000)

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
        if (!fge::texture::gManager.initialize())
        {
            std::cout << "Failed to initialize texture manager" << std::endl;
            return;
        }
        //Init font manager
        if (!fge::font::gManager.initialize())
        {
            std::cout << "Failed to initialize font manager" << std::endl;
            return;
        }

        //Load texture
        fge::texture::gManager.loadFromFile("close", "resources/images/window/close.png");
        fge::texture::gManager.loadFromFile("minimize", "resources/images/window/minimize.png");
        fge::texture::gManager.loadFromFile("resize", "resources/images/window/resize.png");
        fge::texture::gManager.loadFromFile("window", "resources/images/window/window.png");

        //Load font
        fge::font::gManager.loadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto* explainText =
                this->newObject<fge::ObjText>({FGE_SCENE_PLAN_HIGH_TOP + 1}, "", "base", fge::Vector2f{}, 18);
        explainText->setFillColor(fge::Color::Black);

        //Create the window
        auto* objWindow = this->newObject<fge::ObjWindow>({FGE_SCENE_PLAN_HIGH_TOP});
        objWindow->setTextureClose("close");
        objWindow->setTextureMinimize("minimize");
        objWindow->setTextureResize("resize");
        objWindow->setTexture("window");
        objWindow->setSize({500.0f, 360.0f});
        objWindow->setPosition({150.0f, 130.0f});
        objWindow->showExitButton(false);

        //Create the chart (using double for the data) that fill the window
        auto* objChart = objWindow->_windowScene.newObject<fge::ObjChartD>();
        objChart->setFont("base");
        objChart->setDynamicSize(
                {{0.0f, 0.0f}, {fge::DynamicSize::SizeModes::SIZE_AUTO, fge::DynamicSize::SizeModes::SIZE_AUTO}});

        //Add some points, a noisy sine wave
        for (std::size_t i = 0; i < CHART_POINT_COUNT; ++i)
        {
            double const x = static_cast<double>(i) * 0.5;
            objChart->addPoint({x, std::sin(x) * 10.0 + fge::_random.range(-2.0, 2.0)});
        }

        //Move every point randomly, the chart will animate the change
        auto const movePoints = [objChart]() {
            for (std::size_t i = 0; i < objChart->getPointCount(); ++i)
            {
                auto point = objChart->getPoint(i);
                point.y += fge::_random.range(-4.0, 4.0);
                objChart->setPoint(i, point);
            }
        };

        auto const updateExplainText = [explainText, objChart]() {
            char const* shapeName = "circle";
            switch (objChart->getPointShape())
            {
            case fge::ObjChartD::PointShapes::SHAPE_SQUARE:
                shapeName = "square";
                break;
            case fge::ObjChartD::PointShapes::SHAPE_DIAMOND:
                shapeName = "diamond";
                break;
            default:
                break;
            }

            explainText->setString(std::string{"Points are moving every 2 seconds, use your mouse to play with the "
                                               "window\n"
                                               "SPACE: move the points now\n"
                                               "A: toggle animation ("} +
                                   (objChart->isAnimationEnabled() ? "on" : "off") +
                                   ")\n"
                                   "R: add a point, D: remove the last point\n"
                                   "S: change point shape (" +
                                   shapeName + ")");
        };
        updateExplainText();

        event._onKeyDown.addLambda([&]([[maybe_unused]] fge::Event const& evt, SDL_KeyboardEvent const& keyEvent) {
            switch (keyEvent.key)
            {
            case SDLK_SPACE:
                movePoints();
                break;
            case SDLK_A:
                objChart->enableAnimation(!objChart->isAnimationEnabled());
                break;
            case SDLK_R:
            {
                double x = 0.0;
                if (objChart->getPointCount() > 0)
                {
                    x = objChart->getPoint(objChart->getPointCount() - 1).x + 0.5;
                }
                objChart->addPoint({x, std::sin(x) * 10.0 + fge::_random.range(-2.0, 2.0)}, fge::Color{220, 70, 50});
                break;
            }
            case SDLK_D:
                if (objChart->getPointCount() > 0)
                {
                    objChart->removePoint(objChart->getPointCount() - 1);
                }
                break;
            case SDLK_S:
                switch (objChart->getPointShape())
                {
                case fge::ObjChartD::PointShapes::SHAPE_CIRCLE:
                    objChart->setPointShape(fge::ObjChartD::PointShapes::SHAPE_SQUARE);
                    break;
                case fge::ObjChartD::PointShapes::SHAPE_SQUARE:
                    objChart->setPointShape(fge::ObjChartD::PointShapes::SHAPE_DIAMOND);
                    break;
                case fge::ObjChartD::PointShapes::SHAPE_DIAMOND:
                    objChart->setPointShape(fge::ObjChartD::PointShapes::SHAPE_CIRCLE);
                    break;
                }
                break;
            default:
                return;
            }
            updateExplainText();
        });

        fge::Clock moveClock;

        //Begin loop
        bool running = true;
        while (running)
        {
            //Update event
            event.process();
            if (event.isEventType(SDL_EVENT_QUIT))
            {
                running = false;
            }
            if (event.isEventType(SDL_EVENT_WINDOW_RESIZED))
            {
                auto view = renderWindow.getView();
                view.resizeFixCenter({event.getWindowSize().x, event.getWindowSize().y});
                renderWindow.setView(view);
                guiElementHandler.checkViewSize();
            }

            if (moveClock.reached(CHART_MOVE_INTERVAL))
            {
                moveClock.restart();
                movePoints();
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

    auto instance = Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 010: objChart");
    Context::enumerateExtensions();

    SurfaceSDLWindow window(instance, FGE_WINDOWPOS_CENTERED, {800, 600}, SDL_WINDOW_RESIZABLE);

    // Check that the window was successfully created
    if (!window.isCreated())
    {
        // In the case that the window could not be made...
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    Context vulkanContext(window);
    vulkanContext._garbageCollector.enable(true);

    if (!fge::shader::gManager.initialize())
    {
        std::cout << "Could not initialize shader manager" << std::endl;
        return 1;
    }
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
