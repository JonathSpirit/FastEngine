#ifndef _FGE_C_SMOOTHFLOAT_HPP_INCLUDED
#define _FGE_C_SMOOTHFLOAT_HPP_INCLUDED

#include <SFML/System/Vector2.hpp>

namespace fge
{
namespace net
{

struct SmoothVec2Float
{
    sf::Vector2f _real;
    float _errorRange;

    sf::Vector2f _cache;
};

struct SmoothFloat
{
    float _real;
    float _errorRange;

    float _cache;
};

}//end net
}//end fge

#endif // _FGE_C_SMOOTHFLOAT_HPP_INCLUDED
