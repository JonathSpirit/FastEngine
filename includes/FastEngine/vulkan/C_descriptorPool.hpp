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

#ifndef _FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include <optional>
#include <vector>

namespace fge::vulkan
{

class DescriptorSet;

/**
 * \class DescriptorPool
 * \ingroup vulkan
 * \brief This class abstract the vulkan descriptor pool for easier use
 *
 * In vulkan, a descriptor pool must be created with a fixed size without
 * a way to resize it. This class help by allocating any number of descriptor sets
 * by creating multiple pools if needed.
 */
class FGE_API DescriptorPool : public ContextAware
{
public:
    explicit DescriptorPool(Context const& context);
    DescriptorPool(DescriptorPool const& r) = delete;
    DescriptorPool(DescriptorPool&& r) noexcept;
    ~DescriptorPool() override;

    DescriptorPool& operator=(DescriptorPool const& r) = delete;
    DescriptorPool& operator=(DescriptorPool&& r) noexcept = delete;

    /**
     * \brief Create the descriptor pool
     *
     * When the number of descriptor sets allocated reach the maxSetsPerPool,
     * a new pool is created.
     *
     * \param descriptorPoolSizes A vector of VkDescriptorPoolSize
     * \param maxSetsPerPool The max number of descriptor sets per pool
     * \param isUnique If \b true, only one pool is created and will fail if the maxSetsPerPool is reached
     * \param individuallyFree If \b true, the descriptor sets are individually freed
     */
    void create(std::vector<VkDescriptorPoolSize>&& descriptorPoolSizes,
                uint32_t maxSetsPerPool,
                bool isUnique,
                bool individuallyFree);
    void destroy() final;

    /**
     * \brief Allocate a descriptor set
     *
     * \warning If variableElements is not 0, the descriptor set layout must have been created
     * with the VK_DESCRIPTOR_SET_LAYOUT_CREATE_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT flag in this last binding.
     *
     * \param layout The descriptor set layout
     * \param variableElements The number of variable elements in the descriptor set
     * \return The descriptor set or std::nullopt if the allocation failed
     */
    [[nodiscard]] std::optional<DescriptorSet> allocateDescriptorSet(VkDescriptorSetLayout layout,
                                                                     uint32_t variableElements = 0) const;

    /**
     * \brief Free a descriptor set
     *
     * This function should not be directly called, DescriptorSet::destroy() should be used instead.
     *
     * \param descriptorSet The descriptor set
     * \param descriptorPool The descriptor pool that the descriptor set was allocated from
     */
    void freeDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorPool descriptorPool) const;
    /**
     * \brief Reset all the pools
     *
     * This function call vkResetDescriptorPool on all the pools.
     */
    void resetPools() const;

    [[nodiscard]] uint32_t getMaxSetsPerPool() const;
    [[nodiscard]] bool isUnique() const;
    [[nodiscard]] bool isCreated() const;

private:
    struct Pool
    {
        VkDescriptorPool _pool;
        uint32_t _count;
    };

    [[nodiscard]] Pool createPool() const;

    std::vector<VkDescriptorPoolSize> g_descriptorPoolSizes;

    uint32_t g_maxSetsPerPool;
    mutable std::vector<Pool> g_descriptorPools;
    bool g_isUnique;
    bool g_isCreated;
    bool g_individuallyFree;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED
