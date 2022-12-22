#include <C_drink.hpp>
#include <FastEngine/C_random.hpp>
#include <FastEngine/extra/extra_function.hpp>

namespace ls
{

Drink::Drink(const sf::Vector2f& pos)
{
    this->setPosition(pos);
}

void Drink::first([[maybe_unused]] fge::Scene* scene)
{
    this->_nutrition = fge::_random.range(1, 20);
    this->setOrigin({24, 19});

    this->networkRegister();
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(Drink)
{
    sf::CircleShape drink(8);
    drink.setFillColor(sf::Color::Blue);
    drink.setOutlineColor(sf::Color::Black);
    drink.setOutlineThickness(1.0f);
    drink.setPosition(this->getPosition());
    drink.setOrigin(8, 8);

    target.draw(drink);
}
#endif

void Drink::networkRegister()
{
    this->_netList.clear();
    this->_netList.push(new fge::net::NetworkType<sf::Vector2f>(
            {&this->getPosition(), [&](const sf::Vector2f& pos) { this->setPosition(pos); }}));
}

void Drink::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);
}
void Drink::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);
}
void Drink::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);
    pck << this->_nutrition;
}
void Drink::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);
    pck >> this->_nutrition;
}

const char* Drink::getClassName() const
{
    return "LS:OBJ:DRINK";
}
const char* Drink::getReadableClassName() const
{
    return "drink";
}

} // namespace ls
