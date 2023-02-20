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

#include "FastEngine/object/C_objRectangleShape.hpp"

namespace fge
{

ObjRectangleShape::ObjRectangleShape(const Vector2f& size)
{
    this->setSize(size);
}

void ObjRectangleShape::setSize(const Vector2f& size)
{
    this->g_size = size;
    this->updateShape();
}
const Vector2f& ObjRectangleShape::getSize() const
{
    return this->g_size;
}

std::size_t ObjRectangleShape::getPointCount() const
{
    return 4;
}

Vector2f ObjRectangleShape::getPoint(std::size_t index) const
{
    switch (index)
    {
    default:
    case 0:
        return {0.0f, 0.0f};
    case 1:
        return {this->g_size.x, 0.0f};
    case 2:
        return {this->g_size.x, this->g_size.y};
    case 3:
        return {0.0f, this->g_size.y};
    }
}

const char* ObjRectangleShape::getClassName() const
{
    return FGE_OBJRECTANGLESHAPE_CLASSNAME;
}
const char* ObjRectangleShape::getReadableClassName() const
{
    return "rectangle shape";
}

} // namespace fge
