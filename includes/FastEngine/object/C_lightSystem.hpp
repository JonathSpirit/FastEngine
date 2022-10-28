/*
 * Copyright 2022 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FGE_C_LIGHTSYSTEM_HPP_INCLUDED
#define _FGE_C_LIGHTSYSTEM_HPP_INCLUDED

#include "FastEngine/C_tunnel.hpp"
#include "FastEngine/C_scene.hpp"

#define FGE_LIGHT_PROPERTY_DEFAULT_LS "_fge_def_ls"

namespace fge
{

class LightObstacle;

/**
 * \ingroup graphics
 * \brief An fge::Tunnel class that regroups all the lights and obstacles as a tunnel.
 */
using LightSystem = fge::Tunnel<fge::LightObstacle>;
using LightSystemGate = fge::TunnelGate<fge::LightObstacle>;

/**
 * \brief Get the default light system from a scene property
 * \ingroup graphics
 *
 * \param scene The scene to get the light system from
 * \return The default light system
 */
inline fge::LightSystem* GetDefaultLightSystem(fge::Scene* scene)
{
    if (scene != nullptr)
    {
        return scene->_properties.getProperty(FGE_LIGHT_PROPERTY_DEFAULT_LS).get<fge::LightSystem*>().value_or(nullptr);
    }
    return nullptr;
}

/**
 * \class LightComponent
 * \ingroup graphics
 * \brief A base class that define a light component
 */
class LightComponent
{
public:
    explicit LightComponent(fge::LightObstacle* lightObstacle = nullptr) :
            _g_lightSystemGate(lightObstacle)
    {
    }

    /**
     * \brief Set the light system to be used by this light
     *
     * \param lightSystem The light system to use
     */
    void setLightSystem(fge::LightSystem& lightSystem)
    {
        this->_g_lightSystemGate.openTo(&lightSystem, true);
    }

    /**
     * \brief Retrieve the default light system from a scene
     *
     * \param scene The scene to get the light system from
     */
    void setDefaultLightSystem(fge::Scene* scene)
    {
        auto* ls = fge::GetDefaultLightSystem(scene);
        if (ls != nullptr)
        {
            this->setLightSystem(*ls);
        }
    }

protected:
    fge::LightSystemGate _g_lightSystemGate;
};

/**
 * \class ObstacleComponent
 * \ingroup graphics
 * \brief A base class that define an obstacle component
 */
class ObstacleComponent
{
public:
    explicit ObstacleComponent(fge::LightObstacle* lightObstacle = nullptr) :
            _g_lightSystemGate(lightObstacle)
    {
    }

    /**
     * \brief Set the light system to be used by this light
     *
     * \param lightSystem The light system to use
     */
    void setLightSystem(fge::LightSystem& lightSystem)
    {
        this->_g_lightSystemGate.openTo(&lightSystem, false);
    }

    /**
     * \brief Retrieve the default light system from a scene
     *
     * \param scene The scene to get the light system from
     */
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
