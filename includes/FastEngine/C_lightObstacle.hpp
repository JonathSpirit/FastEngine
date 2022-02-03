#ifndef _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED
#define _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED

#include <SFML/System/Vector2.hpp>
#include <FastEngine/C_tunnel.hpp>
#include <vector>

namespace fge
{

class ObjLight;

using ListOfPoints = std::vector<sf::Vector2f>;

struct LightObstacle
{
protected:
    fge::TunnelGate<fge::LightObstacle> _g_lightSystemGate;

    fge::ListOfPoints _g_myPoints;

    friend class fge::ObjLight;
};

}//end fge

#endif // _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED
