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

void* alignedAlloc(size_t size, size_t alignment) ///TODO: move that to extra_function
{
    void* data = nullptr;

#if defined(_MSC_VER) || defined(__MINGW32__)
    data = _aligned_malloc(size, alignment);
#else
    int res = posix_memalign(&data, alignment, size);
    if (res != 0)
    {
        data = nullptr;
    }
#endif
    return data;
}
void alignedFree(void* data) ///TODO: move that to extra_function
{
    if (data != nullptr)
    {
#if	defined(_MSC_VER) || defined(__MINGW32__)
        _aligned_free(data);
#else
        free(data);
#endif
    }
}

ObjSpriteBatches::ObjSpriteBatches() :
        g_instancesTransformData(nullptr),
        g_spriteCount(0),
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
ObjSpriteBatches::~ObjSpriteBatches()  ///TODO: make it safer
{
    alignedFree(this->g_instancesTransformData);
}

void ObjSpriteBatches::setTexture(fge::Texture texture)
{
    this->g_texture = std::move(texture);
}

fge::Transformable& ObjSpriteBatches::addSprite(const fge::RectInt& rectangle)
{ ///TODO: make it safer
    this->g_instancesTextureRect.push_back(rectangle);
    ++this->g_spriteCount;
    this->g_needBuffersUpdate = true;
    return this->g_instancesTransformable.emplace_back();
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
const fge::RectInt& ObjSpriteBatches::getTextureRect() const
{
    //return this->g_textureRect;
}

fge::Color ObjSpriteBatches::getColor() const
{
    //return fge::Color(this->g_vertices.getVertices()[0]._color);
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

    this->g_instancesTransform.copyData(this->g_instancesTransformData, this->g_instancesTransform.getBufferSize()/*sizeof(TransformData)*this->g_spriteCount*/);

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
fge::RectFloat ObjSpriteBatches::getLocalBounds() const
{
    return fge::Object::getLocalBounds();
}
fge::RectFloat ObjSpriteBatches::getLocalBounds(std::size_t index) const
{
    if (index < this->g_spriteCount)
    {
        const auto width = static_cast<float>(this->g_instancesTextureRect[index]._width);
        const auto height = static_cast<float>(this->g_instancesTextureRect[index]._height);

        return {{0.f, 0.f}, {width, height}};
    }
    return {{0.f, 0.f}, {1.0f, 1.0f}};
}

void ObjSpriteBatches::updatePositions(std::size_t index) const
{
    if (index < this->g_spriteCount)
    {
        const fge::RectFloat bounds = this->getLocalBounds(index);
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
            std::size_t dynamicAlignment = sizeof(TransformData);
            if (minUboAlignment > 0)
            {
                dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
            }

            this->g_instancesTransform.create(*fge::vulkan::GlobalContext, dynamicAlignment*this->g_spriteCount);

            alignedFree(this->g_instancesTransformData);
            this->g_instancesTransformData = (TransformData*)alignedAlloc(dynamicAlignment*this->g_spriteCount, dynamicAlignment);///.resize(this->g_spriteCount);

            const fge::vulkan::DescriptorSet::Descriptor descriptor{this->g_instancesTransform,
                                                                    FGE_VULKAN_TRANSFORM_BINDING,
                                                                    fge::vulkan::DescriptorSet::Descriptor::BufferTypes::DYNAMIC,
                                                                    sizeof(TransformData)};
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
