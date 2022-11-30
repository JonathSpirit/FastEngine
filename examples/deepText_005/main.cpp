#include <FastEngine/C_scene.hpp>
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/C_random.hpp"
#include <FastEngine/C_clock.hpp>
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_objSlider.hpp"
#include <cmath>

//Create the MainScene class
class MainScene : public fge::Scene
{
public:

    void main()
    {
        sf::RenderWindow window(sf::VideoMode{800, 600}, "example 005: deepText");
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

        //Load font
        fge::font::LoadFromFile("base", "resources/fonts/SourceSansPro-Regular.ttf");

        fge::Clock tick;

        //Create a text object with explanation
        auto explainText = this->newObject(FGE_NEWOBJECT(fge::ObjText, "Use the slider to change the frequency",
                                                                       "base", {}, 18), FGE_SCENE_PLAN_HIGH_TOP+1);
        explainText->getObject<fge::ObjText>()->setFillColor(sf::Color::Black);

        //Create a text object that display frequency
        auto* frequencyText = this->newObject(FGE_NEWOBJECT(fge::ObjText, "", "base", {}, 18), FGE_SCENE_PLAN_HIGH_TOP+1)->getObject<fge::ObjText>();
        frequencyText->setFillColor(sf::Color::Black);
        frequencyText->setPosition(40.0f, 300.0f);

        //Add a text with characters that will be moved
        auto* movingText = this->newObject(FGE_NEWOBJECT(fge::ObjText, "hello world, I'm a moving text !\ttab\nnewLine", "base", {200.0f,200.0f}))->getObject<fge::ObjText>();
        movingText->setFillColor(sf::Color::Black);
        movingText->setOutlineThickness(2.0f);
        movingText->setOutlineColor(sf::Color::Yellow);
        movingText->setStyle(fge::ObjText::Style::Italic | fge::ObjText::Style::StrikeThrough | fge::ObjText::Style::Bold | fge::ObjText::Style::Underlined);

        float math_t = 0.0f; //total time
        float math_f = 0.1f; //frequency
        float amp = 30.0f; //amplitude

        frequencyText->setString(fge::string::ToStr(math_f)+"Hz");

        //Create a slider object for the frequency
        auto* objSliderFreq = this->newObject(FGE_NEWOBJECT(fge::ObjSlider))->getObject<fge::ObjSlider>();
        objSliderFreq->setSize({{10.0f, 0.0f}, {fge::DynamicSize::SizeModes::SIZE_FIXED,
                                     fge::DynamicSize::SizeModes::SIZE_AUTO}});
        objSliderFreq->setAnchor(fge::Anchor::Types::ANCHOR_UPLEFT_CORNER, {fge::Anchor::Shifts::SHIFT_NONE,
                                                                         fge::Anchor::Shifts::SHIFT_NONE});
        objSliderFreq->needAnchorUpdate(false);

        objSliderFreq->_onSlide.add( new fge::CallbackLambda<float>{[&](float ratio){
            math_f = 3.0f * ratio;
            frequencyText->setString(fge::string::ToStr(math_f)+"Hz");
        }} );

        //Add a rectangle representing the bounds of the moving text
        sf::RectangleShape rectText;

        auto rect = movingText->getGlobalBounds();
        rectText.setPosition(rect.getPosition());
        rectText.setSize(rect.getSize());
        rectText.setFillColor(sf::Color::Transparent);
        rectText.setOutlineColor(sf::Color::Red);
        rectText.setOutlineThickness(2.0f);

        fge::Clock changeTextColorClock;

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

            //Update moving text characters
            auto& characters = movingText->getCharacters();
            float math_tShift = 0.0f;
            for (auto& c : characters)
            {
                if (changeTextColorClock.reached(std::chrono::milliseconds{500}))
                {
                    c.setFillColor(fge::_random.randColor());
                    c.setOutlineColor(fge::_random.randColor());
                }

                c.setOrigin({0.0f, amp*std::sin(2.0f*static_cast<float>(FGE_MATH_PI)*math_f*(math_t+math_tShift))});
                math_tShift += (1.0f/math_f)/static_cast<float>(characters.size());
            }
            math_t += fge::DurationToSecondFloat(deltaTick);
            math_t = fmodf(math_t, 1.0f/math_f);

            if (changeTextColorClock.reached(std::chrono::milliseconds{500}))
            {
                changeTextColorClock.restart();
            }

            //Draw scene
            this->draw(window);
            window.draw(rectText);

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