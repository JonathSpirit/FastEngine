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

#include "FastEngine/object/C_objShape.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

namespace
{

#ifndef FGE_DEF_SERVER
void InstanceVertexShader_constructor(const fge::vulkan::Context* context,
                                      const fge::RenderTarget::GraphicPipelineKey& key,
                                      fge::vulkan::GraphicPipeline* graphicPipeline)
{
    graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT)->_shader);
    graphicPipeline->setShader(fge::shader::GetShader(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX)->_shader);
    graphicPipeline->setBlendMode(key._blendMode);

    graphicPipeline->setPushConstantRanges(
            {VkPushConstantRange{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ObjShape::InstancePushData)}});

    graphicPipeline->setDescriptorSetLayouts({context->getTransformLayout().getLayout()});
}
#endif

} //end namespace

void ObjShape::setTexture(const Texture& texture, bool resetRect)
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

const Texture& ObjShape::getTexture() const
{
    return this->g_texture;
}

void ObjShape::setTextureRect(const RectInt& rect)
{
    this->g_textureRect = rect;
    this->updateTexCoords();
}

const RectInt& ObjShape::getTextureRect() const
{
    return this->g_textureRect;
}

void ObjShape::setFillColor(Color color, std::size_t instance)
{
    if (instance < this->g_instances.size())
    {
        this->g_instances[instance]._fillColor = color;
    }
}
void ObjShape::setOutlineColor(Color color, std::size_t instance)
{
    if (instance < this->g_instances.size())
    {
        this->g_instances[instance]._outlineColor = color;
    }
}
void ObjShape::setOffset(const fge::Vector2f& offset, std::size_t instance)
{
    if (instance < this->g_instances.size())
    {
        this->g_instances[instance]._offset = offset;
    }
}
void ObjShape::setInstancesCount(std::size_t count)
{
    if (count == 0)
    {
        count = 1;
    }
    this->g_instances.resize(count);
}
void ObjShape::addInstance(Color fillColor, Color outlineColor, const fge::Vector2f& offset)
{
    this->g_instances.push_back({fillColor, outlineColor, offset});
}
std::size_t ObjShape::getInstancesCount() const
{
    return this->g_instances.size();
}
void ObjShape::clearInstances()
{
    this->g_instances.resize(1);
}

Color ObjShape::getFillColor(std::size_t instance) const
{
    return this->g_instances[instance]._fillColor;
}
Color ObjShape::getOutlineColor(std::size_t instance) const
{
    return this->g_instances[instance]._outlineColor;
}
const fge::Vector2f& ObjShape::getOffset(std::size_t instance) const
{
    return this->g_instances[instance]._offset;
}

void ObjShape::setOutlineThickness(float thickness)
{
    this->g_outlineThickness = thickness;
    this->updateShape(); // recompute everything because the whole shape must be offset
}

float ObjShape::getOutlineThickness() const
{
    return this->g_outlineThickness;
}

RectFloat ObjShape::getLocalBounds() const
{
    return this->g_bounds;
}

RectFloat ObjShape::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}

ObjShape::ObjShape() :
        g_texture(),
        g_textureRect(),
        g_outlineThickness(0),
        g_vertices(),
        g_outlineVertices(),
        g_insideBounds(),
        g_bounds()
{
    this->g_vertices.create(*fge::vulkan::GlobalContext, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
    this->g_outlineVertices.create(*fge::vulkan::GlobalContext, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

    this->g_instances.push_back({fge::Color::White, fge::Color::White, {0.0f, 0.0f}});
}
void ObjShape::updateShape()
{
    // Get the total number of points of the shape
    const std::size_t count = this->getPointCount();
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
    this->g_vertices.getVertices()[0] =
            this->g_vertices.getVertices()[1]; // so that the result of getBounds() is correct
    this->g_insideBounds = this->g_vertices.getBounds();

    // Compute the center and make it the first vertex
    this->g_vertices.getVertices()[0]._position.x = this->g_insideBounds._x + this->g_insideBounds._width / 2;
    this->g_vertices.getVertices()[0]._position.y = this->g_insideBounds._y + this->g_insideBounds._height / 2;

    // Texture coordinates
    this->updateTexCoords();

    // Outline
    this->updateOutline();
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjShape)
{
    if (this->g_instances.empty())
    {
        return;
    }

    auto copyStates = states.copy(this->_transform.start(*this, states._transform));

    const fge::TextureType* const texture =
            this->g_texture.valid() ? static_cast<const fge::TextureType*>(this->g_texture) : nullptr;

    const fge::RenderTarget::GraphicPipelineKey key{copyStates._blendMode, 0};
    auto* graphicPipeline =
            target.getGraphicPipeline(typeid(fge::ObjShape).name(), key, &InstanceVertexShader_constructor);

    for (const auto& instance: this->g_instances)
    {
        InstancePushData instancePushData{instance._fillColor, instance._offset};

        // Render the inside
        copyStates._textureImage = texture;
        copyStates._vertexBuffer = &this->g_vertices;

        graphicPipeline->pushConstants(target.getCommandBuffer(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                                       sizeof(InstancePushData), &instancePushData);
        target.draw(copyStates, graphicPipeline);

        // Render the outline
        if (this->g_outlineThickness != 0.0f)
        {
            copyStates._textureImage = nullptr;
            copyStates._vertexBuffer = &this->g_outlineVertices;

            instancePushData._color = instance._outlineColor;
            graphicPipeline->pushConstants(target.getCommandBuffer(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                                           sizeof(InstancePushData), &instancePushData);
            target.draw(copyStates, graphicPipeline);
        }
    }
}
#endif

void ObjShape::updateTexCoords()
{
    auto convertedTextureRect = RectFloat(this->g_textureRect);

    for (std::size_t i = 0; i < this->g_vertices.getCount(); ++i)
    {
        const float xratio =
                this->g_insideBounds._width > 0
                        ? (this->g_vertices[i]._position.x - this->g_insideBounds._x) / this->g_insideBounds._width
                        : 0;
        const float yratio =
                this->g_insideBounds._height > 0
                        ? (this->g_vertices[i]._position.y - this->g_insideBounds._y) / this->g_insideBounds._height
                        : 0;
        this->g_vertices[i]._texCoords.x = convertedTextureRect._x + convertedTextureRect._width * xratio;
        this->g_vertices[i]._texCoords.y = convertedTextureRect._y + convertedTextureRect._height * yratio;
    }
}

void ObjShape::updateOutline()
{
    // Return if there is no outline
    if (this->g_outlineThickness == 0.0f)
    {
        this->g_outlineVertices.clear();
        this->g_bounds = this->g_insideBounds;
        return;
    }

    const std::size_t count = this->g_vertices.getCount() - 2;
    this->g_outlineVertices.resize((count + 1) * 2);

    for (std::size_t i = 0; i < count; ++i)
    {
        const std::size_t index = i + 1;

        // Get the two segments shared by the current point
        const fge::Vector2f p0 = (i == 0) ? this->g_vertices[count]._position : this->g_vertices[index - 1]._position;
        const fge::Vector2f p1 = this->g_vertices[index]._position;
        const fge::Vector2f p2 = this->g_vertices[index + 1]._position;

        // Compute their normal
        fge::Vector2f n1 = fge::GetNormal(p0, p1);
        fge::Vector2f n2 = fge::GetNormal(p1, p2);

        // Make sure that the normals point towards the outside of the shape
        // (this depends on the order in which the points were defined)
        if (fge::GetDotProduct(n1, this->g_vertices[0]._position - p1) > 0)
        {
            n1 = -n1;
        }
        if (fge::GetDotProduct(n2, this->g_vertices[0]._position - p1) > 0)
        {
            n2 = -n2;
        }

        // Combine them to get the extrusion direction
        const float factor = 1.f + (n1.x * n2.x + n1.y * n2.y);
        const fge::Vector2f normal = (n1 + n2) / factor;

        // Update the outline points
        this->g_outlineVertices[i * 2 + 0]._position = p1;
        this->g_outlineVertices[i * 2 + 1]._position = p1 + normal * this->g_outlineThickness;
    }

    // Duplicate the first point at the end, to close the outline
    this->g_outlineVertices[count * 2 + 0]._position = this->g_outlineVertices.getVertices()[0]._position;
    this->g_outlineVertices[count * 2 + 1]._position = this->g_outlineVertices.getVertices()[1]._position;

    // Update the shape's bounds
    this->g_bounds = this->g_outlineVertices.getBounds();
}

} // namespace fge
