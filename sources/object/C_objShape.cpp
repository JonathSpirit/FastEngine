/*
 * Copyright 2025 Guillaume Guillet
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
#include "FastEngine/manager/shader_manager.hpp"

namespace fge
{

namespace
{

struct ConstantData
{
    glm::uint _colorIndex;
    glm::uint _instanceIndex;
};

} //end namespace

void ObjShape::setTexture(Texture const& texture, bool resetRect)
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

Texture const& ObjShape::getTexture() const
{
    return this->g_texture;
}

void ObjShape::setTextureRect(RectInt const& rect)
{
    this->g_textureRect = rect;
    this->updateTexCoords();
}

RectInt const& ObjShape::getTextureRect() const
{
    return this->g_textureRect;
}

void ObjShape::setFillColor(Color color, std::size_t instance)
{
    if (instance < this->g_instancesCount)
    {
        this->retrieveInstance(instance)->_color[FGE_OBJSHAPE_INDEX_FILLCOLOR] = color;
    }
}
void ObjShape::setOutlineColor(Color color, std::size_t instance)
{
    if (instance < this->g_instancesCount)
    {
        this->retrieveInstance(instance)->_color[FGE_OBJSHAPE_INDEX_OUTLINECOLOR] = color;
    }
}
void ObjShape::setOffset(fge::Vector2f const& offset, std::size_t instance)
{
    if (instance < this->g_instancesCount)
    {
        this->retrieveInstance(instance)->_offset = offset;
    }
}
void ObjShape::setInstancesCount(std::size_t count)
{
    if (count == 0)
    {
        count = 1;
    }
    this->resizeBuffer(count);
}
void ObjShape::addInstance(Color fillColor, Color outlineColor, fge::Vector2f const& offset)
{
    this->resizeBuffer(this->g_instancesCount + 1);
    *this->retrieveInstance(this->g_instancesCount - 1) = {{fillColor, outlineColor}, offset};
}
std::size_t ObjShape::getInstancesCount() const
{
    return this->g_instancesCount;
}
void ObjShape::clearInstances()
{
    this->resizeBuffer(1);
}

Color ObjShape::getFillColor(std::size_t instance) const
{
    return fge::Color(this->retrieveInstance(instance)->_color[FGE_OBJSHAPE_INDEX_FILLCOLOR]);
}
Color ObjShape::getOutlineColor(std::size_t instance) const
{
    return fge::Color(this->retrieveInstance(instance)->_color[FGE_OBJSHAPE_INDEX_OUTLINECOLOR]);
}
fge::Vector2f const& ObjShape::getOffset(std::size_t instance) const
{
    return this->retrieveInstance(instance)->_offset;
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
        g_outlineThickness(0.0f),
        g_vertices(fge::vulkan::GetActiveContext()),
        g_outlineVertices(fge::vulkan::GetActiveContext()),
        g_instancesCount(0),
        g_instances(fge::vulkan::GetActiveContext(), vulkan::UniformBuffer::Types::STORAGE_BUFFER)
{
    this->g_vertices.create(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
    this->g_outlineVertices.create(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

    this->resizeBuffer(1);
}
ObjShape::ObjShape(ObjShape const& r) :
        fge::Object(r),
        g_texture(r.g_texture),
        g_textureRect(r.g_textureRect),
        g_outlineThickness(r.g_outlineThickness),
        g_vertices(r.g_vertices),
        g_outlineVertices(r.g_outlineVertices),
        g_instancesCount(r.g_instancesCount),
        g_instances(r.g_instances),
        g_insideBounds(r.g_insideBounds),
        g_bounds(r.g_bounds)
{
    this->updateDescriptors();
}
void ObjShape::updateShape()
{
    // Get the total number of points of the shape
    std::size_t const count = this->getPointCount();
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

void ObjShape::first([[maybe_unused]] fge::Scene& scene)
{
    this->updateShape();
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjShape)
{
    if (this->g_instancesCount == 0)
    {
        return;
    }

    auto copyStates = states.copy();

    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform),
                                 RenderResourceTransform::Configs::GLOBAL_TRANSFORMS_INDEX_IS_IGNORED);

    fge::TextureType const* const texture = this->g_texture.valid() ? this->g_texture.retrieve() : nullptr;

    copyStates._shaderVertex = fge::shader::gManager.getElement(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX)->_ptr.get();
    copyStates._shaderFragment = fge::shader::gManager.getElement(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT)->_ptr.get();

    //Drawing inline
    copyStates._resTextures.set(texture, texture == nullptr ? 0 : 1);
    copyStates._vertexBuffer = &this->g_vertices;

    ConstantData constantData{FGE_OBJSHAPE_INDEX_FILLCOLOR,
                              copyStates._resTransform.getGlobalTransformsIndex().value()};
    fge::RenderResourcePushConstants::PushConstantData const pushConstant{VK_SHADER_STAGE_VERTEX_BIT, 0,
                                                                          sizeof(ConstantData), &constantData};

    copyStates._resPushConstants.set(&pushConstant, 1);

    copyStates._resInstances.setInstancesCount(this->g_instancesCount, true);
    uint32_t const sets[] = {1};
    copyStates._resDescriptors.set(&this->g_descriptorSet, sets, 1);

    target.draw(copyStates);

    //Drawing outline
    constantData._colorIndex = FGE_OBJSHAPE_INDEX_OUTLINECOLOR;

    copyStates._resTextures.set<std::nullptr_t>(nullptr, 0);
    copyStates._vertexBuffer = &this->g_outlineVertices;

    target.draw(copyStates);
}
#endif

void ObjShape::updateTexCoords()
{
    auto convertedTextureRect = RectFloat(this->g_textureRect);

    for (std::size_t i = 0; i < this->g_vertices.getCount(); ++i)
    {
        float const xratio =
                this->g_insideBounds._width > 0
                        ? (this->g_vertices[i]._position.x - this->g_insideBounds._x) / this->g_insideBounds._width
                        : 0;
        float const yratio =
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

    std::size_t const count = this->g_vertices.getCount() - 2;
    this->g_outlineVertices.resize((count + 1) * 2);

    for (std::size_t i = 0; i < count; ++i)
    {
        std::size_t const index = i + 1;

        // Get the two segments shared by the current point
        fge::Vector2f const p0 = (i == 0) ? this->g_vertices[count]._position : this->g_vertices[index - 1]._position;
        fge::Vector2f const p1 = this->g_vertices[index]._position;
        fge::Vector2f const p2 = this->g_vertices[index + 1]._position;

        // Compute their normal
        fge::Vector2f n1 = fge::GetSegmentNormal(p0, p1);
        fge::Vector2f n2 = fge::GetSegmentNormal(p1, p2);

        // Make sure that the normals point towards the outside of the shape
        // (this depends on the order in which the points were defined)
        if (glm::dot(n1, this->g_vertices[0]._position - p1) > 0)
        {
            n1 = -n1;
        }
        if (glm::dot(n2, this->g_vertices[0]._position - p1) > 0)
        {
            n2 = -n2;
        }

        // Combine them to get the extrusion direction
        float const factor = 1.f + (n1.x * n2.x + n1.y * n2.y);
        fge::Vector2f const normal = (n1 + n2) / factor;

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

void ObjShape::resizeBuffer(std::size_t size) const
{
    bool needDescriptorUpdate = false;
    VkDeviceSize const newSize = size * sizeof(InstanceData);
    if (newSize > this->g_instances.getBufferCapacity())
    {
        needDescriptorUpdate = true;
    }
    this->g_instances.resize(size * sizeof(InstanceData));

    if (size <= this->g_instancesCount)
    {
        this->g_instancesCount = size;
        return;
    }

    InstanceData defaultInstanceData;
    if (this->g_instancesCount == 0)
    {
        defaultInstanceData._offset = {0.0f, 0.0f};
        defaultInstanceData._color[FGE_OBJSHAPE_INDEX_FILLCOLOR] = fge::Color::White;
        defaultInstanceData._color[FGE_OBJSHAPE_INDEX_OUTLINECOLOR] = fge::Color::White;
    }
    else
    {
        defaultInstanceData = *this->retrieveInstance(0);
    }

    for (std::size_t i = this->g_instancesCount; i < size; ++i)
    {
        new (this->retrieveInstance(i)) InstanceData(defaultInstanceData);
    }

    this->g_instancesCount = size;

    if (needDescriptorUpdate)
    {
        this->updateDescriptors();
    }
}
void ObjShape::updateDescriptors() const
{
#ifndef FGE_DEF_SERVER
    using namespace fge::vulkan;

    if (this->g_descriptorSet.get() == VK_NULL_HANDLE)
    {
        //BufferInstanceData in the vertex shader
        this->g_descriptorSet = GetActiveContext().createDescriptorSet(FGE_OBJSHAPE_INSTANCES_SHADER_VERTEX, 1).value();
    }

    DescriptorSet::Descriptor const descriptor{this->g_instances, FGE_VULKAN_TRANSFORM_BINDING,
                                               DescriptorSet::Descriptor::BufferTypes::STORAGE, VK_WHOLE_SIZE};
    this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);
#endif //FGE_DEF_SERVER
}

ObjShape::InstanceData* ObjShape::retrieveInstance(std::size_t index) const
{
    return static_cast<InstanceData*>(this->g_instances.getBufferMapped()) + index;
}

} // namespace fge
