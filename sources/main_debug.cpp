#include <SFML/Graphics.hpp>
#include <iostream>
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
#include "FastEngine/C_networkType.hpp"
#include "FastEngine/C_socket.hpp"
#include "FastEngine/C_packetBZ2.hpp"
#include "FastEngine/C_packetLZ4.hpp"
#include "FastEngine/C_lightSystem.hpp"
#include "FastEngine/C_callback.hpp"
#include "FastEngine/C_tileset.hpp"
#include "FastEngine/C_matrix.hpp"
#include "FastEngine/C_objLight.hpp"
#include <chrono>
#include <random>
#include <fstream>
#include <system_error>
#include <string_view>
#include <array>
#include <pcg_random.hpp>
#include "FastEngine/C_socket.hpp"
#include "FastEngine/C_ipAddress.hpp"
#include "FastEngine/C_packet.hpp"
#include "FastEngine/fge_endian.hpp"
#include "FastEngine/C_server.hpp"
#include "FastEngine/C_clock.hpp"
#include "FastEngine/C_objRenderMap.hpp"
#include "FastEngine/C_bitBank.hpp"

#define CLIENT 1
#define TEST_UDP 0
#define TEST_PCK fge::net::Packet

using namespace std;

class Bloc : public fge::Object, public fge::LightObstacle
{
public:
    Bloc()
    {
        this->_myshape.setSize( sf::Vector2f(64,32) );
        this->_myshape.setFillColor(sf::Color::Green);
        this->_myshape.setOutlineColor(sf::Color::Red);
        this->_myshape.setOutlineThickness(2);
    }
    Bloc(const Bloc& r)
    {
        this->_myshape.setSize( sf::Vector2f(64,32) );
        this->_myshape.setFillColor(sf::Color::Green);
        this->_myshape.setOutlineColor(sf::Color::Red);
        this->_myshape.setOutlineThickness(2);

        this->setPosition(r.getPosition());
        this->_copied = true;
        r._ls->addGate(&this->_g_lightSystemGate);
    }
    ~Bloc()
    {
    }

    FGE_OBJ_DEFAULT_COPYMETHOD(Bloc)

    void setLightSystem(fge::LightSystem* ls)
    {
        ls->addGate(&this->_g_lightSystemGate);
        _ls = ls;
    }

    void first(fge::Scene* scene_ptr)
    {
        this->_g_lightSystemGate.setData(this);
    }

    void update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr)
    {
        if (_copied)
        {
            this->_myshape.setPosition(this->getPosition());
            this->_myshape.setOrigin(this->getOrigin());

            this->_g_myPoints.resize(this->_myshape.getPointCount());
            for (std::size_t i=0; i<this->_myshape.getPointCount(); ++i)
            {
                this->_g_myPoints[i] = this->_myshape.getTransform().transformPoint(this->_myshape.getPoint(i));
            }
            return;
        }
        this->setPosition( screen.mapPixelToCoords(event.getMousePixelPos()) );

        this->_myshape.setPosition(this->getPosition());
        this->_myshape.setOrigin(this->getOrigin());

        scene_ptr->callCmd("updateTxt", this, fge::Value("Coucou"), scene_ptr).toString();

        this->_g_myPoints.resize(this->_myshape.getPointCount());
        for (std::size_t i=0; i<this->_myshape.getPointCount(); ++i)
        {
            this->_g_myPoints[i] = this->_myshape.getTransform().transformPoint(this->_myshape.getPoint(i));
        }

        if ( event.isMouseButtonPressed(sf::Mouse::Left) )
        {
            scene_ptr->duplicateObject(this->_myObjectData->getSid());
        }
    }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        //_test->_renderTexture.draw(this->_myshape, states);
        target.draw(this->_myshape, states);
    }

    std::string getClassName() const
    {
        return "FGE:_DEBUG_:TESTLIGHT";
    }

    ///Var
    bool _copied = false;
    sf::RectangleShape _myshape;
    fge::LightSystem* _ls;
    //fge::ObjRenderMap* _test;
};

void monTest(bool test)
{
    static unsigned int kk = 0;
    std::cout << "coucou" << kk++ << std::endl;
}
class SuperTest : public fge::Subscriber
{
public:
    SuperTest(const std::string& str)
    {
        you = str;
        valTest.set<unsigned int>(567);
    }
    ~SuperTest()
    {
        std::cout << "Deconstructeur de supertest" << std::endl;
    }

    void monTestClass(bool test)
    {
        std::cout << you << std::endl;
    }
    void printValeur() const
    {
        std::cout << *valTest.get<unsigned int>() << std::endl;
    }

    void onDetach(fge::Subscription* subscription) override
    {
        std::cout << "I'm detached !" << std::endl;
    }

    std::string you;
    fge::Value valTest;
    fge::CallbackHandler<bool> wesh;
};

void Bonjour(int& a, const int& b)
{
    std::cout << a << " "<< b << std::endl;
    ++a;
}

void TestPrint(const fge::Event& evt, const sf::Event::TextEvent& text)
{
    std::cout << (char)text.unicode << std::endl;
}

fge::Clock myClock;
void TestPrintClock(fge::Timer& timer)
{
    timer.restart();
    std::cout << "Hi : " << myClock.restart<std::chrono::milliseconds>() << std::endl;
}



struct A
{
    uint8_t _a;
};

class mainScene : public fge::Scene
{
public:
    mainScene()
    {
    }
    ~mainScene()
    {
    }

    std::string _supertext;

    fge::Value cmd_updateTxt(fge::Object* caller, const fge::Value& arg, fge::Scene* caller_scene)
    {
        _supertext = "SID:"+fge::string::ToStr(this->getSid(caller))+" SCENE:"+caller_scene->getName()+" MSG:"+arg.toString();
        return fge::Value("thx bro !");
    }

    void main()
    {
        /*fge::net::ServerUdp test;
        #if CLIENT == 0
        test.start(42320);
        #else
        test.start(42321);
        #endif

        fge::net::ServerFluxUdp* flux = test.getDefaultFlux();

        fge::net::Identity id;
        id._ip = "127.0.0.1";
        #if CLIENT == 0
        id._port = 42321;
        #else
        id._port = 42320;
        #endif

        flux->_clients.add(id, fge::net::ClientSharedPtr(new fge::net::Client(200)));

        #if CLIENT == 1
            std::string msg;
            do
            {
                std::getline(std::cin, msg);
                std::shared_ptr<fge::net::Packet> pck = std::make_shared<TEST_PCK>();
                *pck << msg;

                flux->_clients.sendToAll(pck);

                //std::cout << "Sent " << pck->getLastCompressionSize() << " bytes of data" << std::endl;
            }
            while (msg != "stop");
        #else
            TEST_PCK pck;
            fge::net::IpAddress ip;
            uint16_t port;

            std::string msg;
            do
            {
                fge::net::FluxPacketSharedPtr fluxPck = flux->popNextPacket();

                if (fluxPck)
                {
                    fluxPck->_pck >> msg;
                    std::cout << "Received : " << msg << " , from " << fluxPck->_id._ip.toString() << ":" << fluxPck->_id._port << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            while (msg != "stop");
        #endif // CLIENT

        test.stop();
        return;*/


        /*#if 0
        fge::net::Socket::initSocket();

        #if TEST_UDP == 1
            fge::net::SocketUdp sock;

            #if CLIENT == 1
            std::cout << (sock.connect("127.0.0.1", 42042) == fge::net::Socket::ERR_NOERROR) << std::endl;

            std::string msg;
            do
            {
                std::getline(std::cin, msg);
                TEST_PCK pck;
                pck << msg;

                sock.send(pck);
                std::cout << "Sent " << pck.getLastCompressionSize() << " bytes of data" << std::endl;
            }
            while (msg != "stop");
            #else
            sock.setBlocking(true);
            sock.bind(42042, "127.0.0.1");

            std::cout << (sock.connect("127.0.0.1", 0) == fge::net::Socket::ERR_NOERROR) << std::endl;

            TEST_PCK pck;
            fge::net::IpAddress ip;
            uint16_t port;

            std::string msg;
            do
            {
                sock.receiveFrom(pck, ip, port);

                pck >> msg;
                std::cout << "Received : " << msg << " , from " << ip.toString() << ":" << port << std::endl;
            }
            while (msg != "stop");

            #endif // CLIENT
        #else
            fge::net::SocketTcp sock;

            #if CLIENT == 1
            std::cout << (sock.connect("127.0.0.1", 42042, 2000) == fge::net::Socket::ERR_NOERROR) << std::endl;

            std::string msg;
            do
            {
                std::getline(std::cin, msg);
                TEST_PCK pck;
                pck << msg;

                sock.send(pck);
                std::cout << "Sent " << pck.getLastCompressionSize() << " bytes of data" << std::endl;
            }
            while (msg != "stop");
            #else

            fge::net::SocketListenerTcp lisener;
            lisener.listen(42042);
            lisener.setBlocking(true);

            lisener.accept(sock);

            sock.setBlocking(true);

            TEST_PCK pck;

            std::string msg;
            do
            {
                fge::net::Socket::Error err;
                err = sock.select(true, 2000);
                if (err == fge::net::Socket::ERR_DONE)
                {
                    while ( sock.receive(pck) == fge::net::Socket::ERR_PARTIAL );
                    pck >> msg;

                    std::cout << msg << std::endl;
                }
                else
                {
                    std::cout << "timeout" << std::endl;
                }
            }
            while (msg != "stop");

            #endif // CLIENT
        #endif //TEST_UDP

        fge::net::Socket::uninitSocket();
        return;
        #endif */

        uint16_t ticks = 0;
        /*fge::CallbackHandler<int, int, int> testc;
        testc += new fge::CallbackFunctor<int, int, int>(Bonjour);
        testc.call(1, 2, 56);*/


        //int log12;
        //fge::Value paslogique( (const int&&)log12 );
        /*std::list<std::string> test;
        fge::GetFilesInFolder(test, "", "*.zip");
        for ( std::string& val : test )
        {
            std::cout << "----- " << val << std::endl;
        }



        return;*/

        /*std::list<std::string> test;

        std::cout << fge::GetFilesInFolder(test, "test/", ".*", true, true, true) << std::endl;

        for ( std::string& val : test )
        {
            std::cout << val << std::endl;
        }*/

        /*sf::Vector2f pos = sf::Vector2f(0,0);
        sf::Vector2f target = sf::Vector2f(700, -700);
        float speed = 20.0;
        float dTime = 0.5;

        while (pos != target)
        {
            pos = fge::ReachVector(pos, target, speed, dTime);
            std::cout << fge::string::ToStr(pos) << std::endl;
        }

        return;*/

        /*fge::Matrix<std::string> testm{
            {"un", "deux", "trois"},
            {"quatre", "cinq", "six"},
            {"sept", "huit", "neuf"}};

        nlohmann::json monJson;
        monJson["coucou"] = "bonjour";
        monJson["test1"] = 42;
        monJson["test2"] = true;
        monJson["matrix"] = testm;

        std::ofstream outFile("test/yes.json");
        if ( outFile )
        {
            outFile << std::setw(2) << monJson << std::endl;
        }
        outFile.close();*/

        /*std::ifstream inFile("test/yes.json");
        if ( !inFile )
        {
            inFile.close();
            return;
        }

        nlohmann::json monJson;
        inFile >> monJson;
        inFile.close();

        fge::Matrix<std::string> testm;

        monJson["matrix"].get_to(testm);

        for (std::size_t y=0; y<testm.getSizeY(); ++y)
        {
            for (std::size_t x=0; x<testm.getSizeX(); ++x)
            {
                std::cout << testm[x][y] << " ";
            }
            std::cout << std::endl;
        }

        return;*/

       /* sf::Vector2f otest = sf::Vector2f(1,0);
        std::vector<float> ptest =
        {
            //sf::Vector2f(1,1),
            //sf::Vector2f(1,-1),
            //sf::Vector2f(-1,0),
            //sf::Vector2f(-1,-1),
            //sf::Vector2f(-1,1),
            //sf::Vector2f(1,1),
            //sf::Vector2f(0,0),
            //sf::Vector2f(1,0)
            45,
            315,
            180,
            225,
            135,
            45,
            0,
            0
        };

        for (auto& value : ptest)
        {
            std::cout << fge::string::ToStr(fge::GetForwardVector(value)) << " " << fge::GetRotation(fge::GetForwardVector(value)) << std::endl;
        }

        return;*/


        fge::Event main_event(sf::Vector2u(800,600));
        sf::RenderWindow window(sf::VideoMode(800, 600), "FastEngine "+(std::string)fge::VERSION_FULLVERSION_STRING);
        sf::View canard = window.getView();canard.zoom(2.0f);
        window.setFramerateLimit(144);
        window.setKeyRepeatEnabled(true);

        sf::RenderWindow* banane = &window;
        fge::Value superTest;

        superTest = &window;

        fge::Value valueTest;
        cout << "value : " << valueTest.get<unsigned int>() << endl;
        cout << "value str : " << valueTest.toString() << endl;
        valueTest.set("test");
        valueTest.setType<std::vector<fge::Value> >();
        cout << "value : " << valueTest.get<unsigned int>() << endl;
        //cout << "value cast : " << (int*)valueTest << endl;
        cout << "value : " << valueTest.get<int*>() << endl;
        valueTest.set<unsigned int>(241);
        //*valueTest.get<unsigned int>() = 24;
        cout << "value : " << valueTest.get<unsigned int>() << endl;
        cout << "value str : " << valueTest.toString() << endl;
        /*valueTest.set<sf::Vector3f>( sf::Vector3f(24.215, 8.49, 7) );
        cout << "value : " << valueTest.get<float>() << endl;
        cout << "value : " << valueTest.get<sf::Vector3f>().x << " " << valueTest.get<sf::Vector3f>().y << " " << valueTest.get<sf::Vector3f>().z << endl;
        cout << "value str : " << valueTest.toString() << endl;*/

        SuperTest* bonsoir = new SuperTest("Bonjour a tous c'est david la farge pokemon !");

        fge::CallbackHandler<bool>* callbacktest = new fge::CallbackHandler<bool>();
        callbacktest->add( new fge::CallbackFunctor<bool>(monTest) );
        callbacktest->add( new fge::CallbackFunctorObject<SuperTest, bool>(SuperTest::monTestClass, bonsoir), bonsoir );

        callbacktest->call(false);

        delete callbacktest;


        (*superTest.get<sf::RenderWindow*>())->setView( canard );
        window.setView( canard );
        window.setView( window.getDefaultView() );


        fge::Value test2;
        test2.clear();
        test2.addData("je suis un text");
        test2.addData(1242);
        test2.addData(78.12f);
        test2.addData(true);
        test2.addData(sf::Vector2f(9.42f, 12.2f));
        test2.addData(-241);
        test2.addData("ahah");
        //test2 = "|||||||||"+test2.Str()+"|";
        //test2.clear();
        //test2.addData("|||||||||", '|');

        test2.setData(2, ":)");
        test2.setData(3, "elephant");
        test2.setData(0, 4269);
        test2.setData(6, "BWAAAAAAAAAAAAAAAAAu");
        test2.setData(7, "lapin");

        cout << test2.toString() << endl;
        cout << "num " << test2.getDataSize() << endl;
        for (unsigned int i=0; i<test2.getDataSize(); ++i)
        {
            cout << test2.getData(i)->toString() << endl;
        }

        fge::net::PacketBZ2 buffpck;

        ///unsigned int i;
        //fge::Event main_event;
        //sf::RenderWindow window(sf::VideoMode(800, 600), "FastEngine "+(std::string)fge::VERSION_FULLVERSION_STRING);

        fge::texture::Init();
        fge::font::Init();
        fge::timer::Init();
        fge::anim::Init();

        fge::texture::LoadFromFile("pouet1", "test/anim/p1.png");
        fge::texture::LoadFromFile("pouet2", "test/anim/p2.png");
        fge::texture::LoadFromFile("light", "test/light_test.png");
        fge::texture::LoadFromFile("arrow", "arrow.png");
        fge::font::LoadFromFile("base", "comic.ttf");
        if ( fge::anim::LoadFromFile("hhh", "test/anim/anim_data.json") )
        {
            std::cout << "YEAH anim loaded !" << endl;
        }
        fge::crash::Init(window, *fge::font::GetFont("base")->_font);
//*fge::texture::GetData("pouet1")._texture = *fge::texture::GetData(FGE_TEXTURE_BAD)._texture;
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

        this->newObject( new fge::ObjSprite("pouet1", sf::Vector2f(100, 100)) );


        ///window.setFramerateLimit(60);
        sf::RectangleShape blup;
        fge::ObjectDataShared buff_ticket;
        std::vector<unsigned int> buff_vec_ticket;

        fge::ObjTextInputBox* buff_inbox;
        fge::ObjSelectBox* buff_selbox;
        fge::ObjSwitch* buff_switch;
        fge::ObjButton* buff_button;
        fge::ObjText* buff_text;
        fge::ObjAnimation* buff_anim;

        this->setName("SuperScene - 9000");

        buff_inbox = new fge::ObjTextInputBox("base", 20, sf::Vector2f(20, 200));
        buff_inbox->setBoxOutlineColor(sf::Color::Blue);
        buff_inbox->setTextColor(sf::Color::Yellow);
        buff_inbox->setBoxColor(sf::Color::Red);
        buff_inbox->setMaxLength(10);
        buff_inbox->setCharacterSize(12);
        buff_inbox->setHideTextFlag(false);
        buff_ticket = this->newObject(buff_inbox, FGE_SCENE_PLAN_MIDDLE);

        this->setObjectPlan(buff_ticket->getSid(), 1);
        this->setObjectPlan(buff_ticket->getSid(), 1);
        this->setObjectPlan(buff_ticket->getSid(), 1);
        this->setObjectPlan(buff_ticket->getSid(), 4);
        this->setObjectPlan(buff_ticket->getSid(), 3);
        this->setObjectPlan(buff_ticket->getSid(), 2);
        this->setObjectPlan(buff_ticket->getSid(), 0);
        this->setObjectPlan(buff_ticket->getSid(), 25);
        this->setObjectPlan(buff_ticket->getSid(), 14);
        this->setObjectPlan(buff_ticket->getSid(), 1);

        buff_ticket = this->duplicateObject(buff_ticket->getSid());
        buff_inbox = (fge::ObjTextInputBox*)buff_ticket->getObject();
        buff_inbox->setPosition(20, 240);

        buff_selbox = new fge::ObjSelectBox("base", sf::Vector2f(20, 360));
        buff_selbox->getTextList().push_back("Cool");
        buff_selbox->getTextList().push_back("Pas cool");
        buff_selbox->getTextList().push_back("Nul a chier");
        buff_selbox->getTextList().push_back("Vraiment merdique");
        buff_selbox->setBoxSize( buff_selbox->getBoxSize()+sf::Vector2f(100,0) );
        buff_selbox->setSelectedText("Pas cool");
        buff_ticket = this->newObject(buff_selbox, FGE_SCENE_PLAN_MIDDLE);
        //fge::reg::SaveObject("test/objtst.json", buff_selbox);

        buff_switch = new fge::ObjSwitch( "pouet2", "pouet1", sf::Vector2f(400,200) );
        buff_switch->setColor(sf::Color::Yellow);
        buff_button = new fge::ObjButton( "pouet2", "pouet1", sf::Vector2f(400,100) );
        buff_button->setScale(2,2);
        this->newObject(buff_button, FGE_SCENE_PLAN_MIDDLE);
        buff_ticket = this->newObject(buff_switch, FGE_SCENE_PLAN_MIDDLE);

        auto start = std::chrono::steady_clock::now();
        /*for (unsigned int i=0; i<20000; ++i)
        {
            this->duplicateObject(buff_button->_myObjectData->getSid());
        }*/
        auto stop = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Create time : " << duration.count() << "ms" << std::endl;


        this->watchEvent(true);
        buff_text = new fge::ObjText("base");
        buff_text->setCharacterSize(16);
        buff_text->setPosition(10, 300);
        buff_text->_tags.add("info");
        buff_text->setFillColor(sf::Color::Black);
        buff_ticket = this->newObject(buff_text, FGE_SCENE_PLAN_MIDDLE);

        this->watchEvent(false);

        ///fge::texture::Load("perso", "human/sc_player_human_stF_0.png");


        for ( unsigned int i=0; i<40; ++i )
        {
            //fge::timer::Create("testtimeA-"+fge::string::ToStr(i), fge::Timer( sf::seconds(fge::__random.range<uint32_t>(1,10)) ), false);
            //fge::timer::Create("testtimeB-"+fge::string::ToStr(i), fge::Timer( sf::seconds(fge::__random.range<uint32_t>(1,3)) ), true);
        }
        //fge::timer::Create("pouet", fge::Timer( sf::seconds(6) ), false);

        buff_anim = new fge::ObjAnimation(fge::Animation("hhh", "just_a_test"));
        buff_anim->getAnimation().setLoop(true);
        buff_anim->setTickDuration( std::chrono::milliseconds(1) );
        buff_anim->getAnimation().setGroup("ewwevwev");
        //buff_anim->setPosition(200, 200);
        //buff_anim->_syncToFps = false;
        buff_ticket = this->newObject(buff_anim, FGE_SCENE_PLAN_MIDDLE);

        buff_ticket = this->duplicateObject(buff_ticket->getSid());
        buff_ticket->getObject()->move(40, 0);


        blup.setFillColor(sf::Color(0,255,0));
        blup.setSize( sf::Vector2f(50,50) );
        blup.setOutlineColor( sf::Color(0,0,0) );
        blup.setOutlineThickness(1);

        std::string buff_str;
        fge::net::Packet buff_receive;

        buff_text->setString("BONJOUR JE SUIS UN TEXT");


        //fge::reg::SaveScene("test/myscene.json", *this);
        //fge::reg::LoadScene("test/myscene.json", *this);


        fge::Scene* super_scene = this;


        ///buff_ticket = super_scene1.newObject(new Personnage(), FGE_SCENE_PLAN_TOP);

        ///fge::NetValueBase* patate = new fge::NetValue<unsigned int>(NULL, 0);

        /*fge::Scene* super_scene;
        super_scene = fge::reg::LoadScene("test/myscene");
        fge::reg::SaveScene("test/myscene_result", *super_scene);*/
        this->_globalData["Lapin0"] = 231;
        this->_globalData["Lapin1"] = 0.213f;
        this->_globalData["Lapin2"] = 42;
        this->_globalData["Lapin3"] = "Bonjour je suis un lapin !";
        /*fge::reg::SaveScene("test/myscene_result", super_scene1);
        super_scene = fge::reg::LoadScene("test/myscene_result");
        fge::reg::SaveScene("test/myscene_result2", *super_scene);*/

        for (int i=0; i<30; ++i)
        {
            double bba = fge::__random.rand<double>();
            cout << bba << " | " << fge::string::ToStr( bba ) << " | " << fge::string::ToDouble( fge::string::ToStr( bba ) ) << endl;
        }

        /*fge::LightSystem* ls = new fge::LightSystem();
        fge::Light* light1 = new fge::Light("light", sf::Color::Yellow, sf::Vector2f(300, 300), 160.0f);
        light1->setOrigin(160, 160);
        light1->setScale(2.0f, 2.0f);
        light1->setPosition(1350,300);
        light1->refreshTextureSize();
        fge::Light* light2 = new fge::Light("light", sf::Color::Red, sf::Vector2f(350, 350), 160.0f);
        light2->setOrigin(160, 160);
        light2->setScale(2.0f, 2.0f);
        light2->refreshTextureSize();
        this->newObject( light1, FGE_SCENE_PLAN_HIGH_TOP )->getSid();
        this->newObject( light2, FGE_SCENE_PLAN_HIGH_TOP );
        this->addCmd("updateTxt", this,FGE_CMD_FUNC(this->cmd_updateTxt));

        ls->addGate(&light1->_lightgate, true);
        ls->addGate(&light2->_lightgate, true);*/

        Bloc* thebloc = new Bloc();
        this->newObject( thebloc, 0 );
        //ls->addGate(&thebloc->_lightgate, false);

        cout << "My checksum : " << fge::net::GetSceneChecksum(*this) << endl;

        //fge::timer::RestartClock();
        sf::Clock time_test;
        sf::Clock time_fps;
        unsigned int count_fps=0;
        unsigned int count_maxfps=0;

        /*fge::log::SetDefaultFolder("test/");

        fge::log::Clean("log_test.log");
        fge::log::Write("log_test.log", "hello world !", "info");
        fge::log::Write("log_test.log", "hello world !");*/
        //sf::View* boby = this->getCustomView().get();

        //fge::texture::Unload("pouet2");

        //fge::reg::SaveScene("test/myscene.json", *this);
        /*try
        {
            fge::reg::LoadScene("test/myscene.json", *this);
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
            return;
        }*/
        //fge::reg::SaveScene("test/myscene2.json", *this);
        //this->saveInFile("test/myscene.json");
        //this->loadFromFile("test/myscene.json");

        /*
        1129 normal
        562  bzip2
        597  lz4
        546  lz4hc
        */

        main_event._onTextEntered.add( new fge::CallbackFunctor(TestPrint) );

        /*for (fge::ObjectContainer::iterator it = this->begin(); it!=this->end(); ++it)
        {
            std::cout << "SID: " << (*it)->getSid() << " PLAN: " << (*it)->getPlan() << std::endl;
        }

        this->setObjectPlan(justt, 2);
        std::cout << "-------------------------------" << std::endl;

        for (fge::ObjectContainer::iterator it = this->begin(); it!=this->end(); ++it)
        {
            std::cout << "SID: " << (*it)->getSid() << " PLAN: " << (*it)->getPlan() << std::endl;
        }

        this->delObject(justt);
        std::cout << "-------------------------------" << std::endl;

        for (fge::ObjectContainer::iterator it = this->begin(); it!=this->end(); ++it)
        {
            std::cout << "SID: " << (*it)->getSid() << " PLAN: " << (*it)->getPlan() << std::endl;
        }*/

        /*{
            fge::net::Packet pck;
            this->pack(pck);
            fge::net::WritePacketDataToFile(pck, "testPacket");
        }
        {
            fge::net::PacketLZ4 pck;
            this->pack(pck);
            fge::net::WritePacketDataToFile(pck, "testPacketLZ4");
        }
        {
            fge::net::PacketLZ4HC pck;
            this->pack(pck);
            fge::net::WritePacketDataToFile(pck, "testPacketLZ4HC");
        }
        {
            fge::net::PacketBZ2 pck;
            this->pack(pck);
            fge::net::WritePacketDataToFile(pck, "testPacketBZ2");
        }*/

        sf::Clock clockTest;
        fge::Clock clockTest2;



        fge::timer::Create( fge::Timer(std::chrono::milliseconds(1000)) )->_onTimeReached.add( new fge::CallbackFunctor<fge::Timer&>(TestPrintClock) );
        fge::timer::Notify();

        //https://ncase.me/sight-and-light/
        //https://pvigier.github.io/2019/07/28/vagabond-2d-light-system.html

        //thebloc->_test = new fge::ObjRenderMap();
        /*bool montest = thebloc->_test->_lightSystem.addGate(&thebloc->_lightgate);
        thebloc->_test->setClearColor(sf::Color(120,120,120,255));
        this->newObject( thebloc->_test, 1 );*/

        fge::LightSystem ls;

        fge::ObjLight* testLight = new fge::ObjLight("light", sf::Vector2f(300,300));
        testLight->setColor(sf::Color::Red);
        testLight->setScale(3, 3);
        this->newObject( testLight, 0 );
        testLight->setLightSystem(ls);

        testLight = new fge::ObjLight("light", sf::Vector2f(200,200));
        testLight->setColor(sf::Color::Yellow);
        testLight->setScale(2, 2);
        this->newObject( testLight, 0 );
        testLight->setLightSystem(ls);

        thebloc->setLightSystem(&ls);

        sf::View testView = window.getView();
        testView.setRotation(0);
        testView.zoom(1.0f);
        window.setView(testView);

        for (fge::ObjectContainer::const_iterator it = this->cbegin(); it != this->cend(); ++it)
        {
            std::cout << "there is a " << (*it)->getObject()->getClassName() << " in this scene !" << std::endl;
        }

        fge::Clock deltaTime;

        while (window.isOpen())
        {
            //std::cout << "yes " << buff_anim->_anim.getFrame() << std::endl;

            main_event.process(window);

            if ( main_event.isEventType( sf::Event::Closed ) )
            {
                window.close();
            }

            auto deltaTimeDuration = deltaTime.restart();

            super_scene->update( window, main_event, std::chrono::duration_cast<std::chrono::milliseconds>(deltaTimeDuration));
            ticks++;
            //std::cout << "tick " << ticks << std::endl;
            std::chrono::duration<float> fsec = deltaTimeDuration;
            blup.setPosition( fge::ReachVector( blup.getPosition(), sf::Vector2f(400,234), 200, fsec.count() ) );
            blup.setRotation( fge::ReachRotation( blup.getRotation(), 280, 40, fsec.count(), fge::TurnMode::TURN_CLOCKWISE ) );
            blup.setFillColor( fge::SetAlpha( blup.getFillColor(), fge::ReachValue<uint8_t>(blup.getFillColor().a, 0, 40, fsec.count()) ) );

            if ( false )
            {
                //super_scene->delObject( super_scene->getDid_ByClass("OBJ:PERSONNAGE") );
                //fge::anim::Uninit();
                //throw std::logic_error("zut :(");
                try
                {

                    //fge::crash::Crash(window, "Unknown error, please contact support ! "+fge::string::ToStr(time_test.getElapsedTime().asMicroseconds())+" timer : " +fge::string::ToStr(fge::timer::GetNumOfTimer()), fge::font::Get("base"));
                }
                catch (std::exception& e)
                {
                    cout << "Argh ! dsl mec ! " << e.what();
                }
                /*system("cls");
                cout << "ID : 0" << endl;
                cout << (*super_scene)[fge::Ticket(0)].getInfo() << endl;*/
            }

            //buff_text = (fge::ObjText*)(*super_scene)[ *super_scene->getFirstObj_ByTag("info") ]->getObject();

            ++count_fps;
            if ( time_fps.getElapsedTime().asMilliseconds() >= 1000 )
            {
                if (count_fps > count_maxfps)
                {
                    count_maxfps = count_fps;
                }
                //buff_text->setString( "FPS : "+fge::string::ToStr(count_fps)+" max FPS : "+fge::string::ToStr(count_maxfps) );
                std::cout << "FPS : "+fge::string::ToStr(count_fps)+" max FPS : "+fge::string::ToStr(count_maxfps) << std::endl;

                //std::cout << "CLOCK : " << clockTest.restart().asMicroseconds() << std::endl;

                //std::cout << "CLOCK FGE : " << clockTest2.restart<std::chrono::microseconds>() << std::endl;

                /*std::cout << main_event.getBinaryKeysString() << std::endl;
                std::cout << main_event.getBinaryMouseButtonsString() << std::endl;
                std::cout << main_event.getBinaryTypesString() << std::endl;
                std::cout << main_event.getWindowSize().x << " " << main_event.getWindowSize().y << std::endl;*/
                count_fps=0;
                time_fps.restart();
                //cout << std::numeric_limits<double>::digits << " " << std::numeric_limits<double>::digits10 << endl;

                /**
                Test 1 (pc stage) : ~3391 fps
                Test 2 (pc stage) : ~3490 fps (pareil)
                Test 3 (pc stage) : ~3474 fps (pareil)
                **/
            }

            ///buff_text->_text = "RefreshLigh1("+fge::string::ToStr(light1->isUpdateable())+") RefreshLigh2("+fge::string::ToStr(light2->isUpdateable())+")";//_supertext;
            /*"POS["+ fge::string::ToStr(blup.getPosition().x) +" "+
            fge::string::ToStr(blup.getPosition().y) +
            "] ANG["+ fge::string::ToStr(blup.getRotation()) +"] ALPHA["+fge::string::ToStr(blup.getFillColor().a)+"]";*/



            super_scene->draw( window );
            window.draw( blup );

            window.display();
        }

        //fge::reg::Uninit();
        fge::timer::Uninit();
        fge::texture::Uninit();
        fge::font::Uninit();
        //fge::timer::DestroyAll();
        fge::anim::Uninit();
        //test.stop();
    }
};

int main(int argc, char *argv[])
{
    mainScene mscene;
    mscene.main();

    return 0;
}
