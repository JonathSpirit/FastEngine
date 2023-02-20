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

#ifndef _FGE_C_OBJSHAPE_HPP_INCLUDED
#define _FGE_C_OBJSHAPE_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fastengine_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_texture.hpp"

namespace fge
{

class FGE_API ObjShape : public fge::Object
{
public:
    ~ObjShape() override = default;

    void setTexture(const Texture& texture, bool resetRect = false);
    void setTextureRect(const fge::RectInt& rect);

    void setFillColor(const Color& color);
    void setOutlineColor(const Color& color);

    void setOutlineThickness(float thickness);

    [[nodiscard]] const Texture& getTexture() const;

    [[nodiscard]] const RectInt& getTextureRect() const;

    [[nodiscard]] const Color& getFillColor() const;
    [[nodiscard]] const Color& getOutlineColor() const;

    [[nodiscard]] float getOutlineThickness() const;

    [[nodiscard]] virtual std::size_t getPointCount() const = 0;
    [[nodiscard]] virtual Vector2f getPoint(std::size_t index) const = 0;

    FGE_OBJ_DRAW_DECLARE

    [[nodiscard]] fge::RectFloat getLocalBounds() const override;
    [[nodiscard]] fge::RectFloat getGlobalBounds() const override;

protected:
    ObjShape();
    void updateShape();

private:
    void updateFillColors();
    void updateTexCoords();
    void updateOutline();
    void updateOutlineColors();

    fge::Texture g_texture;
    fge::RectInt g_textureRect;

    fge::Color g_fillColor;
    fge::Color g_outlineColor;

    float g_outlineThickness;

    fge::vulkan::VertexBuffer g_vertices;
    fge::vulkan::VertexBuffer g_outlineVertices;

    fge::RectFloat g_insideBounds;
    fge::RectFloat g_bounds;
};

} // namespace fge


#endif // _FGE_C_OBJSHAPE_HPP_INCLUDED
