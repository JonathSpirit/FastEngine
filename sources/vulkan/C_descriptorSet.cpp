#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_descriptorSetLayout.hpp"
#include "FastEngine/vulkan/C_descriptorPool.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_uniformBuffer.hpp"
#include "FastEngine/vulkan/C_textureImage.hpp"
#include <vector>

namespace fge::vulkan
{

//Descriptor

DescriptorSet::Descriptor::Descriptor(const UniformBuffer& uniformBuffer, uint32_t binding) :
        _data(VkDescriptorBufferInfo{.buffer=uniformBuffer.getBuffer(),
                                     .offset=0,
                                     .range=uniformBuffer.getBufferSize()}),
        _binding(binding)
{}
DescriptorSet::Descriptor::Descriptor(const TextureImage& textureImage, uint32_t binding) :
        _data(VkDescriptorImageInfo{.sampler=textureImage.getTextureSampler(),
                                    .imageView=textureImage.getTextureImageView(),
                                    .imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}),
        _binding(binding)
{}

//DescriptorSet

DescriptorSet::DescriptorSet() :
        g_descriptorSet(VK_NULL_HANDLE, VK_NULL_HANDLE),

        g_descriptorPool(VK_NULL_HANDLE),
        g_freeFromPool(true),

        g_logicalDevice(nullptr)
{}
DescriptorSet::DescriptorSet([[maybe_unused]] const DescriptorSet& r) :
        DescriptorSet() ///TODO: better copy
{}
DescriptorSet::DescriptorSet(DescriptorSet&& r) noexcept :
        g_descriptorSet(std::move(r.g_descriptorSet)),

        g_descriptorPool(r.g_descriptorPool),
        g_freeFromPool(r.g_freeFromPool),

        g_logicalDevice(r.g_logicalDevice)
{
    r.g_descriptorSet = {VK_NULL_HANDLE, VK_NULL_HANDLE};

    r.g_descriptorPool = nullptr;
    r.g_freeFromPool = true;

    r.g_logicalDevice = nullptr;
}
DescriptorSet::~DescriptorSet()
{
    this->destroy();
}

DescriptorSet& DescriptorSet::operator=([[maybe_unused]] const DescriptorSet& r)
{///TODO: better copy
    this->g_descriptorSet = {VK_NULL_HANDLE, VK_NULL_HANDLE};

    this->g_descriptorPool = nullptr;
    this->g_freeFromPool = true;

    this->g_logicalDevice = nullptr;
    return *this;
}

void DescriptorSet::create(const LogicalDevice& logicalDevice,
                           const DescriptorSetLayout* layouts,
                           std::size_t layoutSize,
                           const DescriptorPool& pool,
                           bool freeFromPool)
{
    if (layoutSize == 0)
    {
        return;
    }

    this->g_descriptorPool = &pool;
    this->g_freeFromPool = freeFromPool;
    this->g_logicalDevice = &logicalDevice;

    std::vector<VkDescriptorSetLayout> vulkanLayouts(layoutSize);
    for (std::size_t i=0; i<layoutSize; ++i)
    {
        vulkanLayouts[i] = layouts[i].getLayout();
    }

    this->g_descriptorSet = pool.allocateDescriptorSets(vulkanLayouts.data(), layoutSize);
}
void DescriptorSet::destroy()
{
    if (this->g_descriptorSet.first != VK_NULL_HANDLE)
    {
        this->g_descriptorPool->freeDescriptorSets(this->g_descriptorSet.first,
                                                   this->g_descriptorSet.second,
                                                   !this->g_freeFromPool);

        this->g_descriptorSet = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        this->g_descriptorPool = nullptr;
        this->g_freeFromPool = true;
        this->g_logicalDevice = nullptr;
    }
}

VkDescriptorSet DescriptorSet::getDescriptorSet() const
{
    return this->g_descriptorSet.first;
}
const DescriptorPool* DescriptorSet::getDescriptorPool() const
{
    return this->g_descriptorPool;
}
bool DescriptorSet::isFreeFromPool() const
{
    return this->g_freeFromPool;
}
const LogicalDevice* DescriptorSet::getLogicalDevice() const
{
    return this->g_logicalDevice;
}

void DescriptorSet::updateDescriptorSet(const Descriptor* descriptors, std::size_t descriptorSize)
{
    if (descriptors == nullptr || descriptorSize == 0)
    {
        return;
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites(descriptorSize);

    for (std::size_t i=0; i<descriptorSize; ++i)
    {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = this->g_descriptorSet.first;
        descriptorWrites[i].dstBinding = descriptors[i]._binding;
        descriptorWrites[i].dstArrayElement = 0;

        descriptorWrites[i].descriptorCount = 1;

        descriptorWrites[i].pTexelBufferView = nullptr;

        if (std::holds_alternative<VkDescriptorBufferInfo>(descriptors[i]._data))
        {
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            descriptorWrites[i].pBufferInfo = &std::get<VkDescriptorBufferInfo>(descriptors[i]._data);
            descriptorWrites[i].pImageInfo = nullptr;
        }
        else
        {//TextureImage
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            descriptorWrites[i].pBufferInfo = nullptr;
            descriptorWrites[i].pImageInfo = &std::get<VkDescriptorImageInfo>(descriptors[i]._data);
        }
    }

    vkUpdateDescriptorSets(this->g_logicalDevice->getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

}//end fge::vulkan