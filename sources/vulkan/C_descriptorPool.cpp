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

#include "FastEngine/vulkan/C_descriptorPool.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include <stdexcept>
#include <vector>

namespace fge::vulkan
{

DescriptorPool::DescriptorPool() :
        g_descriptorPoolSizes(),

        g_maxSetsPerPool(0),
        g_descriptorPools(),
        g_isUnique(false),
        g_isCreated(false),

        g_context(nullptr)
{}
DescriptorPool::DescriptorPool(DescriptorPool&& r) noexcept :
        g_descriptorPoolSizes(std::move(r.g_descriptorPoolSizes)),

        g_maxSetsPerPool(r.g_maxSetsPerPool),
        g_descriptorPools(std::move(r.g_descriptorPools)),
        g_isUnique(r.g_isUnique),
        g_isCreated(r.g_isCreated),

        g_context(r.g_context)
{
    r.g_maxSetsPerPool = 0;

    r.g_isUnique = false;
    r.g_isCreated = false;

    r.g_context = nullptr;
}
DescriptorPool::~DescriptorPool()
{
    this->destroy();
}

void DescriptorPool::create(const Context& context,
                            std::vector<VkDescriptorPoolSize>&& descriptorPoolSizes,
                            uint32_t maxSetsPerPool, bool isUnique)
{
    this->destroy();

    this->g_context = &context;
    this->g_isUnique = isUnique;
    this->g_maxSetsPerPool = maxSetsPerPool;
    this->g_descriptorPoolSizes = std::move(descriptorPoolSizes);
    this->g_isCreated = true;

    this->g_descriptorPools.push_back(this->createPool());
}
void DescriptorPool::destroy()
{
    if (this->g_isCreated)
    {
        for (auto& pool : this->g_descriptorPools)
        {
            vkDestroyDescriptorPool(this->g_context->getLogicalDevice().getDevice(), pool._pool, nullptr);
        }
        this->g_descriptorPools.clear();

        this->g_context = nullptr;
        this->g_isCreated = false;
        this->g_isUnique = false;
        this->g_maxSetsPerPool = 0;
        this->g_descriptorPoolSizes.clear();
    }
}

[[nodiscard]] std::pair<VkDescriptorSet, VkDescriptorPool>  DescriptorPool::allocateDescriptorSets(const VkDescriptorSetLayout* setLayouts, uint32_t descriptorSetCount) const
{
    if (setLayouts == nullptr || descriptorSetCount == 0 || !this->g_isCreated)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = descriptorSetCount;
    allocInfo.pSetLayouts = setLayouts;

    for (auto& pool : this->g_descriptorPools)
    {
        allocInfo.descriptorPool = pool._pool;

        auto result = vkAllocateDescriptorSets(this->g_context->getLogicalDevice().getDevice(), &allocInfo, &descriptorSet);
        if (result != VK_SUCCESS)
        {
            if (result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY)
            {
                continue;
            }
            else
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }
        }
        else
        {
            ++pool._count;
            descriptorPool = pool._pool;
            break;
        }
    }

    //Try creating a new pool
    if (descriptorSet == VK_NULL_HANDLE && !this->g_isUnique)
    {
        this->g_descriptorPools.push_back(this->createPool());
        allocInfo.descriptorPool = this->g_descriptorPools.back()._pool;
        descriptorPool = this->g_descriptorPools.back()._pool;
        vkAllocateDescriptorSets(this->g_context->getLogicalDevice().getDevice(), &allocInfo, &descriptorSet);
    }

    //Last check for a valid descriptor set
    if (descriptorSet == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    return {descriptorSet, descriptorPool};
}
void DescriptorPool::freeDescriptorSets(VkDescriptorSet descriptorSet, VkDescriptorPool descriptorPool, bool dontFreeButDecrementCount) const
{
    for (auto& pool : this->g_descriptorPools)
    {
        if (pool._pool == descriptorPool)
        {
            if (dontFreeButDecrementCount)
            {
                --pool._count;
                return;
            }

            this->g_context->_garbageCollector.push(GarbageCollector::Garbage(descriptorSet, descriptorPool, this->g_context->getLogicalDevice().getDevice()));
            --pool._count;
            return;
        }
    }

    throw std::runtime_error("failed to free descriptor sets!");
}
void DescriptorPool::resetDescriptorPool(VkDescriptorPool descriptorPool) const
{
    for (auto& pool : this->g_descriptorPools)
    {
        if (pool._pool == descriptorPool)
        {
            if (vkResetDescriptorPool(this->g_context->getLogicalDevice().getDevice(), pool._pool, 0) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to reset descriptor pool!");
            }
            pool._count = 0;
            return;
        }
    }

    throw std::runtime_error("failed to reset descriptor pool!");
}
void DescriptorPool::resetDescriptorPool() const
{
    for (auto& pool : this->g_descriptorPools)
    {
        if (vkResetDescriptorPool(this->g_context->getLogicalDevice().getDevice(), pool._pool, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to reset descriptor pool!");
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
const Context* DescriptorPool::getContext() const
{
    return this->g_context;
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
    if (vkCreateDescriptorPool(this->g_context->getLogicalDevice().getDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    return {pool, 0};
}

}//end fge::vulkan