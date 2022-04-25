#ifndef _FGE_C_LIGHTSYSTEM_HPP_INCLUDED
#define _FGE_C_LIGHTSYSTEM_HPP_INCLUDED

#include <FastEngine/C_tunnel.hpp>
#include <FastEngine/C_scene.hpp>

#define FGE_LIGHT_PROPERTY_DEFAULT_LS "_fge_def_ls"

namespace fge
{

class LightObstacle;

using LightSystem = fge::Tunnel<fge::LightObstacle>;
using LightSystemGate = fge::TunnelGate<fge::LightObstacle>;

inline fge::LightSystem* GetDefaultLightSystem(fge::Scene* scene)
{
    if (scene)
    {
        return scene->_properties.getProperty(FGE_LIGHT_PROPERTY_DEFAULT_LS).get<fge::LightSystem*>();
    }
    return nullptr;
}

class LightComponent
{
public:
    explicit LightComponent(fge::LightObstacle* lightObstacle = nullptr) :
            _g_lightSystemGate(lightObstacle)
    {
    }

    void setLightSystem(fge::LightSystem& lightSystem)
    {
        this->_g_lightSystemGate.openTo(&lightSystem, true);
    }

    void setDefaultLightSystem(fge::Scene* scene)
    {
        auto* ls = fge::GetDefaultLightSystem(scene);
        if (ls)
        {
            this->setLightSystem(*ls);
        }
    }

protected:
    fge::LightSystemGate _g_lightSystemGate;
};

class ObstacleComponent
{
public:
    explicit ObstacleComponent(fge::LightObstacle* lightObstacle = nullptr) :
            _g_lightSystemGate(lightObstacle)
    {
    }

    void setLightSystem(fge::LightSystem& lightSystem)
    {
        this->_g_lightSystemGate.openTo(&lightSystem, false);
    }

    void setDefaultLightSystem(fge::Scene* scene)
    {
        auto* ls = fge::GetDefaultLightSystem(scene);
        if (ls)
        {
            this->setLightSystem(*ls);
        }
    }

protected:
    fge::LightSystemGate _g_lightSystemGate;
};

}//end fge

#endif // _FGE_C_LIGHTSYSTEM_HPP_INCLUDED
