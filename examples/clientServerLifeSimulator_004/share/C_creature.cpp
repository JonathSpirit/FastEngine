#include <C_creature.hpp>
#include <C_drink.hpp>
#include <C_food.hpp>
#include <FastEngine/C_random.hpp>
#include <FastEngine/C_scene.hpp>
#include <FastEngine/extra/extra_function.hpp>
#include <FastEngine/manager/audio_manager.hpp>

namespace ls
{

CreatureData::CreatureData()
{
    this->_lifePoint = 100;
    this->_gender = (fge::_random.range(0, 1) == 0) ? CreatureGender::GENDER_FEMALE : CreatureGender::GENDER_MALE;
    this->_hunger = 20;
    this->_thirst = 20;
    this->_libido = 0;
    this->_libidoAdd = fge::_random.range(0, 10);
    this->_pregnant = false;
    this->_energy = 60;
    this->_height = fge::_random.range(10, 100);
    this->_muscularMass = fge::_random.range(1, 100);
    this->_bodyFat = fge::_random.range(1, 100);
    this->_sightRadius = fge::_random.range(10.0f, 60.0f);
}

void CreatureData::networkRegister(fge::net::NetworkTypeContainer& netList)
{
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_lifePoint});
    netList.push(new fge::net::NetworkType<std::underlying_type_t<CreatureGender>>{
            reinterpret_cast<std::underlying_type_t<CreatureGender>*>(&this->_gender)});
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_hunger});
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_thirst});
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_libido});
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_libidoAdd});
    netList.push(new fge::net::NetworkType<bool>{&this->_pregnant});
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_energy});
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_height});
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_muscularMass});
    netList.push(new fge::net::NetworkType<uint8_t>{&this->_bodyFat});
    netList.push(new fge::net::NetworkType<float>{&this->_sightRadius});
}

fge::net::Packet& operator<<(fge::net::Packet& pck, const CreatureData& data)
{
    return pck << data._lifePoint << static_cast<std::underlying_type_t<CreatureGender>>(data._gender) << data._hunger
               << data._thirst << data._libido << data._libidoAdd << data._energy << data._height << data._muscularMass
               << data._bodyFat << data._sightRadius;
}
const fge::net::Packet& operator>>(const fge::net::Packet& pck, CreatureData& data)
{
    return pck >> data._lifePoint >> reinterpret_cast<std::underlying_type_t<CreatureGender>&>(data._gender) >>
           data._hunger >> data._thirst >> data._libido >> data._libidoAdd >> data._energy >> data._height >>
           data._muscularMass >> data._bodyFat >> data._sightRadius;
}

Creature::Creature(const sf::Vector2f& pos)
{
    this->setPosition(pos);
}

void Creature::first([[maybe_unused]] fge::Scene* scene)
{
#ifndef FGE_DEF_SERVER
    this->_anim = "ugandan";
    this->_anim.setGroup("speak");
    this->_anim.setLoop(true);
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

    this->_font = "default";
    _speakSound.setBuffer(*fge::audio::GetAudio("ugandan1")->_audio);
    this->_speakDelay = sf::milliseconds(fge::_random.range<int>(6000, 20000));
#endif // FGE_DEF_SERVER

    this->setOrigin({24, 19});
    this->networkRegister();
}
bool Creature::worldTick()
{
    this->_data._hunger = std::clamp(this->_data._hunger + 10, 0, 100);
    if (this->_data._hunger >= 100)
    {
        this->_data._lifePoint = std::clamp(this->_data._lifePoint - 5, 0, 100);
        if (this->_data._lifePoint == 0)
        {
            return true;
        }
    }

    this->_data._thirst = std::clamp(this->_data._thirst + 10, 0, 100);
    if (this->_data._thirst >= 100)
    {
        this->_data._lifePoint = std::clamp(this->_data._lifePoint - 5, 0, 100);
        if (this->_data._lifePoint == 0)
        {
            return true;
        }
    }

    this->_data._libido = std::clamp(this->_data._libido + this->_data._libidoAdd, 0, 100);

    if (this->_data._pregnant)
    {
        if (this->_timePregnant >= std::chrono::milliseconds(30000))
        {
            this->_data._pregnant = false;
            auto* scene = this->_myObjectData.lock()->getLinkedScene();
            for (std::size_t i = 0; i < fge::_random.range(1, 2); ++i)
            {
                scene->newObject(FGE_NEWOBJECT(ls::Creature, this->getPosition()), FGE_SCENE_PLAN_MIDDLE);
            }
        }
    }

    return false;
}

FGE_OBJ_UPDATE_BODY(Creature)
#ifdef FGE_DEF_SERVER
{
    bool finishMoving = this->updateMoveable(*this, deltaTime);

    if (this->_actionQueue.empty())
    { //No action in queue
        this->_timeRandomMove += deltaTime;

        if (this->_timeRandomMove >= std::chrono::milliseconds{2000})
        { //Random move
            this->_timeRandomMove = std::chrono::milliseconds{0};
            this->setTargetPos(ls::GetRandomPositionFromCenter(this->getPosition(), 200.0f));
        }

        if (this->_data._hunger >= 10)
        { //Finding food
            fge::ObjectContainer objects;
            if (scene->getAllObj_ByClass("LS:OBJ:FOOD", objects) > 0)
            {
                for (auto& obj: objects)
                {
                    float distance = fge::GetDistanceBetween(this->getPosition(), obj->getObject()->getPosition());
                    if (distance <= this->_data._sightRadius)
                    {
                        this->_actionQueue.push({Action::Types::ACTION_EAT, obj->getSid()});
                        this->setTargetPos(obj->getObject()->getPosition());
                        break;
                    }
                }
            }
        }
        if (this->_data._thirst >= 10)
        { //Finding drink
            fge::ObjectContainer objects;
            if (scene->getAllObj_ByClass("LS:OBJ:DRINK", objects) > 0)
            {
                for (auto& obj: objects)
                {
                    float distance = fge::GetDistanceBetween(this->getPosition(), obj->getObject()->getPosition());
                    if (distance <= this->_data._sightRadius)
                    {
                        this->_actionQueue.push({Action::Types::ACTION_DRINK, obj->getSid()});
                        this->setTargetPos(obj->getObject()->getPosition());
                        break;
                    }
                }
            }
        }
        if (this->_data._libido >= 50)
        { //Finding partner
            fge::ObjectContainer objects;
            if (scene->getAllObj_ByClass("LS:OBJ:CREATURE", objects) > 0)
            {
                for (auto& obj: objects)
                {
                    auto* creature = obj->getObject<ls::Creature>();

                    if (creature != this)
                    {
                        float distance = fge::GetDistanceBetween(this->getPosition(), creature->getPosition());
                        if (distance <= this->_data._sightRadius)
                        {
                            if (creature->_data._libido >= 50)
                            {
                                if (!this->_data._pregnant && !creature->_data._pregnant)
                                {
                                    if (creature->_data._gender ==
                                        ((this->_data._gender == ls::CreatureGender::GENDER_FEMALE)
                                                 ? ls::CreatureGender::GENDER_MALE
                                                 : ls::CreatureGender::GENDER_FEMALE))
                                    { //Opposite gender
                                        this->_actionQueue.push({Action::Types::ACTION_MAKEBABY, obj->getSid()});
                                        this->setTargetPos(creature->getPosition());
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    { //Pending action
        auto targetObject = scene->getObject(this->_actionQueue.front()._target);

        if (targetObject)
        {
            if (fge::GetDistanceBetween(this->_g_targetPos, targetObject->getObject()->getPosition()) >= 10.0f)
            {
                this->setTargetPos(targetObject->getObject()->getPosition());
                finishMoving = false;
            }

            if (finishMoving)
            { //Target reached
                switch (this->_actionQueue.front()._type)
                {
                case Action::Types::ACTION_EAT:
                {
                    auto nutrition = targetObject->getObject<ls::Food>()->_nutrition;
                    this->_data._hunger = std::clamp(this->_data._hunger - nutrition, 0, 100);
                    scene->delObject(targetObject->getSid());
                }
                break;
                case Action::Types::ACTION_DRINK:
                {
                    auto nutrition = targetObject->getObject<ls::Drink>()->_nutrition;
                    this->_data._thirst = std::clamp(this->_data._thirst - nutrition, 0, 100);
                    scene->delObject(targetObject->getSid());
                }
                break;
                case Action::Types::ACTION_MAKEBABY:
                {
                    auto* creature = targetObject->getObject<ls::Creature>();

                    //Already pregnant
                    if (creature->_data._pregnant || this->_data._pregnant)
                    {
                        break;
                    }

                    this->_data._libido = 0;

                    creature->_data._libido = 0;
                    if (creature->_data._gender == ls::CreatureGender::GENDER_FEMALE)
                    {
                        creature->_data._pregnant = true;
                    }
                    this->_timePregnant = std::chrono::milliseconds{0};
                }
                break;
                }

                this->_actionQueue.pop();
            }
            /*else
            {
                if (this->_timeRandomMove >= std::chrono::milliseconds{500})
                {
                    this->_timeRandomMove = std::chrono::milliseconds{0};
                    this->setTargetPos( targetObject->getObject()->getPosition() );
                }
            }*/
        }
        else
        {
            this->_actionQueue.pop();
        }
    }

    if (this->_data._pregnant)
    {
        this->_timePregnant += deltaTime;
    }
}
#else
{
    this->_timeAnimation += deltaTime;

    if (this->_timeAnimation >= std::chrono::milliseconds{this->_anim.getFrame()->_ticks})
    {
        this->_timeAnimation = std::chrono::milliseconds{0};
        this->_anim.nextFrame();
    }

    if (this->_speakClock.getElapsedTime() >= this->_speakDelay)
    {
        this->_speakClock.restart();
        this->_speakDelay = sf::milliseconds(fge::_random.range(6000, 50000));
        _speakSound.setBuffer(*fge::audio::GetAudio("ugandan" + fge::string::ToStr(fge::_random.range(1, 2)))->_audio);

        this->_speakSound.play();
    }

    this->updateMoveable(*this, deltaTime);
}
#endif //FGE_DEF_SERVER

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(Creature)
{
    //creature
    sf::Sprite creature;
    creature.setPosition(this->getPosition());
    creature.setOrigin(this->getOrigin());
    /*
    100|val
    ---|---
    2.0| ?
    */
    float scale = (2.0f * this->_data._height) / 100;
    creature.setScale(scale, scale);
    creature.setTexture(this->_anim);

    //sightRadius
    sf::CircleShape sight(this->_data._sightRadius);
    sight.setOutlineThickness(1.0f);
    sight.setOutlineColor(sf::Color(120, 120, 120, 180));
    sight.setFillColor(sf::Color::Transparent);
    sight.setPosition(this->getPosition());
    sight.setOrigin(this->_data._sightRadius, this->_data._sightRadius);

    //Gender
    sf::Text txtGender(std::string(this->_data._gender == ls::CreatureGender::GENDER_MALE ? "male" : "female") +
                               (this->_data._pregnant ? " pregnant" : ""),
                       this->_font, 12);
    txtGender.setPosition(this->getPosition().x - 20, this->getPosition().y + 30);
    txtGender.setOutlineThickness(1.0f);
    txtGender.setOutlineColor(sf::Color::Black);
    txtGender.setFillColor(sf::Color::White);

    //Stat
    /*
    unsigned char _lifePoint;
    unsigned char _hunger;
    unsigned char _thirst;
    unsigned char _libido;
    */
    sf::RectangleShape infoBox(sf::Vector2f(100, 20));
    infoBox.setPosition(this->getPosition() + sf::Vector2f(-20, -40));
    infoBox.setOutlineThickness(1.0f);
    infoBox.setOutlineColor(sf::Color::Black);
    infoBox.setFillColor(sf::Color(100, 100, 100));
    sf::RectangleShape barLife(sf::Vector2f(this->_data._lifePoint, 5));
    barLife.setPosition(this->getPosition() + sf::Vector2f(-20, -40));
    barLife.setFillColor(sf::Color::Red);
    sf::RectangleShape barHunger(sf::Vector2f(this->_data._hunger, 5));
    barHunger.setPosition(this->getPosition() + sf::Vector2f(-20, -35));
    barHunger.setFillColor(sf::Color(255, 154, 29));
    sf::RectangleShape barThirst(sf::Vector2f(this->_data._thirst, 5));
    barThirst.setPosition(this->getPosition() + sf::Vector2f(-20, -30));
    barThirst.setFillColor(sf::Color::Blue);
    sf::RectangleShape barLibido(sf::Vector2f(this->_data._libido, 5));
    barLibido.setPosition(this->getPosition() + sf::Vector2f(-20, -25));
    barLibido.setFillColor(sf::Color(228, 0, 255));

    target.draw(sight);

    target.draw(txtGender);

    target.draw(infoBox);
    target.draw(barLife);
    target.draw(barHunger);
    target.draw(barThirst);
    target.draw(barLibido);

    target.draw(creature);
}
#endif

void Creature::networkRegister()
{
    this->_netList.clear();

    this->_netList.push(new fge::net::NetworkTypeSmoothVec2Float{
            {&this->getPosition(), [&](const sf::Vector2f& pos) { this->setPosition(pos); }},
            100.0f});
    this->_netList.push(new fge::net::NetworkType<sf::Vector2f>{&this->_g_targetPos})
            ->_onApplied.add(
                    new fge::CallbackLambda<>{[&]() { this->_g_finish = this->getPosition() == this->_g_targetPos; }},
                    this);
    this->_netList.push(new fge::net::NetworkType<bool>{&this->_g_finish})
            ->_onApplied.add(new fge::CallbackLambda<>{[&]() {
                                 if (this->_g_finish)
                                 {
                                     this->setPosition(this->_g_targetPos);
                                 }
                             }},
                             this);

    this->_data.networkRegister(this->_netList);
}

void Creature::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);
}
void Creature::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);
}
void Creature::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);
    pck << this->_data;
}
void Creature::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);
    pck >> this->_data;
}

const char* Creature::getClassName() const
{
    return "LS:OBJ:CREATURE";
}
const char* Creature::getReadableClassName() const
{
    return "creature";
}

} // namespace ls
