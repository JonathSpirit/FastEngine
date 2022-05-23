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
#include "FastEngine/C_ipAddress.hpp"
#include "FastEngine/fge_endian.hpp"
#include "FastEngine/C_server.hpp"
#include "FastEngine/C_clock.hpp"
#include "FastEngine/C_objRenderMap.hpp"
#include "FastEngine/C_property.hpp"


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

    void first(fge::Scene* scene_ptr) override
    {
        if (!this->_g_lightSystemGate.isOpen())
        {
            this->setDefaultLightSystem(scene_ptr);
        }
        if (this->g_copied)
        {
            this->_tags.add("badBloc");
        }
    }

    void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr) override
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
            scene_ptr->duplicateObject(this->_myObjectData->getSid());
        }
    }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.transform *= this->getTransform();
        target.draw(this->g_shape, states);
    }

    std::string getClassName() const override
    {
        return "FGE:_DEBUG_:BLOC";
    }
    std::string getReadableClassName() const override
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

        window.setFramerateLimit(60);
        window.setKeyRepeatEnabled(true);

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

        fge::font::LoadFromFile("base", "typed.ttf");
        if ( fge::anim::LoadFromFile("animation", "test/anim/anim_data.json") )
        {
            std::cout << "Animation loaded !" << endl;
        }

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

        this->newObject( new fge::ObjSprite("p1", sf::Vector2f(100, 100)) );

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
        buffDataShared = this->newObject(buffTextInputBox, FGE_SCENE_PLAN_MIDDLE);

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
        this->newObject(buffSelectBox, FGE_SCENE_PLAN_MIDDLE);

        buffSwitch = new fge::ObjSwitch("p2", "p1", sf::Vector2f(400, 200) );
        buffSwitch->setColor(sf::Color::Yellow);
        buffButton = new fge::ObjButton("p2", "p1", sf::Vector2f(400, 100) );
        buffButton->setScale(2, 2);
        this->newObject(buffButton, FGE_SCENE_PLAN_MIDDLE);

        buffText = new fge::ObjText("base");
        buffText->setCharacterSize(16);
        buffText->setPosition(10, 300);
        buffText->_tags.add("info");
        buffText->setFillColor(sf::Color::Black);
        buffText->setString("This is a simple test");
        this->newObject(buffText, FGE_SCENE_PLAN_MIDDLE);

        buffAnimation = new fge::ObjAnimation(fge::Animation("animation", "just_a_test"));
        buffAnimation->getAnimation().setLoop(true);
        buffAnimation->setTickDuration(std::chrono::milliseconds(1) );

        buffDataShared = this->newObject(buffAnimation, FGE_SCENE_PLAN_MIDDLE);

        buffDataShared = this->duplicateObject(buffDataShared->getSid());
        buffDataShared->getObject()->move(40, 0);

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

        fge::timer::Create( fge::Timer(std::chrono::milliseconds(1000)) )->_onTimeReached.add( new fge::CallbackFunctor<fge::Timer&>(TestPrintClock) );
        fge::timer::Notify();

        //https://ncase.me/sight-and-light/
        //https://pvigier.github.io/2019/07/28/vagabond-2d-light-system.html

        fge::LightSystem ls;
        this->_properties.setProperty(FGE_LIGHT_PROPERTY_DEFAULT_LS, &ls);

        Bloc* lightBloc = new Bloc();
        this->newObject(lightBloc, 0 );

        fge::ObjLight* testLight = new fge::ObjLight("light", sf::Vector2f(300,300));
        testLight->setColor(sf::Color::Red);
        testLight->setScale(3, 3);
        this->newObject( testLight, 0 );

        testLight = new fge::ObjLight("light", sf::Vector2f(200,200));
        testLight->setColor(sf::Color::Yellow);
        testLight->setScale(2, 2);
        this->newObject( testLight, 0 );

        this->draw(window, false); //<- dummy draw to refresh planDepth
        this->printObjects();

        cout << "My checksum : " << fge::net::GetSceneChecksum(*this) << endl;

        fge::Clock deltaTime;

        while (window.isOpen())
        {
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
                        auto sid = (*it)->getSid();
                        --it;
                        this->delObject(sid);
                    }
                }
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
