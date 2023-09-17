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
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/extra/extra_pathFinding.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objCircleShape.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_objTilemap.hpp"
#include "SDL.h"
#include <iostream>

//Create a pathFinder class object
class PathFinder : public fge::Object
{
public:
    PathFinder() = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(PathFinder)

    void first([[maybe_unused]] fge::Scene* scene) override
    {
        this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;
    }

    void draw(fge::RenderTarget& target, const fge::RenderStates& states) const override
    {
        //Draw path circles
        for (const auto& circle: this->g_pathCircles)
        {
            target.draw(circle, states);
        }

        //Draw the start circle
        target.draw(this->g_startCircle, states);
    }

    void setWorldSize(const fge::Vector2i& worldSize) { this->g_pathGenerator.setWorldSize(worldSize); }
    void setTileSize(const fge::Vector2i& tileSize) { this->g_tileSize = tileSize; }
    void setObstacle(fge::ObjTileMap* tileMap)
    {
        //Clear the collisions list
        this->g_pathGenerator.clearCollisions();

        //Get the front tile layer
        auto tileLayer = tileMap->getTileLayers().front();

        //For every tiles
        for (std::size_t x = 0; x < tileLayer->getTiles().getSizeX(); ++x)
        {
            for (std::size_t y = 0; y < tileLayer->getTiles().getSizeY(); ++y)
            {
                const auto& tile = tileLayer->getTiles().get(x, y);
                auto tileSet = tile.getTileSet();

                //Add every red tiles to the collisions list
                if (tileSet && tileSet->getTile(tileSet->getLocalId(tile.getGid()))
                                       ->_properties["isred"]
                                       .get<bool>()
                                       .value_or(false))
                {
                    this->g_pathGenerator.addCollision(static_cast<fge::Vector2i>(fge::Vector2<std::size_t>{x, y}));
                }
            }
        }
    }
    void setGoal(const fge::Vector2f& globalPos)
    {
        //Convert the global position to a tile position
        this->g_goal.x = static_cast<int>(globalPos.x) / this->g_tileSize.x;
        this->g_goal.y = static_cast<int>(globalPos.y) / this->g_tileSize.y;

        //Clamp it
        this->g_goal.x = std::clamp(this->g_goal.x, 0, this->g_pathGenerator.getWorldSize().x - 1);
        this->g_goal.y = std::clamp(this->g_goal.y, 0, this->g_pathGenerator.getWorldSize().y - 1);

        //Generate the path
        this->generatePath();
    }
    void setStart(const fge::Vector2f& globalPos)
    {
        //Convert the global position to a tile position
        this->g_start.x = static_cast<int>(globalPos.x) / this->g_tileSize.x;
        this->g_start.y = static_cast<int>(globalPos.y) / this->g_tileSize.y;

        //Clamp it
        this->g_start.x = std::clamp(this->g_start.x, 0, this->g_pathGenerator.getWorldSize().x - 1);
        this->g_start.y = std::clamp(this->g_start.y, 0, this->g_pathGenerator.getWorldSize().y - 1);

        //Generate the path
        this->generatePath();

        //Set object position in the center of the tile
        this->setPosition({static_cast<float>(this->g_start.x) * static_cast<float>(this->g_tileSize.x) +
                                   static_cast<float>(this->g_tileSize.x) / 2.f,
                           static_cast<float>(this->g_start.y) * static_cast<float>(this->g_tileSize.y) +
                                   static_cast<float>(this->g_tileSize.y) / 2.f});
    }

    void generatePath()
    {
        //Clear the path
        this->g_path.clear();

        //Generate the path
        auto tStart = std::chrono::high_resolution_clock::now();
        this->g_path = this->g_pathGenerator.findPath(this->g_start, this->g_goal);
        auto tDelta_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                 std::chrono::high_resolution_clock::now() - tStart)
                                 .count();
        std::cout << "path was generated in " << tDelta_us << "us" << std::endl;

        //Prepare a circle shape for every path point
        this->g_pathCircles.clear();
        for (const auto& pathPoint: this->g_path)
        {
            fge::ObjCircleShape circle;
            circle.setRadius(5.f);
            circle.setOrigin({5.f, 5.f});
            circle.setFillColor(fge::Color::Green);
            circle.setPosition({static_cast<float>(pathPoint.x) * static_cast<float>(this->g_tileSize.x) +
                                        static_cast<float>(this->g_tileSize.x) / 2.f,
                                static_cast<float>(pathPoint.y) * static_cast<float>(this->g_tileSize.y) +
                                        static_cast<float>(this->g_tileSize.y) / 2.f});
            this->g_pathCircles.push_back(std::move(circle));
        }

        //Set the start circle
        this->g_startCircle.setRadius(5.f);
        this->g_startCircle.setOrigin({5.f, 5.f});
        this->g_startCircle.setFillColor(fge::Color::Transparent);
        this->g_startCircle.setPosition({static_cast<float>(this->g_start.x) * static_cast<float>(this->g_tileSize.x) +
                                                 static_cast<float>(this->g_tileSize.x) / 2.f,
                                         static_cast<float>(this->g_start.y) * static_cast<float>(this->g_tileSize.y) +
                                                 static_cast<float>(this->g_tileSize.y) / 2.f});
        this->g_startCircle.setOutlineColor(fge::Color::Red);
        this->g_startCircle.setOutlineThickness(2.f);
    }

    const char* getClassName() const override { return "PATHFINDER"; }
    const char* getReadableClassName() const override { return "pathfinder"; }

private:
    fge::AStar::Generator g_pathGenerator;
    fge::AStar::CoordinateList g_path;
    std::vector<fge::ObjCircleShape> g_pathCircles;
    fge::Vector2i g_goal;
    fge::Vector2i g_start;
    fge::Vector2i g_tileSize;
    fge::ObjCircleShape g_startCircle;
};

//Create the MainScene class
class MainScene : public fge::Scene
{
public:
    void start(fge::RenderWindow& renderWindow)
    {
        fge::Event event(renderWindow);

        //Init texture manager
        fge::texture::Init();
        //Init font manager
        fge::font::Init();

        //Load texture
        fge::texture::LoadFromFile("tileset_basic", "resources/tilesets/tileset_basic.png");

        //Load font
        fge::font::LoadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto explainText = this->newObject(FGE_NEWOBJECT(fge::ObjText,
                                                         "Use WASD/Arrow keys to move the view around\n"
                                                         "Use the left mouse button to set the goal position\n"
                                                         "Use the right mouse button to set the start position\n"
                                                         "Use the mouse wheel to zoom in and out",
                                                         "base", {}, 18),
                                           FGE_SCENE_PLAN_HIGH_TOP);
        explainText->getObject<fge::ObjText>()->setFillColor(fge::Color::Black);

        //Create a tileMap object
        auto tileMap = this->newObject(FGE_NEWOBJECT(fge::ObjTileMap), FGE_SCENE_PLAN_BACK);

        //Make sure it's always drawn
        tileMap->getObject()->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

        //Load the "tiled" json
        nlohmann::json json;
        fge::LoadJsonFromFile("resources/tilemaps/tilemap_basic_1.json", json);

        //Load the tileMap from a "tiled" json
        tileMap->getObject()->load(json, this);

        //Get the tileMap size
        auto tileMapSize = tileMap->getObject<fge::ObjTileMap>()->getTileLayers().front()->getTiles().getSize();

        //Create a pathfinder object
        auto pathFinder = this->newObject(FGE_NEWOBJECT(PathFinder), FGE_SCENE_PLAN_TOP);
        pathFinder->getObject<PathFinder>()->setWorldSize(static_cast<fge::Vector2i>(tileMapSize));
        pathFinder->getObject<PathFinder>()->setTileSize({32, 32});
        pathFinder->getObject<PathFinder>()->setObstacle(reinterpret_cast<fge::ObjTileMap*>(tileMap->getObject()));

        //Create event callback for moving the view
        event._onKeyDown.add(new fge::CallbackLambda<const fge::Event&, const SDL_KeyboardEvent&>{
                [&](const fge::Event&, const SDL_KeyboardEvent& keyEvent) {
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
        }});

        //Create event callback for zooming the view
        event._onMouseWheel.add(new fge::CallbackLambda<const fge::Event&, const SDL_MouseWheelEvent&>{
                [&](const fge::Event&, const SDL_MouseWheelEvent& mouseWheelEvent) {
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
        }});

        //Create event callback for mouse button pressed
        event._onMouseButtonDown.add(new fge::CallbackLambda<const fge::Event&, const SDL_MouseButtonEvent&>{
                [&](const fge::Event&, const SDL_MouseButtonEvent& mouseButtonEvent) {
            //Get the mouse position
            auto mousePosition = renderWindow.mapPixelToCoords(fge::Vector2i{mouseButtonEvent.x, mouseButtonEvent.y});

            //Set the pathfinder goal when the left mouse button is pressed
            if (mouseButtonEvent.button == SDL_BUTTON_LEFT)
            {
                pathFinder->getObject<PathFinder>()->setGoal(mousePosition);
            }
            //Set the pathfinder start when the right mouse button is pressed
            else if (mouseButtonEvent.button == SDL_BUTTON_RIGHT)
            {
                pathFinder->getObject<PathFinder>()->setStart(mousePosition);
            }
        }});

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
            if (imageIndex != FGE_RENDERTARGET_BAD_IMAGE_INDEX)
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
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window =
            SDL_CreateWindow("example 001: tileMapAndPathfinding", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800,
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

    fge::vulkan::SetActiveContext(vulkanContext);

    vulkanContext._garbageCollector.enable(true);

    fge::shader::Init();
    fge::shader::LoadFromFile(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX, "resources/shaders/objShapeInstances_vertex.vert",
                              fge::vulkan::Shader::Type::SHADER_VERTEX, fge::shader::ShaderInputTypes::SHADER_GLSL);

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