#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/extra/extra_pathFinding.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_objTilemap.hpp"
#include <FastEngine/C_clock.hpp>
#include <FastEngine/C_scene.hpp>
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

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        //Draw path circles
        for (const auto& circle: this->g_pathCircles)
        {
            target.draw(circle, states);
        }

        //Draw the start circle
        target.draw(this->g_startCircle, states);
    }

    void setWorldSize(const fge::AStar::Vector2i& worldSize) { this->g_pathGenerator.setWorldSize(worldSize); }
    void setTileSize(const sf::Vector2i& tileSize) { this->g_tileSize = tileSize; }
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
                    this->g_pathGenerator.addCollision(
                            static_cast<fge::AStar::Vector2i>(sf::Vector2<std::size_t>{x, y}));
                }
            }
        }
    }
    void setGoal(const sf::Vector2f& globalPos)
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
    void setStart(const sf::Vector2f& globalPos)
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
        this->setPosition(static_cast<float>(this->g_start.x) * static_cast<float>(this->g_tileSize.x) +
                                  static_cast<float>(this->g_tileSize.x) / 2.f,
                          static_cast<float>(this->g_start.y) * static_cast<float>(this->g_tileSize.y) +
                                  static_cast<float>(this->g_tileSize.y) / 2.f);
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
            sf::CircleShape circle;
            circle.setRadius(5.f);
            circle.setOrigin(5.f, 5.f);
            circle.setFillColor(sf::Color::Green);
            circle.setPosition(static_cast<float>(pathPoint.x) * static_cast<float>(this->g_tileSize.x) +
                                       static_cast<float>(this->g_tileSize.x) / 2.f,
                               static_cast<float>(pathPoint.y) * static_cast<float>(this->g_tileSize.y) +
                                       static_cast<float>(this->g_tileSize.y) / 2.f);
            this->g_pathCircles.push_back(circle);
        }

        //Set the start circle
        this->g_startCircle.setRadius(5.f);
        this->g_startCircle.setOrigin(5.f, 5.f);
        this->g_startCircle.setFillColor(sf::Color::Transparent);
        this->g_startCircle.setPosition(static_cast<float>(this->g_start.x) * static_cast<float>(this->g_tileSize.x) +
                                                static_cast<float>(this->g_tileSize.x) / 2.f,
                                        static_cast<float>(this->g_start.y) * static_cast<float>(this->g_tileSize.y) +
                                                static_cast<float>(this->g_tileSize.y) / 2.f);
        this->g_startCircle.setOutlineColor(sf::Color::Red);
        this->g_startCircle.setOutlineThickness(2.f);
    }

    const char* getClassName() const override { return "PATHFINDER"; }
    const char* getReadableClassName() const override { return "pathfinder"; }

private:
    fge::AStar::Generator g_pathGenerator;
    fge::AStar::CoordinateList g_path;
    std::vector<sf::CircleShape> g_pathCircles;
    fge::AStar::Vector2i g_goal;
    fge::AStar::Vector2i g_start;
    sf::Vector2i g_tileSize;
    sf::CircleShape g_startCircle;
};

//Create the MainScene class
class MainScene : public fge::Scene
{
public:
    void main()
    {
        sf::RenderWindow window(sf::VideoMode{800, 600}, "example 001: tileMapAndPathfinding");
        window.setFramerateLimit(60);
        fge::Event event(window);

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
        explainText->getObject<fge::ObjText>()->setFillColor(sf::Color::Black);

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
        pathFinder->getObject<PathFinder>()->setWorldSize(static_cast<fge::AStar::Vector2i>(tileMapSize));
        pathFinder->getObject<PathFinder>()->setTileSize({32, 32});
        pathFinder->getObject<PathFinder>()->setObstacle(reinterpret_cast<fge::ObjTileMap*>(tileMap->getObject()));

        //Create event callback for moving the view
        event._onKeyPressed.add(new fge::CallbackLambda<const fge::Event&, const sf::Event::KeyEvent&>{
                [&](const fge::Event&, const sf::Event::KeyEvent& keyEvent) {
            auto view = window.getView();
            if (keyEvent.code == sf::Keyboard::Left || keyEvent.code == sf::Keyboard::A)
            {
                view.move(-10, 0);
            }
            else if (keyEvent.code == sf::Keyboard::Right || keyEvent.code == sf::Keyboard::D)
            {
                view.move(10, 0);
            }
            else if (keyEvent.code == sf::Keyboard::Up || keyEvent.code == sf::Keyboard::W)
            {
                view.move(0, -10);
            }
            else if (keyEvent.code == sf::Keyboard::Down || keyEvent.code == sf::Keyboard::S)
            {
                view.move(0, 10);
            }
            window.setView(view);
        }});

        //Create event callback for zooming the view
        event._onMouseWheelScrolled.add(
                new fge::CallbackLambda<const fge::Event&, const sf::Event::MouseWheelScrollEvent&>{
                        [&](const fge::Event&, const sf::Event::MouseWheelScrollEvent& mouseWheelScrollEvent) {
            auto view = window.getView();
            if (mouseWheelScrollEvent.delta > 0)
            {
                view.zoom(0.9f);
            }
            else
            {
                view.zoom(1.1f);
            }
            window.setView(view);
                }});

        //Create event callback for mouse button pressed
        event._onMouseButtonPressed.add(new fge::CallbackLambda<const fge::Event&, const sf::Event::MouseButtonEvent&>{
                [&](const fge::Event&, const sf::Event::MouseButtonEvent& mouseButtonEvent) {
            //Get the mouse position
            auto mousePosition = window.mapPixelToCoords(sf::Vector2i{mouseButtonEvent.x, mouseButtonEvent.y});

            //Set the pathfinder goal when the left mouse button is pressed
            if (mouseButtonEvent.button == sf::Mouse::Left)
            {
                pathFinder->getObject<PathFinder>()->setGoal(mousePosition);
            }
            //Set the pathfinder start when the right mouse button is pressed
            else if (mouseButtonEvent.button == sf::Mouse::Right)
            {
                pathFinder->getObject<PathFinder>()->setStart(mousePosition);
            }
        }});

        //Begin loop
        while (window.isOpen())
        {
            //Update event
            event.process(window);
            if (event.isEventType(sf::Event::EventType::Closed))
            {
                window.close();
            }

            //Clear window
            window.clear();

            //Update scene
            auto deltaTick = tick.restart();
            this->update(window, event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTick));

            //Draw scene
            this->draw(window);

            //Display window
            window.display();
        }

        //Uninit texture manager
        fge::texture::Uninit();
        //Uninit font manager
        fge::font::Uninit();
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    MainScene scene;
    scene.main();

    return 0;
}