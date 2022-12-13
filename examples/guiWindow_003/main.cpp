#include "FastEngine/C_random.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/object/C_objSlider.hpp"
#include "FastEngine/object/C_objTextList.hpp"
#include "FastEngine/object/C_objWindow.hpp"
#include <FastEngine/C_clock.hpp>
#include <FastEngine/C_scene.hpp>

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

        this->setLinkedRenderTarget(&window);

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
        explainText->getObject<fge::ObjText>()->setFillColor(sf::Color::Black);

        //Create the window
        auto* objWindow =
                this->newObject(FGE_NEWOBJECT(fge::ObjWindow), FGE_SCENE_PLAN_HIGH_TOP)->getObject<fge::ObjWindow>();
        objWindow->setTextureClose("close");
        objWindow->setTextureMinimize("minimize");
        objWindow->setTextureResize("resize");
        objWindow->getTileSet().setTexture("window");
        objWindow->setSize({200.0f, 200.0f});
        objWindow->showExitButton(false);

        //Create a text list object
        auto* objTextList =
                objWindow->_windowScene.newObject(FGE_NEWOBJECT(fge::ObjTextList))->getObject<fge::ObjTextList>();
        objTextList->addString("this is a text");
        objTextList->addString("hello world");
        objTextList->addString("good morning");
        objTextList->addString("yes and no");
        objTextList->setFont("base");
        objTextList->move(100.0f, 100.0f);
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
        event._onKeyPressed.add(new fge::CallbackLambda<const fge::Event&, const sf::Event::KeyEvent&>(
                [&]([[maybe_unused]] const fge::Event& event, const sf::Event::KeyEvent& keyEvent) {
            if (keyEvent.code == sf::Keyboard::Space)
            {
                auto newObject = this->duplicateObject(objWindow->_myObjectData.lock()->getSid());
                newObject->getObject<fge::ObjWindow>()->showExitButton(true);
                newObject->getObject()->move(20.0f, 20.0f);

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