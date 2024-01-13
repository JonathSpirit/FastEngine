/*
 * Copyright 2024 Guillaume Guillet
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

#ifndef _FGE_C_OBJLINESHAPE_HPP_INCLUDED
#define _FGE_C_OBJLINESHAPE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/object/C_objShape.hpp"

#define FGE_OBJLINESHAPE_CLASSNAME "FGE:OBJ:LINESHAPE"

namespace fge
{

class FGE_API ObjLineShape : public fge::ObjShape
{
public:
    ObjLineShape(Vector2f const& beginning, Vector2f const& end, float thickness);
    ~ObjLineShape() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjLineShape)

    void setThickness(float thickness);
    void setEndPoint(Vector2f const& point);

    float getThickness() const;
    Vector2f getEndPoint() const;

    float getLength() const;

    [[nodiscard]] std::size_t getPointCount() const override;
    [[nodiscard]] Vector2f getPoint(std::size_t index) const override;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

private:
    Vector2f g_direction;
    float g_thickness;
};

} // namespace fge

#endif // _FGE_C_OBJLINESHAPE_HPP_INCLUDED
