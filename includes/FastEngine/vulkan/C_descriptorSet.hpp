/*
 * Copyright 2026 Guillaume Guillet
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

#ifndef _FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include <variant>

namespace fge::vulkan
{

class Context;
class DescriptorSetLayout;
class DescriptorPool;
class UniformBuffer;
class TextureImage;

/**
 * \class DescriptorSet
 * \ingroup vulkan
 * \brief This class abstract the vulkan descriptor set for easier use
 *
 * In order to instanciate a descriptor set, you need to allocate it from a DescriptorPool.
 */
class FGE_API DescriptorSet
{
public:
    /**
     * \struct Descriptor
     * \ingroup vulkan
     * \brief This struct is used to describe a descriptor
     *
     * Every descriptor must have a binding. The binding is used to link the descriptor
     * to the descriptor set layout.
     */
    struct FGE_API Descriptor
    {
        enum class BufferTypes
        {
            STATIC,
            DYNAMIC,
            STORAGE
        };

        Descriptor() = default;
        Descriptor(UniformBuffer const& uniformBuffer,
                   uint32_t binding,
                   BufferTypes type = BufferTypes::STATIC,
                   VkDeviceSize range = 0);
        Descriptor(TextureImage const& textureImage, uint32_t binding);

        std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo> _data;
        uint32_t _binding{0};
        BufferTypes _bufferType{};
        uint32_t _dstArrayElement{0};
    };

    DescriptorSet();
    DescriptorSet(VkDescriptorSet descriptorSet, DescriptorPool const* pool, VkDescriptorPool descriptorPool);
    DescriptorSet(DescriptorSet const& r);
    DescriptorSet(DescriptorSet&& r) noexcept;
    ~DescriptorSet();

    DescriptorSet& operator=(DescriptorSet const& r);
    DescriptorSet& operator=(DescriptorSet&& r) noexcept;

    void destroy();

    [[nodiscard]] VkDescriptorSet get() const;
    [[nodiscard]] DescriptorPool const* getPool() const;
    [[nodiscard]] Context const* getContext() const;

    /**
     * \brief Update the descriptor set
     *
     * This function update the descriptor set with the given descriptors.
     *
     * \param descriptors An array of descriptors
     * \param descriptorSize The size of the array
     */
    void updateDescriptorSet(Descriptor const* descriptors, std::size_t descriptorSize);

private:
    VkDescriptorSet g_descriptorSet;
    DescriptorPool const* g_pool;
    VkDescriptorPool g_poolKey;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED
