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

#include "FastEngine/C_random.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objLight.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "SDL.h"
#include <FastEngine/C_clock.hpp>
#include <FastEngine/C_scene.hpp>
#include <iostream>

std::vector<fge::Vector2f> ConvertTriangleStripTopologyToPolygon(fge::vulkan::Vertex const* vertices, std::size_t count)
{
    std::vector<fge::Vector2f> polygon;

    for (size_t i = 1; i < count; ++i)
    {
        if (i % 2 != 0)
        {
            polygon.push_back(vertices[i]._position);
        }
    }
    for (size_t i = count; i > 0; --i)
    {
        if ((i - 1) % 2 == 0)
        {
            polygon.push_back(vertices[i - 1]._position);
        }
    }

    return polygon;
}

//Create an obstacle class object
class Obstacle : public fge::Object, public fge::LightObstacle
{
public:
    enum class ObstacleTypes
    {
        OBSTACLE_RECTANGLE,
        OBSTACLE_TRIANGLE,
        OBSTACLE_CONVEX,
        OBSTACLE_CONCAVE
    };

    Obstacle() :
            fge::LightObstacle(this),
            g_vertices(fge::vulkan::GetActiveContext())
    {
        this->g_vertices.create(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    }
    Obstacle(Obstacle const& r) :
            fge::Object(r),
            fge::LightObstacle(*this, this),
            g_type(r.g_type),
            g_vertices(r.g_vertices)
    {}

    FGE_OBJ_DEFAULT_COPYMETHOD(Obstacle)

    void first([[maybe_unused]] fge::Scene& scene) override
    {
        if (!this->_g_lightSystemGate.isOpen())
        {
            this->setDefaultLightSystem(scene);
        }

        this->setObstacle(this->g_type);
    }

    void update([[maybe_unused]] fge::RenderWindow& screen,
                [[maybe_unused]] fge::Event& event,
                [[maybe_unused]] std::chrono::microseconds const& deltaTime,
                [[maybe_unused]] fge::Scene& scene) override
    {
        auto* follow = scene._properties["follow"].getPtr<std::string>();
        if (follow != nullptr && *follow == "obstacle" && !this->_tags.check("duplicate"))
        {
            this->setPosition(screen.mapFramebufferCoordsToViewSpace(event.getMousePixelPos()));
        }
    }

    void draw(fge::RenderTarget& target, fge::RenderStates const& states) const override
    {
        //Draw vertices
        auto copyStates = states.copy();
        copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));
        copyStates._vertexBuffer = &this->g_vertices;

        target.draw(copyStates);
    }

    void setObstacle(ObstacleTypes type)
    {
        this->g_type = type;
        this->g_vertices.clear();
        this->_g_shape.clear();

        switch (type)
        {
        case ObstacleTypes::OBSTACLE_RECTANGLE:
            this->g_vertices.append(fge::vulkan::Vertex{{0.0f, 0.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{0.0f, 40.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{40.0f, 0.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{40.0f, 40.0f}, fge::Color::Green});
            break;
        case ObstacleTypes::OBSTACLE_TRIANGLE:
            this->g_vertices.append(fge::vulkan::Vertex{{0.0f, 0.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{40.0f, 20.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{0.0f, 40.0f}, fge::Color::Green});
            break;
        case ObstacleTypes::OBSTACLE_CONVEX:
            this->g_vertices.append(fge::vulkan::Vertex{{0.0f, 0.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{10.0f, -20.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{20.0f, 0.0f}, fge::Color::Green});

            this->g_vertices.append(fge::vulkan::Vertex{{30.0f, -20.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{40.0f, 0.0f}, fge::Color::Green});
            break;
        case ObstacleTypes::OBSTACLE_CONCAVE:
            this->g_vertices.append(fge::vulkan::Vertex{{0.0f, 0.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{0.0f, 20.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{10.0f, 10.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{20.0f, 20.0f}, fge::Color::Green});
            this->g_vertices.append(fge::vulkan::Vertex{{20.0f, 0.0f}, fge::Color::Green});
            break;
        }
    }

    void updateObstacleShape() override
    {
        auto vertices =
                ConvertTriangleStripTopologyToPolygon(this->g_vertices.getVertices(), this->g_vertices.getCount());
        this->_g_shape = fge::ConcavePolygon(std::move(vertices));
        this->_g_shape.convexDecomposition();
    }

    char const* getClassName() const override { return "OBSTACLE"; }
    char const* getReadableClassName() const override { return "obstacle"; }

private:
    ObstacleTypes g_type{ObstacleTypes::OBSTACLE_RECTANGLE};
    fge::vulkan::VertexBuffer g_vertices;
};

//Create the MainScene class
class MainScene : public fge::Scene
{
public:
    void start(fge::RenderWindow& renderWindow)
    {
        fge::Event event(renderWindow);

        //Init texture manager
        fge::texture::gManager.initialize();
        //Init font manager
        fge::font::Init();

        //Load texture
        fge::texture::gManager.loadFromFile("light_test", "resources/images/light_test.png");

        //Load font
        fge::font::LoadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto* explainText = this->newObject<fge::ObjText>({FGE_SCENE_PLAN_HIGH_TOP + 1},
                                                          "Use Q/E to switch between light and obstacle follow up\n"
                                                          "Use A/D to rotate the obstacle\n"
                                                          "Use 1/2/3/4 to change the obstacle form\n"
                                                          "Use left mouse click to duplicate the obstacle/light\n"
                                                          "Use space to delete all duplicated objects\n"
                                                          "Use right click to change the light color\n",
                                                          "base", fge::Vector2f{}, 18);
        explainText->setFillColor(fge::Color::White);

        //Create the light system
        fge::LightSystem lightSystem;
        this->_properties[FGE_LIGHT_PROPERTY_DEFAULT_LS] = &lightSystem;

        //Create the obstacle
        auto obstacle = this->newObject(FGE_NEWOBJECT(Obstacle), FGE_SCENE_PLAN_MIDDLE);
        obstacle->getObject<Obstacle>()->scale({2.0f, 2.0f});

        //Create a render map
        auto* renderMap = this->newObject<fge::ObjRenderMap>({FGE_SCENE_PLAN_HIGH_TOP});
        renderMap->setClearColor(fge::Color{10, 10, 10, 240});

        //Create the light
        auto light =
                this->newObject(FGE_NEWOBJECT(fge::ObjLight, "light_test", {400.0f, 300.0f}), FGE_SCENE_PLAN_MIDDLE);
        light->getObject<fge::ObjLight>()->setColor(fge::Color::Red);
        light->getObject<fge::ObjLight>()->setScale({3.0f, 3.0f});

        //Have a property that tell what object must follow the mouse
        this->_properties["follow"] = "obstacle";

        //Add a callback for mouse click
        event._onMouseButtonDown.addLambda(
                [&]([[maybe_unused]] fge::Event const& event, SDL_MouseButtonEvent const& mouseEvent) {
            //If the left button is pressed
            if (mouseEvent.button == SDL_BUTTON_LEFT)
            {
                //If the obstacle is followed
                if (this->_properties["follow"] == "obstacle")
                {
                    //Duplicate the obstacle
                    auto newObstacle = this->duplicateObject(obstacle->getSid());
                    newObstacle->getObject()->_tags.add("duplicate");
                }
                else
                {
                    //Duplicate the light
                    auto newLight = this->duplicateObject(light->getSid());
                    newLight->getObject()->_tags.add("duplicate");
                }
            }
            //If the right button is pressed
            if (mouseEvent.button == SDL_BUTTON_RIGHT)
            {
                //Change randomly the color of the light
                light->getObject<fge::ObjLight>()->setColor(fge::_random.randColor());
            }
        });

        //Add a callback for key pressed
        event._onKeyDown.addLambda([&](fge::Event const&, SDL_KeyboardEvent const& keyEvent) {
            //Changing the obstacle type
            if (keyEvent.keysym.sym == SDLK_1)
            {
                obstacle->getObject<Obstacle>()->setObstacle(Obstacle::ObstacleTypes::OBSTACLE_RECTANGLE);
            }
            else if (keyEvent.keysym.sym == SDLK_2)
            {
                obstacle->getObject<Obstacle>()->setObstacle(Obstacle::ObstacleTypes::OBSTACLE_TRIANGLE);
            }
            else if (keyEvent.keysym.sym == SDLK_3)
            {
                obstacle->getObject<Obstacle>()->setObstacle(Obstacle::ObstacleTypes::OBSTACLE_CONVEX);
            }
            else if (keyEvent.keysym.sym == SDLK_4)
            {
                obstacle->getObject<Obstacle>()->setObstacle(Obstacle::ObstacleTypes::OBSTACLE_CONCAVE);
            }

            //Follow up with mouse
            if (keyEvent.keysym.sym == SDLK_q)
            {
                this->_properties["follow"] = "obstacle";
            }
            else if (keyEvent.keysym.sym == SDLK_e)
            {
                this->_properties["follow"] = "light";
            }

            //Rotate the obstacle
            if (keyEvent.keysym.sym == SDLK_a)
            {
                obstacle->getObject()->rotate(-10.0f);
            }
            else if (keyEvent.keysym.sym == SDLK_d)
            {
                obstacle->getObject()->rotate(10.0f);
            }

            //Remove all duplicates
            if (keyEvent.keysym.sym == SDLK_SPACE)
            {
                fge::ObjectContainer duplicates;
                this->getAllObj_ByTag("duplicate", duplicates);
                for (auto& duplicate: duplicates)
                {
                    this->delObject(duplicate->getSid());
                }

                obstacle->getObject()->setRotation(0.0f);
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

            //Update scene
            auto deltaTick = tick.restart();
            this->update(renderWindow, event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTick));

            auto* follow = this->_properties["follow"].getPtr<std::string>();
            if (follow != nullptr && *follow == "light")
            {
                light->getObject()->setPosition(renderWindow.mapFramebufferCoordsToViewSpace(event.getMousePixelPos()));
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

    auto instance = Context::init(SDL_INIT_VIDEO | SDL_INIT_EVENTS, "example 002: lightAndObstacle");
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

    fge::RenderWindow renderWindow(vulkanContext, window);
    renderWindow.setClearColor(fge::Color::White);

    std::unique_ptr<MainScene> scene = std::make_unique<MainScene>();
    scene->start(renderWindow);
    scene.reset();

    fge::texture::gManager.uninitialize();
    fge::font::Uninit();
    fge::shader::Uninit();

    renderWindow.destroy();

    vulkanContext.destroy();

    window.destroy();
    instance.destroy();
    SDL_Quit();

    return 0;
}