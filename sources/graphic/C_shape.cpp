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

#include <FastEngine/graphic/C_shape.hpp>
#include <FastEngine/graphic/C_renderTarget.hpp>
#include <FastEngine/vulkan/vulkanGlobal.hpp>
#include <cmath>

namespace fge
{

namespace
{

// Compute the normal of a segment
fge::Vector2f computeNormal(const fge::Vector2f& p1, const fge::Vector2f& p2)
{
    fge::Vector2f normal(p1.y - p2.y, p2.x - p1.x);
    const float length = std::sqrt(normal.x * normal.x + normal.y * normal.y);
    if (length != 0.f)
    {
        normal /= length;
    }
    return normal;
}

// Compute the dot product of two vectors
float dotProduct(const fge::Vector2f& p1, const fge::Vector2f& p2)
{
    return p1.x * p2.x + p1.y * p2.y;
}

}//end namespace

void Shape::setTexture(const Texture& texture, bool resetRect)
{
    if (texture.valid())
    {
        // Recompute the texture area if requested, or if there was no texture & rect before
        if (resetRect || (!this->g_texture.valid() && (this->g_textureRect == RectInt())))
        {
            this->setTextureRect(RectInt({0, 0}, texture.getTextureSize()));
        }
    }

    // Assign the new texture
    this->g_texture = texture;
}

const Texture& Shape::getTexture() const
{
    return this->g_texture;
}

void Shape::setTextureRect(const RectInt& rect)
{
    this->g_textureRect = rect;
    this->updateTexCoords();
}

const RectInt& Shape::getTextureRect() const
{
    return this->g_textureRect;
}

void Shape::setFillColor(const Color& color)
{
    this->g_fillColor = color;
    this->updateFillColors();
}

const Color& Shape::getFillColor() const
{
    return this->g_fillColor;
}

void Shape::setOutlineColor(const Color& color)
{
    this->g_outlineColor = color;
    this->updateOutlineColors();
}

const Color& Shape::getOutlineColor() const
{
    return this->g_outlineColor;
}

void Shape::setOutlineThickness(float thickness)
{
    this->g_outlineThickness = thickness;
    this->update(); // recompute everything because the whole shape must be offset
}

float Shape::getOutlineThickness() const
{
    return this->g_outlineThickness;
}

RectFloat Shape::getLocalBounds() const
{
    return this->g_bounds;
}

RectFloat Shape::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}

Shape::Shape() :
g_texture         (),
g_textureRect     (),
g_fillColor       (255, 255, 255),
g_outlineColor    (255, 255, 255),
g_outlineThickness(0),
g_vertices        (),
g_outlineVertices (),
g_insideBounds    (),
g_bounds          ()
{
    this->g_vertices.create(*fge::vulkan::GlobalContext, 0,0, false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
    this->g_outlineVertices.create(*fge::vulkan::GlobalContext, 0,0, false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
}

void Shape::update()
{
    // Get the total number of points of the shape
    const std::size_t count = getPointCount();
    if (count < 3)
    {
        this->g_vertices.clear();
        this->g_outlineVertices.clear();
        return;
    }

    this->g_vertices.resize(count + 2); // + 2 for center and repeated first point

    // Position
    for (std::size_t i = 0; i < count; ++i)
    {
        this->g_vertices.getVertices()[i + 1]._position = getPoint(i);
    }
    this->g_vertices.getVertices()[count + 1]._position = this->g_vertices.getVertices()[1]._position;

    // Update the bounding rectangle
    this->g_vertices.getVertices()[0] = this->g_vertices.getVertices()[1]; // so that the result of getBounds() is correct
    this->g_insideBounds = this->g_vertices.getBounds();

    // Compute the center and make it the first vertex
    this->g_vertices.getVertices()[0]._position.x = this->g_insideBounds._x + this->g_insideBounds._width / 2;
    this->g_vertices.getVertices()[0]._position.y = this->g_insideBounds._y + this->g_insideBounds._height / 2;

    // Color
    this->updateFillColors();

    // Texture coordinates
    this->updateTexCoords();

    // Outline
    this->updateOutline();
}

void Shape::draw(RenderTarget& target, const fge::RenderStates& states) const
{
    auto copyStates = states.copy(this->_transform.start(*this, states._transform));

    // Render the inside
    if (this->g_texture.valid())
    {
        copyStates._textureImage = static_cast<const fge::TextureType *>(this->g_texture);
    }

    copyStates._vertexBuffer = &this->g_vertices;
    target.draw(copyStates);

    // Render the outline
    if (this->g_outlineThickness != 0.0f)
    {
        copyStates._textureImage = nullptr;
        copyStates._vertexBuffer = &this->g_outlineVertices;
        target.draw(copyStates);
    }
}

void Shape::updateFillColors()
{
    for (std::size_t i = 0; i < this->g_vertices.getVertexCount(); ++i)
    {
        this->g_vertices.getVertices()[i]._color = this->g_fillColor;
    }
}

void Shape::updateTexCoords()
{
    auto convertedTextureRect = RectFloat(this->g_textureRect);

    for (std::size_t i = 0; i < this->g_vertices.getVertexCount(); ++i)
    {
        const float xratio = this->g_insideBounds._width > 0 ? (this->g_vertices.getVertices()[i]._position.x - this->g_insideBounds._x) / this->g_insideBounds._width : 0;
        const float yratio = this->g_insideBounds._height > 0 ? (this->g_vertices.getVertices()[i]._position.y - this->g_insideBounds._y) / this->g_insideBounds._height : 0;
        this->g_vertices.getVertices()[i]._texCoords.x = convertedTextureRect._x + convertedTextureRect._width * xratio;
        this->g_vertices.getVertices()[i]._texCoords.y = convertedTextureRect._y + convertedTextureRect._height * yratio;
    }
}

void Shape::updateOutline()
{
    // Return if there is no outline
    if (this->g_outlineThickness == 0.0f)
    {
        this->g_outlineVertices.clear();
        this->g_bounds = this->g_insideBounds;
        return;
    }

    const std::size_t count = this->g_vertices.getVertexCount() - 2;
    this->g_outlineVertices.resize((count + 1) * 2);

    for (std::size_t i = 0; i < count; ++i)
    {
        const std::size_t index = i + 1;

        // Get the two segments shared by the current point
        const fge::Vector2f p0 = (i == 0) ? this->g_vertices.getVertices()[count]._position : this->g_vertices.getVertices()[index - 1]._position;
        const fge::Vector2f p1 = this->g_vertices.getVertices()[index]._position;
        const fge::Vector2f p2 = this->g_vertices.getVertices()[index + 1]._position;

        // Compute their normal
        fge::Vector2f n1 = computeNormal(p0, p1);
        fge::Vector2f n2 = computeNormal(p1, p2);

        // Make sure that the normals point towards the outside of the shape
        // (this depends on the order in which the points were defined)
        if (dotProduct(n1, this->g_vertices.getVertices()[0]._position - p1) > 0)
        {
            n1 = -n1;
        }
        if (dotProduct(n2, this->g_vertices.getVertices()[0]._position - p1) > 0)
        {
            n2 = -n2;
        }

        // Combine them to get the extrusion direction
        const float factor = 1.f + (n1.x * n2.x + n1.y * n2.y);
        const fge::Vector2f normal = (n1 + n2) / factor;

        // Update the outline points
        this->g_outlineVertices.getVertices()[i * 2 + 0]._position = p1;
        this->g_outlineVertices.getVertices()[i * 2 + 1]._position = p1 + normal * this->g_outlineThickness;
    }

    // Duplicate the first point at the end, to close the outline
    this->g_outlineVertices.getVertices()[count * 2 + 0]._position = this->g_outlineVertices.getVertices()[0]._position;
    this->g_outlineVertices.getVertices()[count * 2 + 1]._position = this->g_outlineVertices.getVertices()[1]._position;

    // Update outline colors
    updateOutlineColors();

    // Update the shape's bounds
    this->g_bounds = this->g_outlineVertices.getBounds();
}

void Shape::updateOutlineColors()
{
    for (std::size_t i = 0; i < this->g_outlineVertices.getVertexCount(); ++i)
    {
        this->g_outlineVertices.getVertices()[i]._color = this->g_outlineColor;
    }
}

} // namespace fge
