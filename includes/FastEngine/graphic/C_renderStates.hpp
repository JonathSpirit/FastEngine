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

#ifndef _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED

#include "FastEngine/C_texture.hpp"
#include "FastEngine/textureType.hpp"
#include "FastEngine/vulkan/C_blendMode.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "glm/glm.hpp"

namespace fge
{

namespace vulkan
{

class VertexBuffer;
class IndexBuffer;

} // namespace vulkan

class Transform;

/**
 * \class RenderResourceTransform
 * \ingroup graphics
 * \brief Resource containing transform information for rendering
 *
 * A transform is a default uniform buffer containing a model matrix and a view matrix. It is generally used in every
 * object that needs to be rendered.
 * The transform is optional, if set to \b nullptr, no transform descriptor will be bound and the user have to
 * provide there own descriptors in RenderResourceInstances (dynamic descriptors) or in RenderResourceDescriptors.
 *
 * The transform descriptor is bound to the FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TRANSFORM set, binding 0.
 */
class RenderResourceTransform
{
public:
    constexpr RenderResourceTransform() = default;
    explicit constexpr RenderResourceTransform(fge::Transform const* transform) :
            g_transform(transform)
    {}

    [[nodiscard]] fge::Transform const* get() const { return this->g_transform; }
    void set(fge::Transform const* transform) { this->g_transform = transform; }

private:
    fge::Transform const* g_transform{nullptr};
};
/**
 * \class RenderResourceInstances
 * \ingroup graphics
 * \brief Resource containing instances information for rendering
 *
 * Texture indices are used to select the texture to use for each instance, the index refers to a texture
 * in RenderResourceTextures. If the index is out of range, the texture will be replaced by a bad texture.
 * This parameter is optional, if set to \b nullptr, all textures in RenderResourceTextures will be bound to each instance.
 *
 * Vertex count is used to select the number of vertices to use for each instance. If the vertex count is 0, the vertex count
 * will be set to the vertex count of the vertex buffer.
 *
 * Dynamic descriptors are used to set dynamic descriptors for each instance. Dynamic descriptors are used to set dynamic uniform buffers.
 * This parameter is optional, if set to \b nullptr, no dynamic descriptors will be set.
 */
class RenderResourceInstances
{
public:
    constexpr RenderResourceInstances() = default;

    constexpr void setInstancesCount(uint32_t count) { this->g_count = count; }

    constexpr void setTextureIndices(uint32_t const* textureIndices) { this->g_textureIndices = textureIndices; }

    constexpr void setDynamicDescriptors(fge::vulkan::DescriptorSet const* dynamicDescriptors,
                                         uint32_t const* dynamicBufferSizes,
                                         uint32_t const* dynamicSets,
                                         uint32_t count)
    {
        this->g_dynamicDescriptors = dynamicDescriptors;
        this->g_dynamicBufferSizes = dynamicBufferSizes;
        this->g_dynamicSets = dynamicSets;
        this->g_dynamicCount = count;

        assert(count == 0 || dynamicDescriptors != nullptr);
        assert(count == 0 || dynamicBufferSizes != nullptr);
        assert(count == 0 || dynamicSets != nullptr);
    }

    constexpr void setVertexCount(uint32_t count) { this->g_vertexCount = count; }

    [[nodiscard]] constexpr uint32_t getInstancesCount() const { return this->g_count; }

    [[nodiscard]] constexpr uint32_t const* getTextureIndices() const { return this->g_textureIndices; }
    [[nodiscard]] constexpr uint32_t getTextureIndices(uint32_t index) const { return this->g_textureIndices[index]; }

    [[nodiscard]] constexpr fge::vulkan::DescriptorSet const* getDynamicDescriptors(uint32_t index = 0) const
    {
        return this->g_dynamicDescriptors + index;
    }

    [[nodiscard]] constexpr uint32_t const* getDynamicBufferSizes() const { return this->g_dynamicBufferSizes; }
    [[nodiscard]] constexpr uint32_t getDynamicBufferSizes(uint32_t index) const
    {
        return this->g_dynamicBufferSizes[index];
    }

    [[nodiscard]] constexpr uint32_t const* getDynamicSets() const { return this->g_dynamicSets; }
    [[nodiscard]] constexpr uint32_t getDynamicSets(uint32_t index) const { return this->g_dynamicSets[index]; }

    [[nodiscard]] constexpr uint32_t getDynamicCount() const { return this->g_dynamicCount; }

    [[nodiscard]] constexpr uint32_t getVertexCount() const { return this->g_vertexCount; }

private:
    uint32_t g_count{1};
    uint32_t const* g_textureIndices{nullptr};

    fge::vulkan::DescriptorSet const* g_dynamicDescriptors{nullptr};
    uint32_t g_dynamicCount{0};
    uint32_t const* g_dynamicBufferSizes{nullptr};
    uint32_t const* g_dynamicSets{nullptr};

    uint32_t g_vertexCount{0};
};
/**
 * \class RenderResourceTextures
 * \ingroup graphics
 * \brief Resource containing textures information for rendering
 *
 * For new RenderStates by default, the textures are set to nullptr and the count to 0.
 * When setting \b nullptr textures, the provided count is ignored and set to 0.
 */
class RenderResourceTextures
{
public:
    /**
     * \enum PtrTypes
     * \brief The type of pointer used to store the textures
     *
     * There is 2 types of pointers:
     * - TEXTURE: fge::Texture const*
     * - TEXTURE_IMAGE: fge::TextureType const* (fge::vulkan::TextureImage const*)
     */
    enum class PtrTypes
    {
        TEXTURE,
        TEXTURE_IMAGE
    };

    template<class...>
    static constexpr std::false_type always_false{};

    constexpr RenderResourceTextures() = default;
    template<class TTexture>
    constexpr RenderResourceTextures(TTexture const* textures, uint32_t count) :
            g_textures(textures),
            g_count(textures == nullptr ? 0 : count)
    {
        if constexpr (std::is_same_v<TTexture, fge::TextureType> || std::is_same_v<TTexture, std::nullptr_t>)
        {
            this->g_ptrType = PtrTypes::TEXTURE_IMAGE;
        }
        else if constexpr (std::is_same_v<TTexture, fge::Texture>)
        {
            this->g_ptrType = PtrTypes::TEXTURE;
        }
        else
        {
            static_assert(always_false<TTexture>, "Can't set this type of texture pointer !");
        }
    }

    template<class TTexture>
    constexpr void set(TTexture const* textures, uint32_t count)
    {
        this->g_textures = textures;
        this->g_count = textures == nullptr ? 0 : count;

        if constexpr (std::is_same_v<TTexture, fge::TextureType> || std::is_same_v<TTexture, std::nullptr_t>)
        {
            this->g_ptrType = PtrTypes::TEXTURE_IMAGE;
        }
        else if constexpr (std::is_same_v<TTexture, fge::Texture>)
        {
            this->g_ptrType = PtrTypes::TEXTURE;
        }
        else
        {
            static_assert(always_false<TTexture>, "Can't set this type of texture pointer !");
        }
    }
    [[nodiscard]] constexpr uint32_t getCount() const { return this->g_count; }
    [[nodiscard]] constexpr PtrTypes getPointerType() const { return this->g_ptrType; }

    template<PtrTypes TTextureType>
    constexpr auto const* get(uint32_t index) const
    {
        if constexpr (TTextureType == PtrTypes::TEXTURE_IMAGE)
        {
            return static_cast<fge::TextureType const*>(this->g_textures) + index;
        }
        else
        {
            return static_cast<fge::Texture const*>(this->g_textures) + index;
        }
    }

    template<PtrTypes TTextureType>
    [[nodiscard]] constexpr fge::TextureType const* getTextureImage(uint32_t index) const
    {
        if constexpr (TTextureType == PtrTypes::TEXTURE_IMAGE)
        {
            return static_cast<fge::TextureType const*>(this->g_textures) + index;
        }
        else
        {
#ifndef FGE_DEF_SERVER
            return static_cast<fge::Texture const*>(this->g_textures)[index].getData()->_texture.get();
#else
            return nullptr;
#endif
        }
    }

private:
    void const* g_textures{nullptr};
    PtrTypes g_ptrType{PtrTypes::TEXTURE_IMAGE};
    uint32_t g_count{0};
};
/**
 * \class RenderResourceDescriptors
 * \ingroup graphics
 * \brief Resource containing descriptor sets information for rendering
 *
 * When using descriptors and textures/transform resources, be careful to not use the same descriptor set for both.
 * - Transform use descriptor set FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TRANSFORM, at vertex shader stage.
 * - Textures use descriptor set FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TEXTURE, at fragment shader stage.
 *   if there is multiple textures, they are bind in increasing order, starting at FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TEXTURE.
 * On both cases, the binding is 0.
 */
class RenderResourceDescriptors
{
public:
    constexpr RenderResourceDescriptors() = default;

    constexpr RenderResourceDescriptors(fge::vulkan::DescriptorSet const* descriptors,
                                        uint32_t const* sets,
                                        uint32_t count) :
            g_descriptors(descriptors),
            g_sets(sets),
            g_count(count)
    {
        assert(count == 0 || descriptors != nullptr);
        assert(count == 0 || sets != nullptr);
    }

    constexpr void set(fge::vulkan::DescriptorSet const* descriptors, uint32_t const* sets, uint32_t count)
    {
        this->g_descriptors = descriptors;
        this->g_sets = sets;
        this->g_count = count;

        assert(count == 0 || descriptors != nullptr);
        assert(count == 0 || sets != nullptr);
    }

    [[nodiscard]] constexpr uint32_t getCount() const { return this->g_count; }

    [[nodiscard]] constexpr fge::vulkan::DescriptorSet const* getDescriptorSet(uint32_t index) const
    {
        return this->g_descriptors + index;
    }
    [[nodiscard]] constexpr uint32_t getSet(uint32_t index) const { return this->g_sets[index]; }

private:
    fge::vulkan::DescriptorSet const* g_descriptors{nullptr};
    uint32_t const* g_sets{nullptr};
    uint32_t g_count{0};
};

/**
 * \class RenderStates
 * \ingroup graphics
 * \brief The RenderStates class contains all the information needed to render something.
 *
 * This class cannot be copied, user must use copy() to create a new RenderStates.
 */
class RenderStates
{
public:
    RenderStates() = default;
    RenderStates(const RenderStates& r) = delete;
    RenderStates(RenderStates&& r) noexcept = default;
    explicit RenderStates(const fge::Transform* transform, const fge::TextureType* textureImage = nullptr) :
            _resTransform{transform},
            _resTextures{textureImage, 1}
    {}
    RenderStates(const fge::Transform* transform,
                 const fge::vulkan::VertexBuffer* vertexBuffer,
                 const fge::TextureType* textureImage = nullptr,
                 const fge::vulkan::BlendMode& blendMode = {}) :
            _resTransform{transform},
            _resTextures{textureImage, 1},
            _vertexBuffer(vertexBuffer),
            _blendMode(blendMode)
    {}

    [[nodiscard]] RenderStates copy(const fge::Transform* transform,
                                    const fge::TextureType* textureImage = nullptr) const
    {
        return RenderStates{transform, nullptr, textureImage, this->_blendMode};
    }

    RenderStates& operator=(const RenderStates& r) = delete;
    RenderStates& operator=(RenderStates&& r) noexcept = default;

    RenderResourceTransform _resTransform{};
    RenderResourceTextures _resTextures{};
    RenderResourceInstances _resInstances{};
    RenderResourceDescriptors _resDescriptors{};

    const fge::vulkan::VertexBuffer* _vertexBuffer{nullptr};
    const fge::vulkan::IndexBuffer* _indexBuffer{nullptr};
    fge::vulkan::BlendMode _blendMode{};
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED
