/*
 * Copyright 2024 Guillaume Guillet
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
#include "FastEngine/C_vector.hpp"
#include "FastEngine/textureType.hpp"
#include "FastEngine/vulkan/C_blendMode.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"

namespace fge
{

namespace vulkan
{

class Shader;
class VertexBuffer;
class IndexBuffer;

} // namespace vulkan

class Transform;
struct TransformUboData;

/**
 * \class RenderResourceTransform
 * \ingroup graphics
 * \brief Resource containing transform information for rendering
 *
 * A transform is a default uniform buffer containing a model matrix and a view matrix. It is generally used in every
 * object that needs to be rendered.
 * The transform is optional, if set to \b nullptr, no transform descriptor will be bound and the user have to
 * provide their own descriptors in RenderResourceInstances (dynamic descriptors) or in RenderResourceDescriptors.
 *
 * The transform descriptor is bound to the FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TRANSFORM set, binding 0.
 */
class RenderResourceTransform
{
public:
    enum class Configs
    {
        GLOBAL_TRANSFORMS_INDEX_IS_ADDED_TO_FIRST_INSTANCE,
        GLOBAL_TRANSFORMS_INDEX_OVERWRITE_FIRST_INSTANCE,
        GLOBAL_TRANSFORMS_INDEX_IS_IGNORED,

        DEFAULT = GLOBAL_TRANSFORMS_INDEX_OVERWRITE_FIRST_INSTANCE
    };

    constexpr RenderResourceTransform() = default;

    void set(fge::TransformUboData const& data) { this->g_transform = &data; }
    void set(uint32_t globalTransformsIndex, Configs config = Configs::DEFAULT)
    {
        this->g_transform = globalTransformsIndex;
        this->g_config = config;
    }
    [[nodiscard]] fge::TransformUboData const* getTransformData() const
    {
        return std::holds_alternative<fge::TransformUboData const*>(this->g_transform)
                       ? std::get<fge::TransformUboData const*>(this->g_transform)
                       : nullptr;
    }
    [[nodiscard]] std::optional<uint32_t> getGlobalTransformsIndex() const
    {
        return std::holds_alternative<uint32_t>(this->g_transform)
                       ? std::make_optional(std::get<uint32_t>(this->g_transform))
                       : std::nullopt;
    }
    [[nodiscard]] Configs getConfig() const { return this->g_config; }

private:
    std::variant<fge::TransformUboData const*, uint32_t> g_transform{nullptr};
    Configs g_config{Configs::DEFAULT};
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

    constexpr void setInstancesCount(uint32_t count, bool uniqueDrawCall)
    {
        this->g_count = count;
        this->g_uniqueDrawCall = uniqueDrawCall || (this->g_indirectBuffer != VK_NULL_HANDLE);
    }
    constexpr void setIndirectBuffer(VkBuffer buffer)
    {
        this->g_indirectBuffer = buffer;
        this->g_uniqueDrawCall = (buffer != VK_NULL_HANDLE);
    }

    constexpr void setTextureIndices(uint32_t const* textureIndices) { this->g_textureIndices = textureIndices; }

    constexpr void setDynamicDescriptors(fge::vulkan::DescriptorSet const* dynamicDescriptors,
                                         uint32_t const* dynamicBufferSizes,
                                         uint32_t const* dynamicBufferOffsets,
                                         uint32_t const* dynamicSets,
                                         uint32_t count)
    {
        this->g_dynamicDescriptors = dynamicDescriptors;
        this->g_dynamicBufferSizes = dynamicBufferSizes;
        this->g_dynamicBufferOffsets = dynamicBufferOffsets;
        this->g_dynamicSets = dynamicSets;
        this->g_dynamicCount = count;

        assert(count == 0 || dynamicDescriptors != nullptr);
        assert(count == 0 || dynamicBufferSizes != nullptr);
        assert(count == 0 || dynamicBufferOffsets != nullptr);
        assert(count == 0 || dynamicSets != nullptr);
    }

    constexpr void setVertexCount(uint32_t count) { this->g_vertexCount = count; }
    constexpr void setVertexOffset(uint32_t offset) { this->g_vertexOffset = offset; }

    /**
     * \brief Set the first instance value for the draw call
     *
     * Appliqued if hasUniqueDrawCall() is \b true without indirect buffer.
     * Appliqued has an offset if hasUniqueDrawCall() is \b false.
     *
     * \param firstInstance The first instance value
     */
    constexpr void setFirstInstance(uint32_t firstInstance) { this->g_firstInstance = firstInstance; }

    [[nodiscard]] constexpr uint32_t getInstancesCount() const { return this->g_count; }
    [[nodiscard]] constexpr bool hasUniqueDrawCall() const { return this->g_uniqueDrawCall; }
    [[nodiscard]] constexpr VkBuffer getIndirectBuffer() const { return this->g_indirectBuffer; }

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

    [[nodiscard]] constexpr uint32_t const* getDynamicBufferOffsets() const { return this->g_dynamicBufferOffsets; }
    [[nodiscard]] constexpr uint32_t getDynamicBufferOffsets(uint32_t index) const
    {
        return this->g_dynamicBufferOffsets[index];
    }

    [[nodiscard]] constexpr uint32_t const* getDynamicSets() const { return this->g_dynamicSets; }
    [[nodiscard]] constexpr uint32_t getDynamicSets(uint32_t index) const { return this->g_dynamicSets[index]; }

    [[nodiscard]] constexpr uint32_t getDynamicCount() const { return this->g_dynamicCount; }

    [[nodiscard]] constexpr uint32_t getVertexCount() const { return this->g_vertexCount; }
    [[nodiscard]] constexpr uint32_t getVertexOffset() const { return this->g_vertexOffset; }

    [[nodiscard]] constexpr uint32_t getFirstInstance() const { return this->g_firstInstance; }

private:
    uint32_t g_count{1};
    uint32_t const* g_textureIndices{nullptr};

    fge::vulkan::DescriptorSet const* g_dynamicDescriptors{nullptr};
    uint32_t g_dynamicCount{0};
    uint32_t const* g_dynamicBufferSizes{nullptr};
    uint32_t const* g_dynamicBufferOffsets{nullptr};
    uint32_t const* g_dynamicSets{nullptr};

    uint32_t g_vertexCount{0};
    uint32_t g_vertexOffset{0};
    uint32_t g_firstInstance{0};
    bool g_uniqueDrawCall{false};
    VkBuffer g_indirectBuffer{VK_NULL_HANDLE};
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
            return static_cast<fge::Texture const*>(this->g_textures)[index].getSharedData().get();
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
    using ArrayOfDescriptorSet = fge::vulkan::DescriptorSet const*;
    using ArrayOfDescriptorSetPointer = fge::vulkan::DescriptorSet const**;
    using ArrayVariant = std::variant<ArrayOfDescriptorSet, ArrayOfDescriptorSetPointer>;

    constexpr RenderResourceDescriptors() = default;

    constexpr RenderResourceDescriptors(ArrayVariant const& descriptors, uint32_t const* sets, uint32_t count) :
            g_descriptors(descriptors),
            g_sets(sets),
            g_count(count)
    {
        assert(count == 0 || sets != nullptr);
    }

    constexpr void set(ArrayVariant const& descriptors, uint32_t const* sets, uint32_t count)
    {
        this->g_descriptors = descriptors;
        this->g_sets = sets;
        this->g_count = count;

        assert(count == 0 || sets != nullptr);
    }

    [[nodiscard]] constexpr uint32_t getCount() const { return this->g_count; }

    [[nodiscard]] constexpr fge::vulkan::DescriptorSet const* getDescriptorSet(uint32_t index) const
    {
        if (std::holds_alternative<ArrayOfDescriptorSet>(this->g_descriptors))
        {
            return &std::get<ArrayOfDescriptorSet>(this->g_descriptors)[index];
        }
        return std::get<ArrayOfDescriptorSetPointer>(this->g_descriptors)[index];
    }
    [[nodiscard]] constexpr uint32_t getSet(uint32_t index) const { return this->g_sets[index]; }

private:
    ArrayVariant g_descriptors{ArrayOfDescriptorSet{nullptr}};
    uint32_t const* g_sets{nullptr};
    uint32_t g_count{0};
};

/**
 * \class RenderResourcePushConstants
 * \ingroup graphics
 * \brief Resource containing push constants information for rendering
 *
 * Push constants are used to send small amounts of data to the shader with a fast path.
 */
class RenderResourcePushConstants
{
public:
    struct PushConstantData
    {
        VkShaderStageFlags g_stages{VK_SHADER_STAGE_ALL};
        uint32_t g_offset{0};
        uint32_t g_size{0};
        void const* g_data{nullptr};
    };

    constexpr RenderResourcePushConstants() = default;

    constexpr void set(PushConstantData const* data, uint32_t count)
    {
        assert(count == 0 || data != nullptr);

        this->g_pushConstants = data;
        this->count = count;
    }

    [[nodiscard]] constexpr PushConstantData const* getPushConstants(uint32_t index) const
    {
        return this->g_pushConstants + index;
    }
    [[nodiscard]] constexpr uint32_t getCount() const { return this->count; }

private:
    PushConstantData const* g_pushConstants{nullptr};
    uint32_t count{0};
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
    RenderStates(RenderStates const& r) = delete;
    RenderStates(RenderStates&& r) noexcept = default;
    explicit RenderStates(fge::TextureType const* textureImage = nullptr) :
            _resTextures{textureImage, 1}
    {}
    explicit RenderStates(fge::vulkan::VertexBuffer const* vertexBuffer,
                          fge::TextureType const* textureImage = nullptr,
                          fge::vulkan::BlendMode const& blendMode = {}) :
            _resTextures{textureImage, 1},
            _vertexBuffer(vertexBuffer),
            _blendMode(blendMode)
    {}

    [[nodiscard]] RenderStates copy(fge::TextureType const* textureImage = nullptr) const
    {
        return RenderStates{nullptr, textureImage, this->_blendMode};
    }

    RenderStates& operator=(RenderStates const& r) = delete;
    RenderStates& operator=(RenderStates&& r) noexcept = default;

    RenderResourceTransform _resTransform{};
    RenderResourceTextures _resTextures{};
    RenderResourceInstances _resInstances{};
    RenderResourceDescriptors _resDescriptors{};
    RenderResourcePushConstants _resPushConstants{};

    fge::vulkan::VertexBuffer const* _vertexBuffer{nullptr};
    fge::vulkan::IndexBuffer const* _indexBuffer{nullptr};
    fge::vulkan::BlendMode _blendMode{};
    VkPrimitiveTopology _topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST}; //!< Only used if no vertex buffer is set
    fge::vulkan::Shader const* _shaderVertex{nullptr};
    fge::vulkan::Shader const* _shaderGeometry{nullptr};
    fge::vulkan::Shader const* _shaderFragment{nullptr};
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED
