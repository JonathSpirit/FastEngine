#ifndef _FGE_C_LIGHTSYSTEM_HPP_INCLUDED
#define _FGE_C_LIGHTSYSTEM_HPP_INCLUDED

#include <FastEngine/C_lightObstacle.hpp>
#include <FastEngine/C_tunnel.hpp>

namespace fge
{

using LightSystem = fge::Tunnel<fge::LightObstacle>;
using LightSystemGate = fge::TunnelGate<fge::LightObstacle>;

}//end fge

#endif // _FGE_C_LIGHTSYSTEM_HPP_INCLUDED
