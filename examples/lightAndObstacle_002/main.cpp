#include <FastEngine/C_scene.hpp>
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/C_random.hpp"
#include <FastEngine/C_clock.hpp>
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_objLight.hpp"

//Create a obstacle class object
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

    Obstacle() = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(Obstacle)

    void first([[maybe_unused]] fge::Scene* scene) override
    {
        if (!this->_g_lightSystemGate.isOpen())
        {
            this->setDefaultLightSystem(scene);
        }

        this->setObstacle(this->g_type);
    }

    void update([[maybe_unused]] sf::RenderWindow& screen, [[maybe_unused]] fge::Event& event,
                [[maybe_unused]] const std::chrono::milliseconds& deltaTime, [[maybe_unused]] fge::Scene* scene) override
    {
        auto* follow = scene->_properties["follow"].getPtr<std::string>();
        if (follow != nullptr && *follow == "obstacle" && !this->_tags.check("duplicate"))
        {
            this->setPosition( screen.mapPixelToCoords(event.getMousePixelPos()) );
        }

        this->_g_myPoints.resize(this->g_vertices.getVertexCount());
        for (std::size_t i=0; i<this->g_vertices.getVertexCount(); ++i)
        {
            this->_g_myPoints[i] = this->getTransform().transformPoint(this->g_vertices[i].position);
        }
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        //Draw vertices
        states.transform *= this->getTransform();

        target.draw(this->g_vertices, states);
    }

    void setObstacle(ObstacleTypes type)
    {
        this->g_type = type;
        this->g_vertices.clear();

        switch (type)
        {
        case ObstacleTypes::OBSTACLE_RECTANGLE:
            this->g_vertices.append(sf::Vertex{{0.0f,0.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{0.0f,40.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{40.0f,0.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{40.0f,40.0f}, sf::Color::Green});
            break;
        case ObstacleTypes::OBSTACLE_TRIANGLE:
            this->g_vertices.append(sf::Vertex{{0.0f,0.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{40.0f,20.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{0.0f,40.0f}, sf::Color::Green});
            break;
        case ObstacleTypes::OBSTACLE_CONVEX:
            this->g_vertices.append(sf::Vertex{{0.0f,0.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{10.0f,-20.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{20.0f,0.0f}, sf::Color::Green});

            this->g_vertices.append(sf::Vertex{{30.0f,-20.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{40.0f,0.0f}, sf::Color::Green});
            break;
        case ObstacleTypes::OBSTACLE_CONCAVE:
            this->g_vertices.append(sf::Vertex{{0.0f,0.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{10.0f,-20.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{20.0f,0.0f}, sf::Color::Green});

            this->g_vertices.append(sf::Vertex{{20.0f,10.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{30.0f,10.0f}, sf::Color::Green});

            this->g_vertices.append(sf::Vertex{{30.0f,-20.0f}, sf::Color::Green});
            this->g_vertices.append(sf::Vertex{{40.0f,0.0f}, sf::Color::Green});
            break;
        }
    }

    const char* getClassName() const override
    {
        return "OBSTACLE";
    }
    const char* getReadableClassName() const override
    {
        return "obstacle";
    }

private:
    ObstacleTypes g_type{ObstacleTypes::OBSTACLE_RECTANGLE};
    sf::VertexArray g_vertices{sf::PrimitiveType::TriangleStrip};
};

//Create the MainScene class
class MainScene : public fge::Scene
{
public:

    void main()
    {
        sf::RenderWindow window(sf::VideoMode{800, 600}, "example 002: lightAndObstacle");
        window.setFramerateLimit(60);
        fge::Event event(window);

        //Init texture manager
        fge::texture::Init();
        //Init font manager
        fge::font::Init();

        //Load texture
        fge::texture::LoadFromFile("light_test", "resources/images/light_test.png");

        //Load font
        fge::font::LoadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto explainText = this->newObject(FGE_NEWOBJECT(fge::ObjText, "Use Q/E to switch between light and obstacle follow up\n"
                                                                       "Use 1/2/3/4 to change the obstacle form\n"
                                                                       "Use left mouse click to duplicate the obstacle/light\n"
                                                                       "Use space to delete all duplicated objects\n"
                                                                       "Use right click to change the light color\n",
                                                                       "base", {}, 18), FGE_SCENE_PLAN_HIGH_TOP+1);
        reinterpret_cast<fge::ObjText*>(explainText->getObject())->setFillColor(sf::Color::White);

        //Create the light system
        fge::LightSystem lightSystem;
        this->_properties[FGE_LIGHT_PROPERTY_DEFAULT_LS] = &lightSystem;

        //Create the obstacle
        auto obstacle = this->newObject(FGE_NEWOBJECT(Obstacle), FGE_SCENE_PLAN_MIDDLE);
        reinterpret_cast<Obstacle*>(obstacle->getObject())->scale(2.0f, 2.0f);

        //Create a render map
        auto renderMap = this->newObject(FGE_NEWOBJECT(fge::ObjRenderMap), FGE_SCENE_PLAN_HIGH_TOP);
        reinterpret_cast<fge::ObjRenderMap*>(renderMap->getObject())->setClearColor(sf::Color{10,10,10,240});

        //Create the light
        auto light = this->newObject(FGE_NEWOBJECT(fge::ObjLight, "light_test", {400.0f, 300.0f}), FGE_SCENE_PLAN_MIDDLE);
        reinterpret_cast<fge::ObjLight*>(light->getObject())->setColor(sf::Color::Red);
        reinterpret_cast<fge::ObjLight*>(light->getObject())->setScale(3.0f, 3.0f);

        //Have a property that tell what object must follow the mouse
        this->_properties["follow"] = "obstacle";

        //Add a callback for mouse click
        event._onMouseButtonPressed.add(new fge::CallbackLambda<const fge::Event &, const sf::Event::MouseButtonEvent &>([&](const fge::Event &event, const sf::Event::MouseButtonEvent &mouseEvent)
        {
            //If the left button is pressed
            if (mouseEvent.button == sf::Mouse::Left)
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
            if (mouseEvent.button == sf::Mouse::Right)
            {
                //Change randomly the color of the light
                reinterpret_cast<fge::ObjLight*>(light->getObject())->setColor(fge::_random.randColor());
            }
        }));

        //Add a callback for key pressed
        event._onKeyPressed.add(new fge::CallbackLambda<const fge::Event &, const sf::Event::KeyEvent &>([&](const fge::Event &, const sf::Event::KeyEvent &keyEvent)
        {
            //Changing the obstacle type
            if (keyEvent.code == sf::Keyboard::Key::Num1)
            {
                reinterpret_cast<Obstacle*>(obstacle->getObject())->setObstacle(Obstacle::ObstacleTypes::OBSTACLE_RECTANGLE);
            }
            else if (keyEvent.code == sf::Keyboard::Key::Num2)
            {
                reinterpret_cast<Obstacle*>(obstacle->getObject())->setObstacle(Obstacle::ObstacleTypes::OBSTACLE_TRIANGLE);
            }
            else if (keyEvent.code == sf::Keyboard::Key::Num3)
            {
                reinterpret_cast<Obstacle*>(obstacle->getObject())->setObstacle(Obstacle::ObstacleTypes::OBSTACLE_CONVEX);
            }
            else if (keyEvent.code == sf::Keyboard::Key::Num4)
            {
                reinterpret_cast<Obstacle*>(obstacle->getObject())->setObstacle(Obstacle::ObstacleTypes::OBSTACLE_CONCAVE);
            }

            //Follow up with mouse
            if (keyEvent.code == sf::Keyboard::Key::Q)
            {
                this->_properties["follow"] = "obstacle";
            }
            else if (keyEvent.code == sf::Keyboard::Key::E)
            {
                this->_properties["follow"] = "light";
            }

            //Remove all duplicates
            if (keyEvent.code == sf::Keyboard::Key::Space)
            {
                fge::ObjectContainer duplicates;
                this->getAllObj_ByTag("duplicate", duplicates);
                for (auto& duplicate : duplicates)
                {
                    this->delObject(duplicate->getSid());
                }
            }
        }));

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

            auto* follow = this->_properties["follow"].getPtr<std::string>();
            if (follow != nullptr && *follow == "light")
            {
                light->getObject()->setPosition( window.mapPixelToCoords(event.getMousePixelPos()) );
            }

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

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    MainScene scene;
    scene.main();

    return 0;
}