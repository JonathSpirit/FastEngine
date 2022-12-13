#ifndef _EXFGE_CUSTOMOBJECT_HPP_INCLUDED
#define _EXFGE_CUSTOMOBJECT_HPP_INCLUDED

#include <C_moveable.hpp>
#include <FastEngine/object/C_object.hpp>

namespace ls
{

class CustomObject : public fge::Object, public ls::Moveable
{
public:
    CustomObject() = default;
    ~CustomObject() override = default;

    virtual bool worldTick();
};

sf::Vector2f ClampToMapLimit(const sf::Vector2f& position);
sf::Vector2f GetRandomPositionFromCenter(const sf::Vector2f& center, float maxDistance);
sf::Vector2f GetRandomPosition();

} // namespace ls

#endif // _EXFGE_CUSTOMOBJECT_HPP_INCLUDED
