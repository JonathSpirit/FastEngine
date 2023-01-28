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

#include "FastEngine/graphic/C_circleShape.hpp"
#include <cmath>

namespace fge
{

CircleShape::CircleShape(float radius, std::size_t pointCount) :
        g_radius(radius),
        g_pointCount(pointCount)
{
    this->update();
}

void CircleShape::setRadius(float radius)
{
    this->g_radius = radius;
    this->update();
}

float CircleShape::getRadius() const
{
    return this->g_radius;
}

void CircleShape::setPointCount(std::size_t count)
{
    this->g_pointCount = count;
    this->update();
}
std::size_t CircleShape::getPointCount() const
{
    return this->g_pointCount;
}

Vector2f CircleShape::getPoint(std::size_t index) const
{
    static const float pi = 3.141592654f;

    const float angle = static_cast<float>(index) * 2.f * pi / static_cast<float>(g_pointCount) - pi / 2.f;
    const float x = std::cos(angle) * g_radius;
    const float y = std::sin(angle) * g_radius;

    return {g_radius + x, g_radius + y};
}

} // namespace fge
