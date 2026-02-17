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

#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_descriptorPool.hpp"

namespace fge::vulkan
{

//Descriptor

DescriptorSet::Descriptor::Descriptor(UniformBuffer const& uniformBuffer,
                                      uint32_t binding,
                                      BufferTypes type,
                                      VkDeviceSize range) :
        _data(VkDescriptorBufferInfo{.buffer = uniformBuffer.getBuffer(),
                                     .offset = 0,
                                     .range = type == BufferTypes::STATIC ? uniformBuffer.getBufferSize() : range}),
        _binding(binding),
        _bufferType(type)
{}
DescriptorSet::Descriptor::Descriptor(TextureImage const& textureImage, uint32_t binding) :
        _data(VkDescriptorImageInfo{.sampler = textureImage.getTextureSampler(),
                                    .imageView = textureImage.getTextureImageView(),
                                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}),
        _binding(binding),
        _bufferType()
{}

//DescriptorSet

DescriptorSet::DescriptorSet() :
        g_descriptorSet(VK_NULL_HANDLE),
        g_pool(nullptr),
        g_poolKey(VK_NULL_HANDLE)
{}
DescriptorSet::DescriptorSet(VkDescriptorSet descriptorSet,
                             DescriptorPool const* pool,
                             VkDescriptorPool descriptorPool) :
        g_descriptorSet(descriptorSet),
        g_pool(pool),
        g_poolKey(descriptorPool)
{}
DescriptorSet::DescriptorSet([[maybe_unused]] DescriptorSet const& r) :
        DescriptorSet() ///TODO: better copy
{}
DescriptorSet::DescriptorSet(DescriptorSet&& r) noexcept :
        g_descriptorSet(r.g_descriptorSet),
        g_pool(r.g_pool),
        g_poolKey(r.g_poolKey)
{
    r.g_descriptorSet = VK_NULL_HANDLE;
    r.g_pool = nullptr;
    r.g_poolKey = VK_NULL_HANDLE;
}
DescriptorSet::~DescriptorSet()
{
    this->destroy();
}

void DescriptorSet::destroy()
{
    if (this->g_descriptorSet != VK_NULL_HANDLE)
    {
        this->g_pool->freeDescriptorSet(this->g_descriptorSet, this->g_poolKey);

        this->g_descriptorSet = VK_NULL_HANDLE;
        this->g_pool = nullptr;
        this->g_poolKey = VK_NULL_HANDLE;
    }
}

DescriptorSet& DescriptorSet::operator=([[maybe_unused]] DescriptorSet const& r)
{ ///TODO: better copy
    this->g_descriptorSet = VK_NULL_HANDLE;
    this->g_pool = nullptr;
    this->g_poolKey = VK_NULL_HANDLE;

    return *this;
}
DescriptorSet& DescriptorSet::operator=(DescriptorSet&& r) noexcept
{
    if (this != &r)
    {
        this->destroy();

        this->g_descriptorSet = r.g_descriptorSet;
        this->g_pool = r.g_pool;
        this->g_poolKey = r.g_poolKey;

        r.g_descriptorSet = VK_NULL_HANDLE;
        r.g_pool = nullptr;
        r.g_poolKey = VK_NULL_HANDLE;
    }

    return *this;
}

[[nodiscard]] VkDescriptorSet DescriptorSet::get() const
{
    return this->g_descriptorSet;
}
[[nodiscard]] DescriptorPool const* DescriptorSet::getPool() const
{
    return this->g_pool;
}
[[nodiscard]] Context const* DescriptorSet::getContext() const
{
    if (this->g_pool != nullptr)
    {
        return &this->g_pool->getContext();
    }
    return nullptr;
}

void DescriptorSet::updateDescriptorSet(Descriptor const* descriptors, std::size_t descriptorSize)
{
    if (descriptors == nullptr || descriptorSize == 0 || this->g_descriptorSet == VK_NULL_HANDLE)
    {
        return;
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites(descriptorSize);

    for (std::size_t i = 0; i < descriptorSize; ++i)
    {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = this->g_descriptorSet;
        descriptorWrites[i].dstBinding = descriptors[i]._binding;
        descriptorWrites[i].dstArrayElement = descriptors[i]._dstArrayElement;

        descriptorWrites[i].descriptorCount = 1; ///TODO: multiple descriptors in one write

        descriptorWrites[i].pTexelBufferView = nullptr;

        if (std::holds_alternative<VkDescriptorBufferInfo>(descriptors[i]._data))
        {
            switch (descriptors[i]._bufferType)
            {
            case Descriptor::BufferTypes::STATIC:
                descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case Descriptor::BufferTypes::DYNAMIC:
                descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                break;
            case Descriptor::BufferTypes::STORAGE:
                descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            }

            descriptorWrites[i].pBufferInfo = &std::get<VkDescriptorBufferInfo>(descriptors[i]._data);
            descriptorWrites[i].pImageInfo = nullptr;
        }
        else
        { //TextureImage
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            descriptorWrites[i].pBufferInfo = nullptr;
            descriptorWrites[i].pImageInfo = &std::get<VkDescriptorImageInfo>(descriptors[i]._data);
        }
    }

    vkUpdateDescriptorSets(this->g_pool->getContext().getLogicalDevice().getDevice(), descriptorWrites.size(),
                           descriptorWrites.data(), 0, nullptr);
}

} // namespace fge::vulkan