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

#include "FastEngine/object/C_objLineShape.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjLineShape::ObjLineShape(const Vector2f& beginning, const Vector2f& end, float thickness) :
        g_direction(end - beginning),
        g_thickness(thickness)
{
    this->setPosition(beginning);
    this->updateShape();
}

void ObjLineShape::setThickness(float thickness)
{
    this->g_thickness = thickness;
    this->updateShape();
}
void ObjLineShape::setEndPoint(const Vector2f& point)
{
    this->g_direction = point - getPosition();
    this->updateShape();
}

float ObjLineShape::getThickness() const
{
    return this->g_thickness;
}
Vector2f ObjLineShape::getEndPoint() const
{
    return this->getPosition() + this->g_direction;
}

float ObjLineShape::getLength() const
{
    return glm::length(this->g_direction);
}

std::size_t ObjLineShape::getPointCount() const
{
    return 4;
}
Vector2f ObjLineShape::getPoint(std::size_t index) const
{
    auto offset = Vector2f();
    if (this->g_direction != Vector2f(0, 0))
    {
        auto vec = glm::normalize(this->g_direction);
        vec = fge::Vector2f{-vec.y, vec.x};
        offset = (this->g_thickness / 2.f) * vec;
    }

    switch (index)
    {
    default:
    case 0:
        return offset;
    case 1:
        return this->g_direction + offset;
    case 2:
        return this->g_direction - offset;
    case 3:
        return -offset;
    }
}

const char* ObjLineShape::getClassName() const
{
    return FGE_OBJLINESHAPE_CLASSNAME;
}
const char* ObjLineShape::getReadableClassName() const
{
    return "line shape";
}

} // namespace fge
