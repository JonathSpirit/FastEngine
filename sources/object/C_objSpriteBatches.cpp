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

#include "FastEngine/object/C_objSpriteBatches.hpp"
#include "FastEngine/manager/shader_manager.hpp"

#define FGE_OBJSPRITEBATCHES_DESCRIPTORSET_INSTANCES 0
#define FGE_OBJSPRITEBATCHES_DESCRIPTORSET_TEXTURES 1

namespace fge
{

ObjSpriteBatches::ObjSpriteBatches() :
        g_instancesTransform(fge::vulkan::GetActiveContext(), vulkan::UniformBuffer::Types::STORAGE_BUFFER),
        g_instancesIndirectCommands(fge::vulkan::GetActiveContext(), vulkan::UniformBuffer::Types::INDIRECT_BUFFER),
        g_instancesVertices(fge::vulkan::GetActiveContext()),
        g_needBuffersUpdate(true),
        g_featureMultiDrawIndirect(
                fge::vulkan::GetActiveContext().getLogicalDevice().getEnabledFeatures().multiDrawIndirect == VK_TRUE)
{
    this->g_instancesVertices.create(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, fge::vulkan::BufferTypes::LOCAL);
}
ObjSpriteBatches::ObjSpriteBatches(ObjSpriteBatches const& r) :
        fge::Object(r),
        g_textures(r.g_textures),
        g_instancesData(r.g_instancesData),
        g_instancesTransform(r.g_instancesTransform.getContext(), vulkan::UniformBuffer::Types::STORAGE_BUFFER),
        g_instancesIndirectCommands(r.g_instancesIndirectCommands),
        g_instancesVertices(r.g_instancesVertices),
        g_needBuffersUpdate(true),
        g_featureMultiDrawIndirect(r.g_featureMultiDrawIndirect)
{}
ObjSpriteBatches::ObjSpriteBatches(fge::Texture texture) :
        ObjSpriteBatches()
{
    this->addTexture(std::move(texture));
}

void ObjSpriteBatches::addTexture(fge::Texture texture)
{
    this->g_textures.push_back(std::move(texture));
    this->updateTextures(true);
}
void ObjSpriteBatches::setTexture(std::size_t index, fge::Texture texture)
{
    if (index < this->g_textures.size())
    {
        this->g_textures[index] = std::move(texture);
        this->updateTextures(false);
    }
}
void ObjSpriteBatches::setTexture(fge::Texture texture)
{
    bool sizeChanged = this->g_textures.size() != 1;
    this->g_textures.resize(1);
    this->g_textures.front() = std::move(texture);
    this->updateTextures(sizeChanged);
}
fge::Texture const& ObjSpriteBatches::getTexture(std::size_t index) const
{
    return this->g_textures[index];
}
std::size_t ObjSpriteBatches::getTextureCount() const
{
    return this->g_textures.size();
}
void ObjSpriteBatches::clearTexture()
{
    this->g_textures.clear();
}

void ObjSpriteBatches::clear()
{
    this->g_instancesData.clear();
    this->g_instancesVertices.clear();
    this->g_needBuffersUpdate = true;
}
fge::Transformable& ObjSpriteBatches::addSprite(fge::RectInt const& rectangle, uint32_t textureIndex)
{
    auto& transformable = this->g_instancesData.emplace_back(rectangle, textureIndex)._transformable;
    this->g_instancesVertices.resize(this->g_instancesVertices.getCount() + FGE_OBJSPRITEBATCHES_VERTEX_COUNT);
    this->updatePositions(this->g_instancesData.size() - 1);
    this->updateTexCoords(this->g_instancesData.size() - 1);
    this->g_needBuffersUpdate = true;
    return transformable;
}
void ObjSpriteBatches::resize(std::size_t size)
{
    std::size_t const oldSize = this->g_instancesData.size();

    this->g_instancesData.resize(size);
    this->g_instancesVertices.resize(size * FGE_OBJSPRITEBATCHES_VERTEX_COUNT);

    if (size > oldSize)
    {
        for (std::size_t i = oldSize; i < size; ++i)
        {
            this->updatePositions(i);
            this->updateTexCoords(i);
        }
    }
}
void ObjSpriteBatches::setTextureRect(std::size_t index, fge::RectInt const& rectangle)
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

void ObjSpriteBatches::setColor(std::size_t index, fge::Color const& color)
{
    if (index < this->g_instancesData.size())
    {
        std::size_t const startIndex = index * FGE_OBJSPRITEBATCHES_VERTEX_COUNT;

        this->g_instancesVertices[startIndex]._color = color;
        this->g_instancesVertices[startIndex + 1]._color = color;
        this->g_instancesVertices[startIndex + 2]._color = color;
        this->g_instancesVertices[startIndex + 3]._color = color;
    }
}

void ObjSpriteBatches::setSpriteTexture(std::size_t spriteIndex, uint32_t textureIndex)
{
    if (spriteIndex < this->g_instancesData.size())
    {
        this->g_instancesData[spriteIndex]._textureIndex = textureIndex;
        this->updateTexCoords(spriteIndex);
    }
}

std::size_t ObjSpriteBatches::getSpriteCount() const
{
    return this->g_instancesData.size();
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
        return fge::Color(this->g_instancesVertices[index * FGE_OBJSPRITEBATCHES_VERTEX_COUNT]._color);
    }
    return std::nullopt;
}

fge::Transformable* ObjSpriteBatches::getTransformable(std::size_t index)
{
    if (index < this->g_instancesData.size())
    {
        return &this->g_instancesData[index]._transformable;
    }
    return nullptr;
}
fge::Transformable const* ObjSpriteBatches::getTransformable(std::size_t index) const
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

    //Update the view matrix (always the first element of the buffer)
    auto* view = static_cast<InstanceDataBuffer*>(this->g_instancesTransform.getBufferMapped());
    view->_transform = target.getView().getProjection() * target.getView().getTransform();

    fge::TransformUboData const* parentTransform = target.getGlobalTransform(states._resTransform);

    //Update all model matrices
    for (std::size_t i = 0; i < this->g_instancesData.size(); ++i)
    {
        auto* instance = static_cast<InstanceDataBuffer*>(this->g_instancesTransform.getBufferMapped()) + i + 1;

        instance->_textureIndex = this->g_instancesData[i]._textureIndex;
        if (parentTransform != nullptr)
        {
            instance->_transform = parentTransform->_modelTransform * this->getTransform() *
                                   this->g_instancesData[i]._transformable.getTransform();
        }
        else
        {
            instance->_transform = this->getTransform() * this->g_instancesData[i]._transformable.getTransform();
        }
    }

    auto copyStates = states.copy();

    copyStates._blendMode = states._blendMode;

    copyStates._resInstances.setInstancesCount(this->g_instancesData.size(), false);
    copyStates._resInstances.setVertexCount(FGE_OBJSPRITEBATCHES_VERTEX_COUNT);
    if (this->g_featureMultiDrawIndirect)
    {
        copyStates._resInstances.setIndirectBuffer(this->g_instancesIndirectCommands.getBuffer());
    }

    uint32_t const sets[] = {0, 1};
    copyStates._resDescriptors.set(this->g_descriptorSets, sets, 2);

    copyStates._vertexBuffer = &this->g_instancesVertices;

    bool const haveTexture = !this->g_textures.empty();

    copyStates._shaderVertex = fge::shader::gManager.getElement(FGE_OBJSPRITEBATCHES_SHADER_VERTEX)->_ptr.get();
    //TODO: FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT
    copyStates._shaderFragment = fge::shader::gManager
                                         .getElement(haveTexture ? FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT
                                                                 : FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT)
                                         ->_ptr.get();

    target.draw(copyStates);
}
#endif

void ObjSpriteBatches::save(nlohmann::json& jsonObject)
{
    fge::Object::save(jsonObject);
}
void ObjSpriteBatches::load(nlohmann::json& jsonObject, std::filesystem::path const& filePath)
{
    fge::Object::load(jsonObject, filePath);
}

void ObjSpriteBatches::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);
}
void ObjSpriteBatches::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);
}

char const* ObjSpriteBatches::getClassName() const
{
    return FGE_OBJSPRITEBATCHES_CLASSNAME;
}
char const* ObjSpriteBatches::getReadableClassName() const
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
        auto const width = static_cast<float>(this->g_instancesData[index]._textureRect._width);
        auto const height = static_cast<float>(this->g_instancesData[index]._textureRect._height);

        return fge::RectFloat{{0.f, 0.f}, {width, height}};
    }
    return std::nullopt;
}

void ObjSpriteBatches::updatePositions(std::size_t index)
{
    if (index < this->g_instancesData.size())
    {
        fge::RectFloat const bounds = this->getLocalBounds(index).value();
        std::size_t const startIndex = index * FGE_OBJSPRITEBATCHES_VERTEX_COUNT;

        this->g_instancesVertices[startIndex]._position = fge::Vector2f(0, 0);
        this->g_instancesVertices[startIndex + 1]._position = fge::Vector2f(0, bounds._height);
        this->g_instancesVertices[startIndex + 2]._position = fge::Vector2f(bounds._width, 0);
        this->g_instancesVertices[startIndex + 3]._position = fge::Vector2f(bounds._width, bounds._height);
    }
}

void ObjSpriteBatches::updateTexCoords(std::size_t index)
{
    if (index < this->g_instancesData.size())
    {
        auto const textureIndex = this->g_instancesData[index]._textureIndex;
        fge::TextureType const* texture = texture::gManager.getBadElement()->_ptr.get();
        if (textureIndex < this->g_textures.size())
        {
            texture = this->g_textures[textureIndex].getSharedData().get();
        }

        auto const rect = texture->normalizeTextureRect(this->g_instancesData[index]._textureRect);
        std::size_t const startIndex = index * FGE_OBJSPRITEBATCHES_VERTEX_COUNT;

        this->g_instancesVertices[startIndex]._texCoords = fge::Vector2f(rect._x, rect._y);
        this->g_instancesVertices[startIndex + 1]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
        this->g_instancesVertices[startIndex + 2]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
        this->g_instancesVertices[startIndex + 3]._texCoords =
                fge::Vector2f(rect._x + rect._width, rect._y + rect._height);
    }
}
void ObjSpriteBatches::updateBuffers() const
{
    using namespace fge::vulkan;

    if (this->g_needBuffersUpdate)
    {
        this->g_needBuffersUpdate = false;

        if (this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_INSTANCES].get() == VK_NULL_HANDLE)
        {
            this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_INSTANCES] =
                    GetActiveContext().createDescriptorSet(FGE_OBJSPRITEBATCHES_SHADER_VERTEX, 0).value();
        }

        if (!this->g_instancesData.empty())
        {
            this->g_instancesTransform.resize(sizeof(InstanceDataBuffer) * (this->g_instancesData.size() + 1));

            if (this->g_featureMultiDrawIndirect)
            {
                this->g_instancesIndirectCommands.resize(sizeof(VkDrawIndirectCommand) * this->g_instancesData.size());
            }

            DescriptorSet::Descriptor const descriptor{this->g_instancesTransform, 0,
                                                       DescriptorSet::Descriptor::BufferTypes::STORAGE,
                                                       this->g_instancesTransform.getBufferSize()};
            this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_INSTANCES].updateDescriptorSet(&descriptor, 1);
        }

        if (this->g_featureMultiDrawIndirect)
        { //Only for multiDrawIndirect feature
            //Fill indirect commands buffer
            for (std::size_t i = 0; i < this->g_instancesData.size(); ++i)
            {
                auto* command =
                        static_cast<VkDrawIndirectCommand*>(this->g_instancesIndirectCommands.getBufferMapped()) + i;
                command->vertexCount = FGE_OBJSPRITEBATCHES_VERTEX_COUNT;
                command->instanceCount = 1;
                command->firstVertex = i * FGE_OBJSPRITEBATCHES_VERTEX_COUNT;
                command->firstInstance = i;
            }
        }
    }
}
void ObjSpriteBatches::updateTextures(bool sizeHasChanged)
{
    using namespace fge::vulkan;

    if (sizeHasChanged || this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_TEXTURES].get() == VK_NULL_HANDLE)
    {
        this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_TEXTURES] =
                GetActiveContext()
                        .createDescriptorSet(FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT, 0, this->g_textures.size())
                        .value();
    }

#ifndef FGE_DEF_SERVER
    std::vector<DescriptorSet::Descriptor> descriptors;
    descriptors.reserve(this->g_textures.size());
    for (std::size_t i = 0; i < this->g_textures.size(); ++i)
    {
        descriptors.emplace_back(*this->g_textures[i].getSharedData(), 0);
        descriptors.back()._dstArrayElement = i;
    }

    this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_TEXTURES].updateDescriptorSet(descriptors.data(),
                                                                                            descriptors.size());
#endif //FGE_DEF_SERVER
}

} // namespace fge
