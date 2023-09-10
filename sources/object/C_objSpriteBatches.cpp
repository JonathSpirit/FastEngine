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
#include "FastEngine/manager/shader_manager.hpp"

#define FGE_OBJSPRITEBATCHES_DESCRIPTORSET_INSTANCES 0
#define FGE_OBJSPRITEBATCHES_DESCRIPTORSET_TEXTURES 1

namespace fge
{

namespace
{

#ifndef FGE_DEF_SERVER
void DefaultGraphicPipelineBatchesWithTexture_constructor(const fge::vulkan::Context* context,
                                                          const fge::RenderTarget::GraphicPipelineKey& key,
                                                          fge::vulkan::GraphicPipeline* graphicPipeline)
{
    graphicPipeline->setShader(fge::shader::GetShader(FGE_OBJSPRITEBATCHES_SHADER_FRAGMENT)->_shader);
    graphicPipeline->setShader(fge::shader::GetShader(FGE_OBJSPRITEBATCHES_SHADER_VERTEX)->_shader);
    graphicPipeline->setBlendMode(key._blendMode);
    graphicPipeline->setPrimitiveTopology(key._topology);

    auto& layout = context->getCacheLayout(FGE_OBJSPRITEBATCHES_LAYOUT);
    if (layout.getLayout() == VK_NULL_HANDLE)
    {
        layout.create({fge::vulkan::CreateSimpleLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                              VK_SHADER_STAGE_VERTEX_BIT)});
    }

    auto& textureLayout = fge::vulkan::GlobalContext->getCacheLayout(FGE_OBJSPRITEBATCHES_LAYOUT_TEXTURES);
    if (textureLayout.getLayout() == VK_NULL_HANDLE)
    {
        VkDescriptorBindingFlagsEXT const bindingFlags[] = {VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT};
        textureLayout.create({VkDescriptorSetLayoutBinding{.binding = 0,
                                                           .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                           .descriptorCount = FGE_OBJSPRITEBATCHES_MAXIMUM_TEXTURES,
                                                           .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                           .pImmutableSamplers = nullptr}},
                             bindingFlags);
    }

    graphicPipeline->setDescriptorSetLayouts({layout.getLayout(), textureLayout.getLayout()});
}
void DefaultGraphicPipelineBatches_constructor(const fge::vulkan::Context* context,
                                               const fge::RenderTarget::GraphicPipelineKey& key,
                                               fge::vulkan::GraphicPipeline* graphicPipeline)
{
    graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT)->_shader); ///TODO
    graphicPipeline->setShader(fge::shader::GetShader(FGE_OBJSPRITEBATCHES_SHADER_VERTEX)->_shader);
    graphicPipeline->setBlendMode(key._blendMode);
    graphicPipeline->setPrimitiveTopology(key._topology);

    auto& layout = context->getCacheLayout(FGE_OBJSPRITEBATCHES_LAYOUT);
    if (layout.getLayout() == VK_NULL_HANDLE)
    {
        layout.create({fge::vulkan::CreateSimpleLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                              VK_SHADER_STAGE_VERTEX_BIT)});
    }

    graphicPipeline->setDescriptorSetLayouts({layout.getLayout()});
}
#endif //FGE_DEF_SERVER

} // end namespace

ObjSpriteBatches::ObjSpriteBatches() :
        g_instancesTransformDataCapacity(0),
        g_instancesTransform(*fge::vulkan::GlobalContext),
        g_needBuffersUpdate(true)
{
    this->g_instancesVertices.create(*fge::vulkan::GlobalContext, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                                     fge::vulkan::BufferTypes::LOCAL);
}
ObjSpriteBatches::ObjSpriteBatches(const ObjSpriteBatches& r) :
        fge::Object(r),
        g_textures(r.g_textures),
        g_instancesData(r.g_instancesData),
        g_instancesTransformDataCapacity(0),
        g_instancesTransform(r.g_instancesTransform.getContext()),
        g_instancesVertices(r.g_instancesVertices),
        g_needBuffersUpdate(true)
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
const fge::Texture& ObjSpriteBatches::getTexture(std::size_t index) const
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
fge::Transformable& ObjSpriteBatches::addSprite(const fge::RectInt& rectangle, uint32_t textureIndex)
{
    auto& transformable = this->g_instancesData.emplace_back(rectangle, textureIndex)._transformable;
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
        return fge::Color(this->g_instancesVertices[index * 4]._color);
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
const fge::Transformable* ObjSpriteBatches::getTransformable(std::size_t index) const
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
    view->_transform = target.getView().getTransform();

    //Update all model matrices
    for (std::size_t i = 0; i < this->g_instancesData.size(); ++i)
    {
        auto* instance = static_cast<InstanceDataBuffer*>(this->g_instancesTransform.getBufferMapped()) + i + 1;

        instance->_textureIndex = this->g_instancesData[i]._textureIndex;
        if (states._resTransform.get() != nullptr)
        {
            instance->_transform = states._resTransform.get()->getData()._modelTransform *
                                   this->g_instancesData[i]._transformable.getTransform();
        }
        else
        {
            instance->_transform = this->g_instancesData[i]._transformable.getTransform();
        }
    }

    auto copyStates = states.copy(nullptr, nullptr);

    copyStates._blendMode = states._blendMode;

    copyStates._resInstances.setInstancesCount(this->g_instancesData.size(), false);
    copyStates._resInstances.setVertexCount(4);

    const uint32_t sets[] = {0, 1};
    copyStates._resDescriptors.set(this->g_descriptorSets, sets, 2);

    copyStates._vertexBuffer = &this->g_instancesVertices;

    //copyStates._resTextures.set(this->g_textures.data(), this->g_textures.size());

    const bool haveTexture = !this->g_textures.empty();

    const fge::RenderTarget::GraphicPipelineKey graphicPipelineKey{
            this->g_instancesVertices.getPrimitiveTopology(), copyStates._blendMode,
            uint8_t(haveTexture ? FGE_OBJSPRITEBATCHES_ID_TEXTURE : FGE_OBJSPRITEBATCHES_ID)};

    auto* graphicPipeline = target.getGraphicPipeline(FGE_OBJSPRITEBATCHES_PIPELINE_CACHE_NAME, graphicPipelineKey,
                                                      haveTexture ? DefaultGraphicPipelineBatchesWithTexture_constructor
                                                                  : DefaultGraphicPipelineBatches_constructor);

    target.draw(copyStates, graphicPipeline);
}
#endif

void ObjSpriteBatches::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);
}
void ObjSpriteBatches::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);
}

void ObjSpriteBatches::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);
}
void ObjSpriteBatches::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);
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

void ObjSpriteBatches::updatePositions(std::size_t index)
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

void ObjSpriteBatches::updateTexCoords(std::size_t index)
{
    if (index < this->g_instancesData.size())
    {
        const auto textureIndex = this->g_instancesData[index]._textureIndex;
        const fge::TextureType* texture = fge::texture::GetBadTexture()->_texture.get();
        if (textureIndex < this->g_textures.size())
        {
            texture = this->g_textures[textureIndex].getData()->_texture.get();
        }

        const auto rect = texture->normalizeTextureRect(this->g_instancesData[index]._textureRect);
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

        if (this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_INSTANCES].get() == VK_NULL_HANDLE)
        {
            auto& layout = fge::vulkan::GlobalContext->getCacheLayout(FGE_OBJSPRITEBATCHES_LAYOUT);
            if (layout.getLayout() == VK_NULL_HANDLE)
            {
                layout.create({fge::vulkan::CreateSimpleLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                                      VK_SHADER_STAGE_VERTEX_BIT)});
            }

            this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_INSTANCES] =
                    fge::vulkan::GlobalContext->getMultiUseDescriptorPool()
                            .allocateDescriptorSet(layout.getLayout())
                            .value();
        }

        if (!this->g_instancesData.empty() && this->g_instancesData.size() > this->g_instancesTransformDataCapacity)
        {
            this->g_instancesTransformDataCapacity = this->g_instancesData.size();

            this->g_instancesTransform.create(sizeof(InstanceDataBuffer) * (this->g_instancesData.size() + 1), true);

            const fge::vulkan::DescriptorSet::Descriptor descriptor{
                    this->g_instancesTransform, 0, fge::vulkan::DescriptorSet::Descriptor::BufferTypes::STORAGE,
                    this->g_instancesTransform.getBufferSize()};
            this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_INSTANCES].updateDescriptorSet(&descriptor, 1);
        }
    }
}
void ObjSpriteBatches::updateTextures(bool sizeHasChanged)
{
    if (sizeHasChanged || this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_TEXTURES].get() == VK_NULL_HANDLE)
    {
        auto& layout = fge::vulkan::GlobalContext->getCacheLayout(FGE_OBJSPRITEBATCHES_LAYOUT_TEXTURES);
        if (layout.getLayout() == VK_NULL_HANDLE)
        {
            VkDescriptorBindingFlagsEXT const bindingFlags[] = {
                    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT};
            layout.create({VkDescriptorSetLayoutBinding{.binding = 0,
                                                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                        .descriptorCount = FGE_OBJSPRITEBATCHES_MAXIMUM_TEXTURES,
                                                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                        .pImmutableSamplers = nullptr}},
                          bindingFlags);
        }

        this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_TEXTURES] =
                fge::vulkan::GlobalContext->getMultiUseDescriptorPool()
                        .allocateDescriptorSet(layout.getLayout(), this->g_textures.size())
                        .value();
    }

#ifndef FGE_DEF_SERVER
    std::vector<fge::vulkan::DescriptorSet::Descriptor> descriptors;
    descriptors.reserve(this->g_textures.size());
    for (std::size_t i = 0; i < this->g_textures.size(); ++i)
    {
        descriptors.emplace_back(*this->g_textures[i].getData()->_texture, 0);
        descriptors.back()._dstArrayElement = i;
    }

    this->g_descriptorSets[FGE_OBJSPRITEBATCHES_DESCRIPTORSET_TEXTURES].updateDescriptorSet(descriptors.data(),
                                                                                            descriptors.size());
#endif //FGE_DEF_SERVER
}

} // namespace fge
