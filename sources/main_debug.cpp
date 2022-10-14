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

#include <SFML/Graphics.hpp>

#include <iostream>
#include <chrono>
#include <memory>
#include <string_view>
#include <array>

#include "FastEngine/fastengine_includes.hpp"
#include "FastEngine/extra_objectclass.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/timer_manager.hpp"
#include "FastEngine/reg_manager.hpp"
#include "FastEngine/network_manager.hpp"
#include "FastEngine/C_objAnim.hpp"
#include "FastEngine/C_animation.hpp"
#include "FastEngine/anim_manager.hpp"
#include "FastEngine/crash_manager.hpp"
#include "FastEngine/C_lightSystem.hpp"
#include "FastEngine/C_callback.hpp"
#include "FastEngine/C_tileset.hpp"
#include "FastEngine/C_objLight.hpp"
#include "FastEngine/C_objText.hpp"
#include "FastEngine/C_ipAddress.hpp"
#include "FastEngine/fge_endian.hpp"
#include "FastEngine/C_server.hpp"
#include "FastEngine/C_clock.hpp"
#include "FastEngine/C_objRenderMap.hpp"
#include "FastEngine/C_property.hpp"
#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/arbitraryJsonTypes.hpp"

using namespace std;

class Bloc : public fge::Object, public fge::LightObstacle
{
public:
    Bloc()
    {
        this->g_shape.setSize(sf::Vector2f(64, 32) );
        this->g_shape.setFillColor(sf::Color::Green);
        this->g_shape.setOutlineColor(sf::Color::Red);
        this->g_shape.setOutlineThickness(2);
    }
    Bloc(const Bloc& r) :
            Bloc()
    {
        this->setPosition(r.getPosition());
        this->g_copied = true;
    }
    ~Bloc() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(Bloc)

    void first(fge::Scene* scene) override
    {
        if (!this->_g_lightSystemGate.isOpen())
        {
            this->setDefaultLightSystem(scene);
        }
        if (this->g_copied)
        {
            this->_tags.add("badBloc");
        }
    }

    void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene) override
    {
        if (this->g_copied)
        {
            this->_g_myPoints.resize(this->g_shape.getPointCount());
            for (std::size_t i=0; i<this->g_shape.getPointCount(); ++i)
            {
                this->_g_myPoints[i] = this->getTransform().transformPoint(this->g_shape.getPoint(i));
            }
            return;
        }

        this->setPosition( screen.mapPixelToCoords(event.getMousePixelPos()) );

        this->_g_myPoints.resize(this->g_shape.getPointCount());
        for (std::size_t i=0; i<this->g_shape.getPointCount(); ++i)
        {
            this->_g_myPoints[i] = this->getTransform().transformPoint(this->g_shape.getPoint(i));
        }

        if ( event.isMouseButtonPressed(sf::Mouse::Left) )
        {
            scene->duplicateObject(this->_myObjectData.lock()->getSid());
        }
    }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.transform *= this->getTransform();
        target.draw(this->g_shape, states);
    }

    const char* getClassName() const override
    {
        return "FGE:_DEBUG_:BLOC";
    }
    const char* getReadableClassName() const override
    {
        return "debug bloc";
    }

private:
    sf::RectangleShape g_shape;
    bool g_copied = false;
};

void CallbackFunctionTest()
{
    static unsigned int count = 0;
    std::cout << "callback function test : count = " << count++ << std::endl;
}
class CallbackTestClass : public fge::Subscriber
{
public:
    explicit CallbackTestClass(const std::string& str)
    {
        this->_message = str;
    }
    ~CallbackTestClass() override
    {
        std::cout << "destructor of CallbackTestClass called" << std::endl;
    }

    void callbackMethodTestClass()
    {
        std::cout << "From "<< this <<": " << this->_message << std::endl;
    }

    void onDetach(fge::Subscription* subscription) override
    {
        std::cout << "I'm detached !" << std::endl;
    }

    std::string _message;
};

void TestPrint(const fge::Event& evt, const sf::Event::TextEvent& text)
{
    std::cout << static_cast<char>(text.unicode) << std::endl;
}

void TestPrintClock(fge::Timer& timer)
{
    static fge::Clock clock;

    timer.restart();
    std::cout << "Hi : " << clock.restart<std::chrono::milliseconds>() << "ms" << std::endl;
}

class MainScene : public fge::Scene
{
public:
    MainScene() = default;
    ~MainScene() override = default;

    std::string _updatedText;

    fge::Property cmd_updateTxt(fge::Object* caller, const fge::Property& arg, fge::Scene* caller_scene)
    {
        this->_updatedText = "SID:"+fge::string::ToStr(this->getSid(caller))+" SCENE:"+caller_scene->getName()+" MSG:"+arg.toString();
        return {};
    }

    void printObjects()
    {
        for (auto it = this->begin(); it != this->end(); ++it)
        {
            std::cout << "\tsid: " << (*it)->getSid() << " class: " << (*it)->getObject()->getClassName() << " plan: "
                      << (*it)->getPlan() << " isTopPlan: " << (it == this->findPlan((*it)->getPlan()))
                      << " planDepth: " << (*it)->getPlanDepth() << std::endl;
        }
        std::cout << "\t-----------" << std::endl;
    }

    void main()
    {
        fge::_random.setSeed(2);

        sf::RenderWindow window(sf::VideoMode(800, 600), "FastEngine "+(std::string)FGE_VERSION_FULL_WITHTAG_STRING);
        fge::Event event(window);

        event._onClosed.add( new fge::CallbackLambda<const fge::Event&>([&]([[maybe_unused]] const fge::Event& evt){
            std::cout << "event _onClosed called from a lambda with capture" << std::endl;
        }) );

        window.setFramerateLimit(60);
        window.setKeyRepeatEnabled(true);

        fge::Matrix<std::string> matrixTest{{{"hello", "hi", "gm", "1"},
                                             {"goodbye", "bye", "goodnight", "2"},
                                             {"eat", "lunch", "dinner", "3"}}};

        for (std::size_t y=0; y<matrixTest.getSizeY(); ++y)
        {
            for (std::size_t x=0; x<matrixTest.getSizeX(); ++x)
            {
                std::cout << matrixTest.get(x,y) << " ";
            }
            std::cout << '\n';
        }

        matrixTest.rotateClockwise();
        matrixTest.rotateCounterClockwise(2);

        std::cout << "-------------" << '\n';
        for (std::size_t y=0; y<matrixTest.getSizeY(); ++y)
        {
            for (std::size_t x=0; x<matrixTest.getSizeX(); ++x)
            {
                std::cout << matrixTest[x][y] << " ";
            }
            std::cout << '\n';
        }
        std::cout << "-------------" << '\n';

        fge::Matrix<bool> matrixTest2;
        matrixTest2.setSize(5,5);
        matrixTest2.get(1,1) = true;

        bool test1;
        matrixTest2.get(1,1,test1);

        if (matrixTest2[0][0])
        {
            matrixTest2.fill(true);
        }
        *matrixTest2.getPtr(0,0) = false;

        fge::Property valueTest;
        cout << "value : " << valueTest.get<fge::PuintType>() << endl;
        cout << "value str : " << valueTest.toString() << endl;

        valueTest.set("test");
        cout << "value : " << valueTest.get<std::string>() << endl;
        cout << "value str : " << valueTest.toString() << endl;

        valueTest.setType<std::vector<fge::Property> >();
        cout << "value : " << valueTest.get<fge::PuintType>() << endl;
        cout << "value type : " << valueTest.getType() << endl;

        CallbackTestClass* callbackTestClass = new CallbackTestClass("Hey I'm a text !");

        fge::CallbackHandler<>* callbackHandlerTest = new fge::CallbackHandler();
        callbackHandlerTest->add(new fge::CallbackFunctor(CallbackFunctionTest) );
        callbackHandlerTest->add(new fge::CallbackFunctorObject<CallbackTestClass>(&CallbackTestClass::callbackMethodTestClass, callbackTestClass), callbackTestClass );

        callbackHandlerTest->call();


        delete callbackHandlerTest;

        fge::Property propertyArrayTest;
        propertyArrayTest.clear();
        propertyArrayTest.pushData("I'm a text");
        propertyArrayTest.pushData(fge::Property(1242) );
        propertyArrayTest.pushData(78.12f);
        propertyArrayTest.pushData(true);
        propertyArrayTest.pushData(sf::Vector2f(9.42f, 12.2f));
        propertyArrayTest.pushData(-241);
        propertyArrayTest.pushData("duck");

        propertyArrayTest.setData(2, ":)");
        propertyArrayTest.setData(3, "elephant");
        propertyArrayTest.setData(0, 4269);
        propertyArrayTest.setData(6, "lonnnnng text");
        propertyArrayTest.setData(7, "bunny");

        cout << propertyArrayTest.toString() << endl;
        cout << "size: " << propertyArrayTest.getDataSize() << endl;
        for (std::size_t i=0; i < propertyArrayTest.getDataSize(); ++i)
        {
            cout << propertyArrayTest.getData(i)->toString() << endl;
        }

        fge::texture::Init();
        fge::font::Init();
        fge::timer::Init();
        fge::anim::Init();

        fge::texture::LoadFromFile("p1", "test/anim/p1.png");
        fge::texture::LoadFromFile("p2", "test/anim/p2.png");
        fge::texture::LoadFromFile("light", "test/light_test.png");
        fge::texture::LoadFromFile("arrow", "arrow.png");

        fge::font::LoadFromFile("base", "SourceSansPro-Regular.ttf");
        if ( fge::anim::LoadFromFile("animation", "test/anim/anim_data.json") )
        {
            std::cout << "Animation loaded !" << endl;
        }
        fge::anim::LoadFromFile("animationTileset", "test/testTileset.json");

        fge::crash::Init(window, *fge::font::GetFont("base")->_font);

        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<fge::ObjAnimation>()) );
        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<fge::ObjText>()) );
        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<fge::ObjButton>()) );
        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<fge::ObjSwitch>()) );
        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<fge::ObjSprite>()) );
        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<fge::ObjTextInputBox>()) );
        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<fge::ObjSelectBox>()) );
        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<fge::ObjLight>()) );

        fge::reg::RegisterNewClass( std::unique_ptr<fge::reg::BaseStamp>(new fge::reg::Stamp<Bloc>()) );

        for (std::size_t i=0; i<fge::reg::GetRegisterSize(); ++i)
        {
            fge::reg::BaseStamp* stamp = fge::reg::GetStampOf(i);
            std::cout << stamp->getClassName() << " is registered, with classId " << i << std::endl;
        }

        this->newObject( FGE_NEWOBJECT(fge::ObjSprite, "p1", sf::Vector2f(100, 100)) );

        sf::RectangleShape rectangleTest;

        fge::ObjectDataShared buffDataShared;

        fge::ObjTextInputBox* buffTextInputBox;
        fge::ObjSelectBox* buffSelectBox;
        fge::ObjSwitch* buffSwitch;
        fge::ObjButton* buffButton;
        fge::ObjText* buffText;
        fge::ObjAnimation* buffAnimation;

        this->setName("SuperScene");

        buffTextInputBox = new fge::ObjTextInputBox("base", 20, sf::Vector2f(20, 200));
        buffTextInputBox->setBoxOutlineColor(sf::Color::Blue);
        buffTextInputBox->setTextColor(sf::Color::Yellow);
        buffTextInputBox->setBoxColor(sf::Color::Red);
        buffTextInputBox->setMaxLength(10);
        buffTextInputBox->setCharacterSize(12);
        buffTextInputBox->setHideTextFlag(false);
        buffDataShared = this->newObject(FGE_NEWOBJECT_PTR(buffTextInputBox), FGE_SCENE_PLAN_MIDDLE);

        std::cout << "Switching plan test ..." << std::endl;
        this->setObjectPlan(buffDataShared->getSid(), 1);
        this->setObjectPlan(buffDataShared->getSid(), 1);
        this->setObjectPlan(buffDataShared->getSid(), 1);
        this->setObjectPlan(buffDataShared->getSid(), 4);
        this->setObjectPlan(buffDataShared->getSid(), 3);
        this->setObjectPlan(buffDataShared->getSid(), 2);
        this->setObjectPlan(buffDataShared->getSid(), 0);
        this->setObjectPlan(buffDataShared->getSid(), 25);
        this->setObjectPlan(buffDataShared->getSid(), 14);
        this->setObjectPlan(buffDataShared->getSid(), 1);
        std::cout << "ok" << std::endl;

        this->printObjects();

        buffDataShared = this->duplicateObject(buffDataShared->getSid());
        buffTextInputBox = reinterpret_cast<fge::ObjTextInputBox*>(buffDataShared->getObject());
        buffTextInputBox->setPosition(20, 240);

        buffSelectBox = new fge::ObjSelectBox("base", sf::Vector2f(20, 360));
        buffSelectBox->getTextList().emplace_back("Cool");
        buffSelectBox->getTextList().emplace_back("Not cool");
        buffSelectBox->getTextList().emplace_back("Bad");
        buffSelectBox->getTextList().emplace_back("Really bad");
        buffSelectBox->setBoxSize(buffSelectBox->getBoxSize() + sf::Vector2f(100, 0) );
        buffSelectBox->setSelectedText("Not cool");
        this->newObject(FGE_NEWOBJECT_PTR(buffSelectBox), FGE_SCENE_PLAN_MIDDLE);

        buffSwitch = new fge::ObjSwitch("p2", "p1", sf::Vector2f(400, 200) );
        buffSwitch->setColor(sf::Color::Yellow);
        buffButton = new fge::ObjButton("p2", "p1", sf::Vector2f(400, 100) );
        buffButton->setScale(2, 2);
        this->newObject(FGE_NEWOBJECT_PTR(buffButton), FGE_SCENE_PLAN_MIDDLE);

        buffText = new fge::ObjText("base");
        buffText->setCharacterSize(16);
        buffText->setPosition(10, 300);
        buffText->_tags.add("info");
        buffText->setFillColor(sf::Color::Black);
        buffText->setUtf8String("This is a simple text with utf8 char lik é¨àöüöüäà");
        this->newObject(FGE_NEWOBJECT_PTR(buffText), FGE_SCENE_PLAN_MIDDLE);

        buffAnimation = new fge::ObjAnimation(fge::Animation("animation", "just_a_test"));
        buffAnimation->getAnimation().setLoop(true);
        buffAnimation->getAnimation().setReverse(true);
        buffAnimation->setTickDuration(std::chrono::milliseconds(1) );
        buffDataShared = this->newObject(FGE_NEWOBJECT_PTR(buffAnimation), FGE_SCENE_PLAN_MIDDLE);

        buffDataShared = this->duplicateObject(buffDataShared->getSid());
        buffDataShared->getObject()->move(40, 0);

        buffAnimation = new fge::ObjAnimation(fge::Animation("animationTileset", "group3"));
        buffAnimation->move(80.0f, 0.0f);
        buffAnimation->getAnimation().setLoop(true);
        buffAnimation->getAnimation().setReverse(false);
        buffAnimation->setTickDuration(std::chrono::milliseconds(50) );
        buffDataShared = this->newObject(FGE_NEWOBJECT_PTR(buffAnimation), FGE_SCENE_PLAN_MIDDLE);

        rectangleTest.setFillColor(sf::Color(0, 255, 0));
        rectangleTest.setSize(sf::Vector2f(50, 50) );
        rectangleTest.setOutlineColor(sf::Color(0, 0, 0) );
        rectangleTest.setOutlineThickness(1);

        std::cout << "generating random numbers ..." << std::endl;
        for (std::size_t i=0; i<30; ++i)
        {
            double bba = fge::_random.rand<double>();
            cout << bba << " | " << fge::string::ToStr( bba ) << " | " << fge::string::ToDouble( fge::string::ToStr( bba ) ) << endl;
        }

        sf::Clock clockFps;
        unsigned int countFps=0;
        unsigned int countMaxFps=0;

        event._onTextEntered.add(new fge::CallbackFunctor(TestPrint) );

        fge::timer::Create( std::make_shared<fge::Timer>(std::chrono::milliseconds(1000)) )->_onTimeReached.add( new fge::CallbackFunctor<fge::Timer&>(TestPrintClock) );
        fge::timer::Notify();

        //https://ncase.me/sight-and-light/
        //https://pvigier.github.io/2019/07/28/vagabond-2d-light-system.html

        fge::LightSystem ls;
        this->_properties.setProperty(FGE_LIGHT_PROPERTY_DEFAULT_LS, &ls);

        Bloc* lightBloc = new Bloc();
        this->newObject(FGE_NEWOBJECT_PTR(lightBloc), 0 );

        fge::ObjLight* testLight = new fge::ObjLight("light", sf::Vector2f(300,300));
        testLight->setColor(sf::Color::Red);
        testLight->setScale(3, 3);
        this->newObject(FGE_NEWOBJECT_PTR(testLight), 0 );

        testLight = new fge::ObjLight("light", sf::Vector2f(200,200));
        testLight->setColor(sf::Color::Yellow);
        testLight->setScale(2, 2);
        this->newObject(FGE_NEWOBJECT_PTR(testLight), 0 );

        this->draw(window, false); //<- dummy draw to refresh planDepth
        this->printObjects();

        cout << "My checksum : " << fge::net::GetSceneChecksum(*this) << endl;

        this->setLinkedRenderTarget(&window);

        auto elementTest1 = this->newObject( FGE_NEWOBJECT(fge::ObjButton, "p1", "p2", sf::Vector2f(300, 50)) );
        elementTest1->getObject()->setOrigin(20.0f,20.0f);
        auto elementTest2 = this->newObject( FGE_NEWOBJECT(fge::ObjButton, "p2", "p1") );
        elementTest2->getObject()->setOrigin(-2.0f,2.0f);
        elementTest2->getObject()->setScale(0.5f, 0.5f);

        fge::GuiElementDefault test2;

        test2.setObjectGuiParent(elementTest2);
        test2.setAnchor(fge::AnchorType::ANCHOR_DOWNRIGHT_CORNER, {fge::AnchorShift::SHIFT_NONE, fge::AnchorShift::SHIFT_NONE}, elementTest1->getSid());

        test2.updateAnchor();

        fge::Clock deltaTime;

        {
            auto lock = fge::texture::AcquireLock();
            for (auto it=fge::texture::IteratorBegin(lock); it!=fge::texture::IteratorEnd(lock); ++it)
            {
                std::cout << "TEXTURE: " << it->first << " FROM: " << it->second->_path << std::endl;
            }
        }

        auto newTextTest = this->newObject(FGE_NEWOBJECT(fge::ObjText, "hello world !\ttab\nnewLine", "base", {100.0f,400.0f}));
        auto* newTextTestPtr = (fge::ObjText*)newTextTest->getObject();
        newTextTestPtr->setFillColor(sf::Color::Black);
        newTextTestPtr->setOutlineThickness(2.0f);
        newTextTestPtr->setOutlineColor(sf::Color::Yellow);
        newTextTestPtr->setStyle(fge::ObjText::Style::Italic | fge::ObjText::Style::StrikeThrough | fge::ObjText::Style::Bold | fge::ObjText::Style::Underlined);

        float t = 0.0f;
        float f = 0.0002f;
        float amp = 30.0f;

        sf::RectangleShape rectText;

        auto rect = newTextTestPtr->getGlobalBounds();
        rectText.setPosition(rect.getPosition());
        rectText.setSize(rect.getSize());
        rectText.setFillColor(sf::Color::Transparent);
        rectText.setOutlineColor(sf::Color::Red);
        rectText.setOutlineThickness(2.0f);

        fge::Clock changeTextColorClock;

        while (window.isOpen())
        {
            {
                auto& characters = newTextTestPtr->getCharacters();
                float ti = (1/f)/static_cast<float>(characters.size());
                for (auto& c : characters)
                {
                    if (changeTextColorClock.reached(std::chrono::milliseconds{500}))
                    {
                        c.setFillColor(fge::_random.randColor());
                        c.setOutlineColor(fge::_random.randColor());
                    }

                    c.setOrigin({0.0f, amp*std::sin(2.0f*static_cast<float>(FGE_MATH_PI)*f*(t+ti))});
                    ti += (1/f)/static_cast<float>(characters.size());

                    if (c.getOrigin().y > -(amp/2.0f))
                    {
                        c.setVisibility(false);
                    }
                    else
                    {
                        c.setVisibility(true);
                    }
                }

                if (changeTextColorClock.reached(std::chrono::milliseconds{500}))
                {
                    changeTextColorClock.restart();
                }

                t += 1000.0f/60.0f;
            }


            event.process(window);

            if ( event.isEventType(sf::Event::Closed ) )
            {
                window.close();
            }
            if ( event.isKeyPressed(sf::Keyboard::Space) )
            {
                for (auto it = this->begin(); it != this->end(); ++it)
                {
                    if ( (*it)->getObject()->_tags.check("badBloc") )
                    {
                        auto sid= (*it)->getSid();
                        --it;
                        this->delObject(sid);
                    }
                }
            }
            if ( event.isKeyPressed(sf::Keyboard::A) )
            {
                fge::ObjectContainer container;
                const sf::Vector2i localPosition{event.getMousePixelPos()};

                auto tstart = std::chrono::steady_clock::now();

                this->getAllObj_FromLocalPosition( localPosition, window, container );

                //Only selectable object
                for ( auto it=container.begin(); it!=container.end();)
                {
                    auto* tmpObject = (*it)->getObject();

                    if ( std::strcmp(tmpObject->getClassName(), "FGE:_DEBUG_:BLOC") == 0 )
                    {
                        it = container.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }

                auto tstop = std::chrono::steady_clock::now();
                std::cout << "time took : " << std::chrono::duration_cast<std::chrono::milliseconds>(tstop-tstart).count() << std::endl;
            }

            auto deltaTimeDuration = deltaTime.restart();

            this->update(window, event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTimeDuration));

            std::chrono::duration<float> fsec = deltaTimeDuration;
            rectangleTest.setPosition(fge::ReachVector(rectangleTest.getPosition(), sf::Vector2f(400, 234), 200, fsec.count() ) );
            rectangleTest.setRotation(fge::ReachRotation(rectangleTest.getRotation(), 280, 40, fsec.count(), fge::TurnMode::TURN_CLOCKWISE ) );
            rectangleTest.setFillColor(fge::SetAlpha(rectangleTest.getFillColor(), fge::ReachValue<uint8_t>(rectangleTest.getFillColor().a, 0, 40, fsec.count()) ) );

            ++countFps;
            if (clockFps.getElapsedTime().asMilliseconds() >= 1000 )
            {
                if (countFps > countMaxFps)
                {
                    countMaxFps = countFps;
                }

                std::cout << "FPS : " + fge::string::ToStr(countFps) + " max FPS : " + fge::string::ToStr(countMaxFps) << std::endl;
                std::cout << "Object count : " << this->getObjectSize() << std::endl;

                countFps=0;
                clockFps.restart();
            }

            this->draw( window );
            window.draw(rectangleTest );
            window.draw(rectText);

            window.display();
        }

        fge::timer::Uninit();
        fge::texture::Uninit();
        fge::font::Uninit();
        fge::anim::Uninit();
    }
};

int main(int argc, char *argv[])
{
    MainScene scene;
    scene.main();

    return 0;
}
