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

#ifndef _FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "SDL_vulkan.h"
#include "volk.h"
#include <variant>
#include <utility>

namespace fge::vulkan
{

class LogicalDevice;
class DescriptorSetLayout;
class DescriptorPool;
class UniformBuffer;
class TextureImage;

class FGE_API DescriptorSet
{
public:
    struct Descriptor
    {
        enum class BufferTypes
        {
            STATIC,
            DYNAMIC
        };

        Descriptor() = default;
        Descriptor(const UniformBuffer& uniformBuffer, uint32_t binding, BufferTypes type=BufferTypes::STATIC, VkDeviceSize range=0);
        Descriptor(const TextureImage& textureImage, uint32_t binding);

        std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo> _data;
        uint32_t _binding{0};
        BufferTypes _bufferType{};
    };

    DescriptorSet();
    DescriptorSet(const DescriptorSet& r);
    DescriptorSet(DescriptorSet&& r) noexcept;
    ~DescriptorSet();

    DescriptorSet& operator=(const DescriptorSet& r);
    DescriptorSet& operator=(DescriptorSet&& r) noexcept;

    void create(const LogicalDevice& logicalDevice,
                const DescriptorSetLayout* layouts,
                std::size_t layoutSize,
                const DescriptorPool& pool,
                bool freeFromPool=true);
    void destroy();

    [[nodiscard]] VkDescriptorSet getDescriptorSet() const;
    [[nodiscard]] const DescriptorPool* getDescriptorPool() const;
    [[nodiscard]] bool isFreeFromPool() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

    void updateDescriptorSet(const Descriptor* descriptors, std::size_t descriptorSize);

private:
    std::pair<VkDescriptorSet, VkDescriptorPool> g_descriptorSet;

    const DescriptorPool* g_descriptorPool;
    bool g_freeFromPool;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED
