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

#include "FastEngine/object/C_objCircleShape.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include <cmath>

namespace fge
{

ObjCircleShape::ObjCircleShape(float radius, std::size_t pointCount) :
        g_radius(radius),
        g_pointCount(pointCount)
{
    this->updateShape();
}

void ObjCircleShape::setRadius(float radius)
{
    this->g_radius = radius;
    this->updateShape();
}

float ObjCircleShape::getRadius() const
{
    return this->g_radius;
}

void ObjCircleShape::setPointCount(std::size_t count)
{
    this->g_pointCount = count;
    this->updateShape();
}
std::size_t ObjCircleShape::getPointCount() const
{
    return this->g_pointCount;
}

Vector2f ObjCircleShape::getPoint(std::size_t index) const
{
    float const angle =
            static_cast<float>(index) * 2.f * static_cast<float>(FGE_MATH_PI) / static_cast<float>(this->g_pointCount) -
            static_cast<float>(FGE_MATH_PI) / 2.f;
    float const x = std::cos(angle) * this->g_radius;
    float const y = std::sin(angle) * this->g_radius;

    return {this->g_radius + x, this->g_radius + y};
}

char const* ObjCircleShape::getClassName() const
{
    return FGE_OBJCIRCLESHAPE_CLASSNAME;
}
char const* ObjCircleShape::getReadableClassName() const
{
    return "circle shape";
}

} // namespace fge
