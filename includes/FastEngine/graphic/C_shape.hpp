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

#ifndef _FGE_GRAPHIC_C_SHAPE_HPP_INCLUDED
#define _FGE_GRAPHIC_C_SHAPE_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/graphic/C_drawable.hpp>
#include <FastEngine/graphic/C_transformable.hpp>
#include <FastEngine/vulkan/C_vertexBuffer.hpp>
#include <FastEngine/graphic/C_vector.hpp>
#include <FastEngine/C_texture.hpp>

namespace fge
{

class FGE_API Shape : public fge::Drawable, public fge::Transformable
{
public:
    ~Shape() override = default;

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

    [[nodiscard]] RectFloat getLocalBounds() const;
    [[nodiscard]] RectFloat getGlobalBounds() const;

    void draw(RenderTarget& target, const fge::RenderStates& states) const override;

protected:
    Shape();

    void update();

private:
    void updateFillColors();

    void updateTexCoords();

    void updateOutline();

    void updateOutlineColors();

    Texture g_texture;          //!< Texture of the shape
    RectInt        g_textureRect;      //!< Rectangle defining the area of the source texture to display
    Color          g_fillColor;        //!< Fill color
    Color          g_outlineColor;     //!< Outline color
    float          g_outlineThickness; //!< Thickness of the shape's outline
    fge::vulkan::VertexBuffer    g_vertices;         //!< Vertex array containing the fill geometry
    fge::vulkan::VertexBuffer    g_outlineVertices;  //!< Vertex array containing the outline geometry
    RectFloat     g_insideBounds;     //!< Bounding rectangle of the inside (fill)
    RectFloat      g_bounds;           //!< Bounding rectangle of the whole shape (outline + fill)
};

} // namespace fge


#endif // _FGE_GRAPHIC_C_SHAPE_HPP_INCLUDED
