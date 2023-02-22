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

#include "FastEngine/object/C_objSpriteBatches.hpp"

namespace fge
{

ObjSpriteBatches::ObjSpriteBatches() :
        g_instancesTransformDataCapacity(0),
        g_needBuffersUpdate(true),
        g_dynamicAlignment(0)
{
    this->g_descriptorSet =
            fge::vulkan::GlobalContext->getTransformBatchesDescriptorPool()
                    .allocateDescriptorSet(fge::vulkan::GlobalContext->getTransformBatchesLayout().getLayout())
                    .value();

    this->g_instancesVertices.create(*fge::vulkan::GlobalContext, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                                     fge::vulkan::BufferTypes::LOCAL);

    const std::size_t minUboAlignment =
            fge::vulkan::GlobalContext->getPhysicalDevice().getMinUniformBufferOffsetAlignment();

    this->g_dynamicAlignment = fge::TransformUboData::uboSize;
    if (minUboAlignment > 0)
    {
        this->g_dynamicAlignment = (this->g_dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
}
ObjSpriteBatches::ObjSpriteBatches(const ObjSpriteBatches& r) :
        fge::Object(r),
        g_texture(r.g_texture),
        g_instancesData(r.g_instancesData),
        g_instancesTransformDataCapacity(0),
        g_needBuffersUpdate(true),
        g_dynamicAlignment(r.g_dynamicAlignment)
{
    this->g_descriptorSet =
            fge::vulkan::GlobalContext->getTransformBatchesDescriptorPool()
                    .allocateDescriptorSet(fge::vulkan::GlobalContext->getTransformBatchesLayout().getLayout())
                    .value();

    this->g_instancesVertices.create(*fge::vulkan::GlobalContext, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                                     fge::vulkan::BufferTypes::LOCAL);
}
ObjSpriteBatches::ObjSpriteBatches(fge::Texture texture) :
        ObjSpriteBatches()
{
    this->setTexture(std::move(texture));
}

void ObjSpriteBatches::setTexture(fge::Texture texture)
{
    this->g_texture = std::move(texture);
}

void ObjSpriteBatches::clear()
{
    this->g_instancesData.clear();
    this->g_instancesVertices.clear();
    this->g_needBuffersUpdate = true;
}
fge::Transformable& ObjSpriteBatches::addSprite(const fge::RectInt& rectangle)
{
    auto& transformable = this->g_instancesData.emplace_back(rectangle)._transformable;
    this->g_instancesVertices.resize(this->g_instancesVertices.getCount() + 4);
    this->updatePositions(this->g_instancesData.size() - 1);
    this->updateTexCoords(this->g_instancesData.size() - 1);
    this->g_needBuffersUpdate = true;
    return transformable;
}
void ObjSpriteBatches::resize(std::size_t size)
{
    const std::size_t oldSize = this->g_instancesData.size();

    this->g_instancesData.resize(size);
    this->g_instancesVertices.resize(size * 4);

    if (size > oldSize)
    {
        for (std::size_t i = oldSize; i < size; ++i)
        {
            this->updatePositions(i);
            this->updateTexCoords(i);
        }
    }
}
void ObjSpriteBatches::setTextureRect(std::size_t index, const fge::RectInt& rectangle)
{
    if (index < this->g_instancesData.size())
    {
        if (rectangle != this->g_instancesData[index]._textureRect)
        {
            this->g_instancesData[index]._textureRect = rectangle;
            this->updatePositions(index);
            this->updateTexCoords(index);
        }
    }
}

void ObjSpriteBatches::setColor(std::size_t index, const fge::Color& color)
{
    if (index < this->g_instancesData.size())
    {
        const std::size_t startIndex = index * 4;

        this->g_instancesVertices[startIndex]._color = color;
        this->g_instancesVertices[startIndex + 1]._color = color;
        this->g_instancesVertices[startIndex + 2]._color = color;
        this->g_instancesVertices[startIndex + 3]._color = color;
    }
}

const fge::Texture& ObjSpriteBatches::getTexture() const
{
    return this->g_texture;
}

std::optional<fge::RectInt> ObjSpriteBatches::getTextureRect(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        return this->g_instancesData[index]._textureRect;
    }
    return std::nullopt;
}
std::optional<fge::Color> ObjSpriteBatches::getColor(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        return fge::Color(this->g_instancesVertices[index * 4]._color);
    }
    return std::nullopt;
}

fge::Transformable* ObjSpriteBatches::getTransformable(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        return &this->g_instancesData[index]._transformable;
    }
    return nullptr;
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSpriteBatches)
{
    if (this->g_instancesData.empty())
    {
        return;
    }

    this->updateBuffers();

    for (std::size_t i = 0; i < this->g_instancesData.size(); ++i)
    {
        auto* uboData = reinterpret_cast<fge::TransformUboData*>(
                static_cast<uint8_t*>(this->g_instancesTransform.getBufferMapped()) + i * this->g_dynamicAlignment);

        if (states._transform != nullptr)
        {
            uboData->_modelTransform =
                    states._transform->_data._modelTransform * this->g_instancesData[i]._transformable.getTransform();
        }
        else
        {
            uboData->_modelTransform = this->g_instancesData[i]._transformable.getTransform();
        }
        uboData->_viewTransform = target.getView().getTransform();
    }

    target.drawBatches(states._blendMode, static_cast<const fge::vulkan::TextureImage*>(this->g_texture),
                       this->g_descriptorSet, &this->g_instancesVertices, 4, this->g_instancesData.size());
}
#endif

void ObjSpriteBatches::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["texture"] = this->g_texture;
}
void ObjSpriteBatches::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->g_texture = jsonObject.value<std::string>("texture", FGE_TEXTURE_BAD);
}

void ObjSpriteBatches::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_texture;
}
void ObjSpriteBatches::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_texture;
}

const char* ObjSpriteBatches::getClassName() const
{
    return FGE_OBJSPRITEBATCHES_CLASSNAME;
}
const char* ObjSpriteBatches::getReadableClassName() const
{
    return "sprite batches";
}

fge::RectFloat ObjSpriteBatches::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
std::optional<fge::RectFloat> ObjSpriteBatches::getGlobalBounds(std::size_t index) const
{
    auto localBounds = this->getLocalBounds(index);
    if (localBounds && !this->g_needBuffersUpdate)
    {
        return this->g_instancesData[index]._transformable.getTransform() * localBounds.value();
    }
    return std::nullopt;
}
fge::RectFloat ObjSpriteBatches::getLocalBounds() const
{
    return fge::Object::getLocalBounds();
}
std::optional<fge::RectFloat> ObjSpriteBatches::getLocalBounds(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        const auto width = static_cast<float>(this->g_instancesData[index]._textureRect._width);
        const auto height = static_cast<float>(this->g_instancesData[index]._textureRect._height);

        return fge::RectFloat{{0.f, 0.f}, {width, height}};
    }
    return std::nullopt;
}

void ObjSpriteBatches::updatePositions(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        const fge::RectFloat bounds = this->getLocalBounds(index).value();
        const std::size_t startIndex = index * 4;

        this->g_instancesVertices[startIndex]._position = fge::Vector2f(0, 0);
        this->g_instancesVertices[startIndex + 1]._position = fge::Vector2f(0, bounds._height);
        this->g_instancesVertices[startIndex + 2]._position = fge::Vector2f(bounds._width, 0);
        this->g_instancesVertices[startIndex + 3]._position = fge::Vector2f(bounds._width, bounds._height);
    }
}

void ObjSpriteBatches::updateTexCoords(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        const auto rect =
                this->g_texture.getData()->_texture->normalizeTextureRect(this->g_instancesData[index]._textureRect);
        const std::size_t startIndex = index * 4;

        this->g_instancesVertices[startIndex]._texCoords = fge::Vector2f(rect._x, rect._y);
        this->g_instancesVertices[startIndex + 1]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
        this->g_instancesVertices[startIndex + 2]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
        this->g_instancesVertices[startIndex + 3]._texCoords =
                fge::Vector2f(rect._x + rect._width, rect._y + rect._height);
    }
}
void ObjSpriteBatches::updateBuffers() const
{
    if (this->g_needBuffersUpdate)
    {
        this->g_needBuffersUpdate = false;

        if (!this->g_instancesData.empty() && this->g_instancesData.size() > this->g_instancesTransformDataCapacity)
        {
            this->g_instancesTransformDataCapacity = this->g_instancesData.size();

            this->g_instancesTransform.create(*fge::vulkan::GlobalContext,
                                              this->g_dynamicAlignment * this->g_instancesData.size());

            const fge::vulkan::DescriptorSet::Descriptor descriptor{
                    this->g_instancesTransform, FGE_VULKAN_TRANSFORM_BINDING,
                    fge::vulkan::DescriptorSet::Descriptor::BufferTypes::DYNAMIC, fge::TransformUboData::uboSize};
            this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);
        }
    }
}

} // namespace fge
