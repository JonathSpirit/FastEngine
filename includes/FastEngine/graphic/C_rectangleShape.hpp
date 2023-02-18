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

#ifndef _FGE_GRAPHIC_C_RECTANGLESHAPE_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RECTANGLESHAPE_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/graphic/C_shape.hpp"

namespace fge
{

class FGE_API RectangleShape : public Shape
{
public:
    explicit RectangleShape(const Vector2f& size = Vector2f(0, 0));

    void setSize(const Vector2f& size);
    const Vector2f& getSize() const;

    [[nodiscard]] std::size_t getPointCount() const override;

    [[nodiscard]] Vector2f getPoint(std::size_t index) const override;

private:
    Vector2f g_size; //!< Size of the rectangle
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_RECTANGLESHAPE_HPP_INCLUDED
