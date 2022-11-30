#ifndef _EXFGE_C_MOVEABLE_HPP_INCLUDED
#define _EXFGE_C_MOVEABLE_HPP_INCLUDED

#include <FastEngine/object/C_object.hpp>

namespace ls
{

class Moveable
{
public:
    Moveable() = default;
    ~Moveable() = default;

    void setTargetPos(const sf::Vector2f& pos);
    bool updateMoveable(sf::Transformable& transformable, const std::chrono::milliseconds& deltaTime);

protected:
    sf::Vector2f _g_targetPos;
    bool _g_finish{true};
};

}//end ls

#endif // _EXFGE_C_MOVEABLE_HPP_INCLUDED
