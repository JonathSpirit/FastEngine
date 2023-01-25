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

#include "FastEngine/object/C_objSpriteBatches.hpp"

namespace fge
{

ObjSpriteBatches::ObjSpriteBatches() :
        g_instancesTransformData(nullptr),
        g_spriteCount(0),
        g_needBuffersUpdate(true)
{
    this->g_descriptorSet.create(fge::vulkan::GlobalContext->getLogicalDevice(),
                                 &fge::vulkan::GlobalContext->getTransformBatchesLayout(),
                                 1, fge::vulkan::GlobalContext->getTransformBatchesDescriptorPool(), true);
}
ObjSpriteBatches::ObjSpriteBatches(const ObjSpriteBatches& r) :
        fge::Object(r),
        g_texture(r.g_texture),
        g_instancesTransformable(r.g_instancesTransformable),
        g_instancesTransformData(nullptr),
        g_instancesTextureRect(r.g_instancesTextureRect),
        g_spriteCount(r.g_spriteCount),
        g_needBuffersUpdate(true)
{
    this->g_descriptorSet.create(fge::vulkan::GlobalContext->getLogicalDevice(),
                                 &fge::vulkan::GlobalContext->getTransformBatchesLayout(),
                                 1, fge::vulkan::GlobalContext->getTransformBatchesDescriptorPool(), true);
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

fge::Transformable& ObjSpriteBatches::addSprite(const fge::RectInt& rectangle)
{
    this->g_instancesTextureRect.push_back(rectangle);
    ++this->g_spriteCount;
    this->g_needBuffersUpdate = true;
    return this->g_instancesTransformable.emplace_back();
}
void ObjSpriteBatches::resize(std::size_t size)
{
    this->g_instancesTextureRect.resize(size);
    this->g_spriteCount = size;
    this->g_needBuffersUpdate = true;
    this->g_instancesTransformable.resize(size);
}
void ObjSpriteBatches::setTextureRect(std::size_t index, const fge::RectInt& rectangle)
{
    if (index < this->g_spriteCount)
    {
        if (rectangle != this->g_instancesTextureRect[index])
        {
            this->g_instancesTextureRect[index] = rectangle;
            this->updatePositions(index);
            this->updateTexCoords(index);
        }
    }
}

void ObjSpriteBatches::setColor(std::size_t index, const fge::Color& color)
{
    if (index < this->g_spriteCount && !this->g_needBuffersUpdate)
    {
        const std::size_t startIndex = index*4;

        this->g_instancesVertices[startIndex]._color = color;
        this->g_instancesVertices[startIndex+1]._color = color;
        this->g_instancesVertices[startIndex+2]._color = color;
        this->g_instancesVertices[startIndex+3]._color = color;
    }
}

const fge::Texture& ObjSpriteBatches::getTexture() const
{
    return this->g_texture;
}

std::optional<fge::RectInt> ObjSpriteBatches::getTextureRect(std::size_t index) const
{
    if (index < this->g_spriteCount && !this->g_needBuffersUpdate)
    {
        return this->g_instancesTextureRect[index];
    }
    return std::nullopt;
}
std::optional<fge::Color> ObjSpriteBatches::getColor(std::size_t index) const
{
    if (index < this->g_spriteCount && !this->g_needBuffersUpdate)
    {
        return fge::Color(this->g_instancesVertices[index*4]._color);
    }
    return std::nullopt;
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSpriteBatches)
{
    this->updateBuffers();

    for (std::size_t i=0; i<this->g_spriteCount; ++i)
    {
        this->g_instancesTransformData[i]._modelTransform = this->g_instancesTransformable[i].getTransform();
        this->g_instancesTransformData[i]._viewTransform = target.getView().getTransform();
    }

    this->g_instancesTransform.copyData(this->g_instancesTransformData.get(), this->g_instancesTransform.getBufferSize());

    target.drawBatches(states._blendMode,
                       static_cast<const fge::vulkan::TextureImage*>(this->g_texture),
                       this->g_descriptorSet,
                       &this->g_instancesVertices,
                       4, this->g_spriteCount);
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
        return this->g_instancesTransformable[index].getTransform() * localBounds.value();
    }
    return std::nullopt;
}
fge::RectFloat ObjSpriteBatches::getLocalBounds() const
{
    return fge::Object::getLocalBounds();
}
std::optional<fge::RectFloat> ObjSpriteBatches::getLocalBounds(std::size_t index) const
{
    if (index < this->g_spriteCount)
    {
        const auto width = static_cast<float>(this->g_instancesTextureRect[index]._width);
        const auto height = static_cast<float>(this->g_instancesTextureRect[index]._height);

        return fge::RectFloat{{0.f, 0.f}, {width, height}};
    }
    return std::nullopt;
}

void ObjSpriteBatches::updatePositions(std::size_t index) const
{
    if (index < this->g_spriteCount)
    {
        const fge::RectFloat bounds = this->getLocalBounds(index).value();
        const std::size_t startIndex = index*4;

        this->g_instancesVertices[startIndex]._position = fge::Vector2f(0, 0);
        this->g_instancesVertices[startIndex+1]._position = fge::Vector2f(0, bounds._height);
        this->g_instancesVertices[startIndex+2]._position = fge::Vector2f(bounds._width, 0);
        this->g_instancesVertices[startIndex+3]._position = fge::Vector2f(bounds._width, bounds._height);
    }
}

void ObjSpriteBatches::updateTexCoords(std::size_t index) const
{
    if (index < this->g_spriteCount)
    {
        const auto rect = this->g_texture.getData()->_texture->normalizeTextureRect(this->g_instancesTextureRect[index]);
        const std::size_t startIndex = index*4;

        this->g_instancesVertices[startIndex]._texCoords = fge::Vector2f(rect._x, rect._y);
        this->g_instancesVertices[startIndex+1]._texCoords = fge::Vector2f(rect._x, rect._y+rect._height);
        this->g_instancesVertices[startIndex+2]._texCoords = fge::Vector2f(rect._x+rect._width, rect._y);
        this->g_instancesVertices[startIndex+3]._texCoords = fge::Vector2f(rect._x+rect._width, rect._y+rect._height);
    }
}
void ObjSpriteBatches::updateBuffers() const
{
    if (this->g_needBuffersUpdate)
    {
        this->g_needBuffersUpdate = false;

        if (this->g_instancesVertices.getCount() != this->g_spriteCount*4)
        {
            this->g_instancesVertices.create(*fge::vulkan::GlobalContext, this->g_spriteCount*4,
                                             VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, fge::vulkan::BufferTypes::LOCAL);

            const std::size_t minUboAlignment = fge::vulkan::GlobalContext->getPhysicalDevice().getMinUniformBufferOffsetAlignment();
            std::size_t dynamicAlignment = fge::TransformUboData::uboSize;
            if (minUboAlignment > 0)
            {
                dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
            }

            this->g_instancesTransform.create(*fge::vulkan::GlobalContext, dynamicAlignment*this->g_spriteCount);

            this->g_instancesTransformData.reset(reinterpret_cast<fge::TransformUboData*>(fge::AlignedAlloc(dynamicAlignment*this->g_spriteCount, dynamicAlignment)));

            const fge::vulkan::DescriptorSet::Descriptor descriptor{this->g_instancesTransform,
                                                                    FGE_VULKAN_TRANSFORM_BINDING,
                                                                    fge::vulkan::DescriptorSet::Descriptor::BufferTypes::DYNAMIC,
                                                                    fge::TransformUboData::uboSize};
            this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);

            for (std::size_t i=0; i<this->g_spriteCount; ++i)
            {
                this->updatePositions(i);
                this->updateTexCoords(i);
            }
        }
    }
}

} // namespace fge
