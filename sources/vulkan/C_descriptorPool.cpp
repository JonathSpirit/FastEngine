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

#include "FastEngine/vulkan/C_descriptorPool.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"

namespace fge::vulkan
{

DescriptorPool::DescriptorPool(Context const& context) :
        ContextAware(context),
        g_maxSetsPerPool(0),
        g_isUnique(false),
        g_isCreated(false),
        g_individuallyFree(true)
{}
DescriptorPool::DescriptorPool(DescriptorPool&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_descriptorPoolSizes(std::move(r.g_descriptorPoolSizes)),

        g_maxSetsPerPool(r.g_maxSetsPerPool),
        g_descriptorPools(std::move(r.g_descriptorPools)),
        g_isUnique(r.g_isUnique),
        g_isCreated(r.g_isCreated),
        g_individuallyFree(r.g_individuallyFree)
{
    r.g_maxSetsPerPool = 0;
    r.g_isUnique = false;
    r.g_isCreated = false;
    r.g_individuallyFree = true;
}
DescriptorPool::~DescriptorPool()
{
    this->destroy();
}

void DescriptorPool::create(std::vector<VkDescriptorPoolSize>&& descriptorPoolSizes,
                            uint32_t maxSetsPerPool,
                            bool isUnique,
                            bool individuallyFree)
{
    this->destroy();

    this->g_maxSetsPerPool = maxSetsPerPool;
    this->g_isUnique = isUnique;
    this->g_isCreated = true;
    this->g_individuallyFree = individuallyFree;
    ///TODO: if false, need to reset pools when they reach count=0

    this->g_descriptorPoolSizes = std::move(descriptorPoolSizes);
    this->g_descriptorPools.push_back(this->createPool());
}
void DescriptorPool::destroy()
{
    if (this->g_isCreated)
    {
        this->g_descriptorPoolSizes.clear();

        for (auto& pool: this->g_descriptorPools)
        {
            vkDestroyDescriptorPool(this->getContext().getLogicalDevice().getDevice(), pool._pool, nullptr);
        }
        this->g_descriptorPools.clear();

        this->g_maxSetsPerPool = 0;
        this->g_isUnique = false;
        this->g_isCreated = false;
        this->g_individuallyFree = true;
    }
}

[[nodiscard]] std::optional<DescriptorSet> DescriptorPool::allocateDescriptorSet(VkDescriptorSetLayout layout,
                                                                                 uint32_t variableElements) const
{
    if (layout == VK_NULL_HANDLE || !this->g_isCreated)
    {
        return std::nullopt;
    }

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    allocInfo.pNext = nullptr;

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variableDescriptorCountInfo = {};
    if (variableElements != 0)
    {
        variableDescriptorCountInfo.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        variableDescriptorCountInfo.descriptorSetCount = 1;
        variableDescriptorCountInfo.pDescriptorCounts = &variableElements;
        allocInfo.pNext = &variableDescriptorCountInfo;
    }

    for (auto& pool: this->g_descriptorPools)
    {
        allocInfo.descriptorPool = pool._pool;

        auto result =
                vkAllocateDescriptorSets(this->getContext().getLogicalDevice().getDevice(), &allocInfo, &descriptorSet);
        if (result != VK_SUCCESS)
        {
            if (result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY)
            {
                continue;
            }
            return std::nullopt;
        }

        ++pool._count;
        descriptorPool = pool._pool;
        break;
    }

    //Try creating a new pool
    if (descriptorSet == VK_NULL_HANDLE && !this->g_isUnique)
    {
        this->g_descriptorPools.push_back(this->createPool());
        allocInfo.descriptorPool = this->g_descriptorPools.back()._pool;
        descriptorPool = this->g_descriptorPools.back()._pool;
        vkAllocateDescriptorSets(this->getContext().getLogicalDevice().getDevice(), &allocInfo, &descriptorSet);
    }

    //Last check for a valid descriptor set
    if (descriptorSet == VK_NULL_HANDLE)
    {
        return std::nullopt;
    }

    return DescriptorSet{descriptorSet, this, descriptorPool};
}
void DescriptorPool::freeDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorPool descriptorPool) const
{
    if (descriptorSet == VK_NULL_HANDLE || descriptorPool == VK_NULL_HANDLE)
    {
        return;
    }

    for (auto& pool: this->g_descriptorPools)
    {
        if (pool._pool == descriptorPool)
        {
            if (!this->g_individuallyFree)
            {
                --pool._count;
                return;
            }

            this->getContext()._garbageCollector.push(GarbageDescriptorSet(
                    descriptorSet, descriptorPool, this->getContext().getLogicalDevice().getDevice()));
            --pool._count;
            return;
        }
    }

    //Should never happen, but in order to avoid memory leaks, we free the descriptor set anyway
    this->getContext()._garbageCollector.push(
            GarbageDescriptorSet(descriptorSet, descriptorPool, this->getContext().getLogicalDevice().getDevice()));
}
void DescriptorPool::resetPools() const
{
    for (auto& pool: this->g_descriptorPools)
    {
        if (vkResetDescriptorPool(this->getContext().getLogicalDevice().getDevice(), pool._pool, 0) != VK_SUCCESS)
        {
            throw fge::Exception("failed to reset descriptor pool!");
        }
        pool._count = 0;
    }
}

uint32_t DescriptorPool::getMaxSetsPerPool() const
{
    return this->g_maxSetsPerPool;
}
bool DescriptorPool::isUnique() const
{
    return this->g_isUnique;
}
bool DescriptorPool::isCreated() const
{
    return this->g_isCreated;
}

DescriptorPool::Pool DescriptorPool::createPool() const
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(this->g_descriptorPoolSizes.size());
    poolInfo.pPoolSizes = this->g_descriptorPoolSizes.data();
    poolInfo.maxSets = this->g_maxSetsPerPool;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkDescriptorPool pool;
    if (vkCreateDescriptorPool(this->getContext().getLogicalDevice().getDevice(), &poolInfo, nullptr, &pool) !=
        VK_SUCCESS)
    {
        throw fge::Exception("failed to create descriptor pool!");
    }

    return {pool, 0};
}

} // namespace fge::vulkan