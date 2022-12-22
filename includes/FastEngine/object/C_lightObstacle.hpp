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

#ifndef _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED
#define _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED

#include "C_lightSystem.hpp"
#include "SFML/System/Vector2.hpp"
#include <vector>

namespace fge
{

class ObjLight;

using ListOfPoints = std::vector<sf::Vector2f>;

/**
 * \class LightObstacle
 * \ingroup graphics
 * \brief A base class to define an obstacle for the light system
 *
 * An obstacle is a group of points that define the shape of the object.
 */
class LightObstacle : public fge::ObstacleComponent
{
public:
    LightObstacle() :
            fge::ObstacleComponent(this)
    {}
    LightObstacle(const fge::LightObstacle& r) :
            fge::ObstacleComponent(r),
            _g_myPoints(r._g_myPoints)
    {
        this->_g_lightSystemGate.setData(this);
    }

    fge::LightObstacle& operator=(const fge::LightObstacle& r)
    {
        this->_g_lightSystemGate = r._g_lightSystemGate;
        this->_g_myPoints = r._g_myPoints;
        this->_g_lightSystemGate.setData(this);
        return *this;
    }

protected:
    fge::ListOfPoints _g_myPoints;

    friend class fge::ObjLight;
};

} // namespace fge

#endif // _FGE_C_LIGHTOBSTACLE_HPP_INCLUDED
