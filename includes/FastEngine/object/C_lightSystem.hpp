/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/C_scene.hpp"
#include "FastEngine/C_tunnel.hpp"
#include "FastEngine/C_concavePolygon.hpp"

#define FGE_LIGHT_PROPERTY_DEFAULT_LS "_fge_def_ls"

namespace fge
{

class LightComponent;

/**
 * \ingroup graphics
 * \brief An fge::Tunnel class that regroups all the lights and obstacles as a tunnel.
 */
using LightSystem = fge::Tunnel<fge::LightComponent>;
using LightSystemGate = fge::TunnelGate<fge::LightComponent>;

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
    inline LightComponent() :
            _g_lightSystemGate(this)
    {}
    inline LightComponent(fge::LightComponent const& r) :
            _g_lightSystemGate(r._g_lightSystemGate)
    {
        this->_g_lightSystemGate.setData(this);
    }
    inline LightComponent(fge::LightComponent&& r) noexcept :
            _g_lightSystemGate(std::move(r._g_lightSystemGate))
    {
        this->_g_lightSystemGate.setData(this);
    }
    virtual ~LightComponent() = default;

    inline LightComponent& operator=(fge::LightComponent const& r)
    {
        this->_g_lightSystemGate = r._g_lightSystemGate;
        this->_g_lightSystemGate.setData(this);
        return *this;
    }
    inline LightComponent& operator=(fge::LightComponent&& r) noexcept
    {
        this->_g_lightSystemGate = std::move(r._g_lightSystemGate);
        this->_g_lightSystemGate.setData(this);
        return *this;
    }

    /**
     * \brief Set the light system to be used by this light
     *
     * \param lightSystem The light system to use
     */
    inline void setLightSystem(fge::LightSystem& lightSystem) { this->_g_lightSystemGate.openTo(lightSystem, !this->isObstacle()); }

    [[nodiscard]] inline virtual bool isObstacle() const { return false; }
    inline virtual void updateObstacleShape() {}

    /**
     * \brief Retrieve the default light system from a scene
     *
     * \param scene The scene to get the light system from
     */
    inline void setDefaultLightSystem(fge::Scene* scene)
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
 * \class LightObstacle
 * \ingroup graphics
 * \brief A base class to define an obstacle for the light system
 *
 * An obstacle is a group of points that define the shape of the object.
 */
class LightObstacle : public fge::LightComponent
{
public:
    inline explicit LightObstacle(fge::Transformable const* transformableParent) :
            g_transformableParent(transformableParent)
    {}
    inline LightObstacle(fge::LightObstacle const& r, fge::Transformable const* transformableParent) :
            fge::LightComponent(r),
            g_transformableParent(transformableParent)
    {}
    inline LightObstacle(fge::LightObstacle&& r, fge::Transformable const* transformableParent) noexcept :
            fge::LightComponent(std::move(static_cast<fge::LightComponent&&>(r))),
            g_transformableParent(transformableParent)
    {}
    inline LightObstacle(fge::LightObstacle const& r) = delete;
    inline LightObstacle(fge::LightObstacle&& r) noexcept = delete;
    inline ~LightObstacle() override = default;

    inline fge::LightObstacle& operator=(fge::LightObstacle const& r)
    {
        LightComponent::operator=(static_cast<fge::LightComponent const&>(r));
        return *this;
    }
    inline fge::LightObstacle& operator=(fge::LightObstacle&& r) noexcept
    {
        LightComponent::operator=(std::move(static_cast<fge::LightComponent&&>(r)));
        return *this;
    }

    [[nodiscard]] inline bool isObstacle() const final { return true; }

    [[nodiscard]] inline fge::Transformable const& getTransformableParent() const { return *this->g_transformableParent; }

    [[nodiscard]] inline fge::ConcavePolygon const& getShape() const { return this->_g_shape; }

protected:
    fge::ConcavePolygon _g_shape;

private:
    fge::Transformable const* g_transformableParent;
};

} // namespace fge

#endif // _FGE_C_LIGHTSYSTEM_HPP_INCLUDED
