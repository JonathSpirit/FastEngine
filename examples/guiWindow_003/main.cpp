#include <FastEngine/C_scene.hpp>
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/C_random.hpp"
#include <FastEngine/C_clock.hpp>
#include "FastEngine/object/C_objWindow.hpp"
#include "FastEngine/object/C_objTextList.hpp"

//Create the MainScene class
class MainScene : public fge::Scene
{
public:

    void main()
    {
        sf::RenderWindow window(sf::VideoMode{800, 600}, "example 003: guiWindow");
        window.setFramerateLimit(60);

        fge::Event event(window);
        fge::GuiElementHandler guiElementHandler(event, window);
        guiElementHandler.setEventCallback(event);

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
        auto explainText = this->newObject(FGE_NEWOBJECT(fge::ObjText, "Use your mouse to play with a window\n",
                                                                       "base", {}, 18), FGE_SCENE_PLAN_HIGH_TOP+1);
        explainText->getObject<fge::ObjText>()->setFillColor(sf::Color::Black);

        //Create the window
        auto* objWindow = this->newObject(FGE_NEWOBJECT(fge::ObjWindow), FGE_SCENE_PLAN_HIGH_TOP)->getObject<fge::ObjWindow>();
        objWindow->setTextureClose("close");
        objWindow->setTextureMinimize("minimize");
        objWindow->setTextureResize("resize");
        objWindow->getTileSet().setTexture("window");
        objWindow->scale(1.0f, 1.0f);
        objWindow->refreshRectBounds();

        auto* objTextList = objWindow->_windowScene.newObject(FGE_NEWOBJECT(fge::ObjTextList))->getObject<fge::ObjTextList>();
        objTextList->addString("this is a text");
        objTextList->addString("hello world");
        objTextList->addString("good morning");
        objTextList->addString("yes and no");
        objTextList->setFont("base");
        objTextList->move(100.0f, 100.0f);
        objTextList->setCursorRatio(0.0f);

        auto* objTextList2 = this->newObject(FGE_NEWOBJECT(fge::ObjTextList))->getObject<fge::ObjTextList>();
        objTextList2->addString("this is a text");
        objTextList2->addString("hello world");
        objTextList2->addString("good morning");
        objTextList2->addString("yes and no");
        objTextList2->setFont("base");
        objTextList2->move(0.0f, 100.0f);
        objTextList2->setCursorRatio(0.0f);

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

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    MainScene scene;
    scene.main();

    return 0;
}