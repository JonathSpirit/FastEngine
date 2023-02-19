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

#ifndef _FGE_C_OBJRECTANGLESHAPE_HPP_INCLUDED
#define _FGE_C_OBJRECTANGLESHAPE_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/object/C_objShape.hpp"

#define FGE_OBJRECTANGLESHAPE_CLASSNAME "FGE:OBJ:RECTANGLESHAPE"

namespace fge
{

class FGE_API ObjRectangleShape : public fge::ObjShape
{
public:
    explicit ObjRectangleShape(const Vector2f& size = Vector2f(0, 0));
    ~ObjRectangleShape() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjRectangleShape)

    void setSize(const Vector2f& size);
    const Vector2f& getSize() const;

    [[nodiscard]] std::size_t getPointCount() const override;

    [[nodiscard]] Vector2f getPoint(std::size_t index) const override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

private:
    Vector2f g_size;
};

} // namespace fge

#endif // _FGE_C_OBJRECTANGLESHAPE_HPP_INCLUDED
