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

#ifndef _FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "SDL_vulkan.h"
#include "volk.h"
#include <vector>
#include <utility>

namespace fge::vulkan
{

class LogicalDevice;

class FGE_API DescriptorPool
{
public:
    struct Pool
    {
        VkDescriptorPool _pool;
        uint32_t _count;
    };

    DescriptorPool();
    DescriptorPool(const DescriptorPool& r) = delete;
    DescriptorPool(DescriptorPool&& r) noexcept;
    ~DescriptorPool();

    DescriptorPool& operator=(const DescriptorPool& r) = delete;
    DescriptorPool& operator=(DescriptorPool&& r) noexcept = delete;

    void create(const LogicalDevice& logicalDevice,
                std::vector<VkDescriptorPoolSize>&& descriptorPoolSizes,
                uint32_t maxSetsPerPool, bool isUnique);
    void destroy();

    [[nodiscard]] std::pair<VkDescriptorSet, VkDescriptorPool> allocateDescriptorSets(const VkDescriptorSetLayout* setLayouts, uint32_t descriptorSetCount) const;
    void freeDescriptorSets(VkDescriptorSet descriptorSet, VkDescriptorPool descriptorPool, bool dontFreeButDecrementCount) const;
    void resetDescriptorPool(VkDescriptorPool descriptorPool) const;
    void resetDescriptorPool() const;

    [[nodiscard]] uint32_t getMaxSetsPerPool() const;
    [[nodiscard]] bool isUnique() const;
    [[nodiscard]] bool isCreated() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

private:
    [[nodiscard]] Pool createPool() const;

    std::vector<VkDescriptorPoolSize> g_descriptorPoolSizes;

    uint32_t g_maxSetsPerPool;
    mutable std::vector<Pool> g_descriptorPools;
    bool g_isUnique;
    bool g_isCreated;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED
