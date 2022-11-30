#include <C_food.hpp>
#include <FastEngine/extra/extra_function.hpp>
#include <FastEngine/C_random.hpp>

namespace ls
{

Food::Food(const sf::Vector2f& pos)
{
    this->setPosition(pos);
}

void Food::first([[maybe_unused]] fge::Scene* scene)
{
    this->_nutrition = fge::_random.range(1, 20);
    this->setOrigin({24, 19});

    this->networkRegister();
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(Food)
{
    sf::CircleShape food(8);
    food.setFillColor(sf::Color::Green);
    food.setOutlineColor(sf::Color::Black);
    food.setOutlineThickness(1.0f);
    food.setPosition(this->getPosition());
    food.setOrigin(8,8);

    target.draw(food);
}
#endif

void Food::networkRegister()
{
    this->_netList.clear();
    this->_netList.push( new fge::net::NetworkType<sf::Vector2f>({&this->getPosition(), [&](const sf::Vector2f& pos){this->setPosition(pos);}}) );
}

void Food::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);
}
void Food::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);
}
void Food::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);
    pck << this->_nutrition;
}
void Food::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);
    pck >> this->_nutrition;
}

const char* Food::getClassName() const
{
    return "LS:OBJ:FOOD";
}
const char* Food::getReadableClassName() const
{
    return "food";
}

}//end ls
