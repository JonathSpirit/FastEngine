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

#include "FastEngine/vulkan/C_context.hpp"
#include <iostream>
#include <optional>
#include <vector>

//https://docs.tizen.org/application/native/guides/graphics/vulkan/

//https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules

namespace fge::vulkan
{

Context::Context() :
        g_multiUseDescriptorPool(*this),
        g_textureLayout(*this),
        g_transformLayout(*this),
        g_textureDescriptorPool(*this),
        g_transformDescriptorPool(*this),
        g_commandPool(VK_NULL_HANDLE),
        g_isCreated(false)
{}
Context::~Context()
{
    this->destroy();
}

void Context::destroy()
{
    if (this->g_isCreated)
    {
        this->g_cacheLayouts.clear();
        this->g_textureLayout.destroy();
        this->g_transformLayout.destroy();

        this->g_multiUseDescriptorPool.destroy();
        this->g_textureDescriptorPool.destroy();
        this->g_transformDescriptorPool.destroy();

        vkDestroyCommandPool(this->g_logicalDevice.getDevice(), this->g_commandPool, nullptr);

        vmaDestroyAllocator(this->g_allocator);

        this->g_surface.destroy();
        this->g_logicalDevice.destroy();
        this->g_instance.destroy();

        this->g_commandPool = VK_NULL_HANDLE;
        this->g_isCreated = false;
    }
}

VkCommandBuffer Context::beginSingleTimeCommands() const
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = this->g_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    vkAllocateCommandBuffers(this->g_logicalDevice.getDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}
void Context::endSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(this->g_logicalDevice.getGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->g_logicalDevice.getGraphicQueue()); ///TODO stop doing that

    vkFreeCommandBuffers(this->g_logicalDevice.getDevice(), this->g_commandPool, 1, &commandBuffer);
}

void Context::initVolk()
{
    auto result = volkInitialize();
    if (result != VK_SUCCESS)
    {
        throw fge::Exception{"Can't init volk!"};
    }
}

void Context::initVulkan(SDL_Window* window)
{
    this->g_isCreated = true;

    this->g_instance.create(window, "VulkanTest");

    this->g_surface.create(this->g_instance);
    this->g_physicalDevice = this->g_instance.pickPhysicalDevice(this->g_surface.getSurface());
    if (this->g_physicalDevice.getDevice() == VK_NULL_HANDLE)
    {
        throw fge::Exception("failed to find a suitable GPU!");
    }
    this->g_logicalDevice.create(this->g_physicalDevice, this->g_surface.getSurface());

    VmaVulkanFunctions vulkanFunctions{vkGetInstanceProcAddr,
                                       vkGetDeviceProcAddr,
                                       vkGetPhysicalDeviceProperties,
                                       vkGetPhysicalDeviceMemoryProperties,
                                       vkAllocateMemory,
                                       vkFreeMemory,
                                       vkMapMemory,
                                       vkUnmapMemory,
                                       vkFlushMappedMemoryRanges,
                                       vkInvalidateMappedMemoryRanges,
                                       vkBindBufferMemory,
                                       vkBindImageMemory,
                                       vkGetBufferMemoryRequirements,
                                       vkGetImageMemoryRequirements,
                                       vkCreateBuffer,
                                       vkDestroyBuffer,
                                       vkCreateImage,
                                       vkDestroyImage,
                                       vkCmdCopyBuffer,
                                       vkGetBufferMemoryRequirements2,
                                       vkGetImageMemoryRequirements2,
                                       vkBindBufferMemory2,
                                       vkBindImageMemory2,
                                       vkGetPhysicalDeviceMemoryProperties2};

    VmaAllocatorCreateInfo allocatorCreateInfo{0,
                                               this->g_physicalDevice.getDevice(),
                                               this->g_logicalDevice.getDevice(),
                                               0,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               &vulkanFunctions,
                                               this->g_instance.getInstance(),
                                               VK_API_VERSION_1_1,
                                               nullptr};

    if (vmaCreateAllocator(&allocatorCreateInfo, &this->g_allocator) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create allocator!");
    }

    this->createCommandPool();
    this->createMultiUseDescriptorPool();
    this->createTextureDescriptorPool();
    this->createTransformDescriptorPool();

    this->g_textureLayout.create({fge::vulkan::CreateSimpleLayoutBinding(
            FGE_VULKAN_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)});
    this->g_transformLayout.create({fge::vulkan::CreateSimpleLayoutBinding(
            FGE_VULKAN_TRANSFORM_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)});
}
void Context::enumerateExtensions()
{
#ifdef FGE_DEF_DEBUG
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "available extensions:\n";

    for (const auto& extension: extensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }
    std::cout << std::endl;
#endif
}

void Context::waitIdle()
{
    vkDeviceWaitIdle(this->g_logicalDevice.getDevice());
}

const Instance& Context::getInstance() const
{
    return this->g_instance;
}
const Surface& Context::getSurface() const
{
    return this->g_surface;
}
const LogicalDevice& Context::getLogicalDevice() const
{
    return this->g_logicalDevice;
}
const PhysicalDevice& Context::getPhysicalDevice() const
{
    return this->g_physicalDevice;
}

void Context::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    VkCommandBuffer commandBuffer = this->beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    this->endSingleTimeCommands(commandBuffer);
}

void Context::transitionImageLayout(VkImage image,
                                    [[maybe_unused]] VkFormat format,
                                    VkImageLayout oldLayout,
                                    VkImageLayout newLayout) const
{ ///TODO: format
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage{};
    VkPipelineStageFlags destinationStage{};

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw fge::Exception("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

void Context::copyBufferToImage(VkBuffer buffer,
                                VkImage image,
                                uint32_t width,
                                uint32_t height,
                                int32_t offsetX,
                                int32_t offsetY) const
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {offsetX, offsetY, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}
void Context::copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height) const
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);

    endSingleTimeCommands(commandBuffer);
}
void Context::copyImageToImage(VkImage srcImage,
                               VkImage dstImage,
                               uint32_t width,
                               uint32_t height,
                               int32_t offsetX,
                               int32_t offsetY) const
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageCopy region{};
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffset = {offsetX, offsetY, 0};

    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = 1;
    region.dstOffset = {0, 0, 0};

    region.extent.width = width;
    region.extent.height = height;
    region.extent.depth = 1;

    vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

fge::vulkan::DescriptorSetLayout& Context::getCacheLayout(std::string_view key) const
{
    auto it = this->g_cacheLayouts.find(key);
    if (it != this->g_cacheLayouts.end())
    {
        return it->second;
    }
    return this->g_cacheLayouts
            .emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(*this))
            .first->second;
}
const DescriptorPool& Context::getMultiUseDescriptorPool() const
{
    return this->g_multiUseDescriptorPool;
}

const fge::vulkan::DescriptorSetLayout& Context::getTextureLayout() const
{
    return this->g_textureLayout;
}
const fge::vulkan::DescriptorSetLayout& Context::getTransformLayout() const
{
    return this->g_transformLayout;
}
const DescriptorPool& Context::getTextureDescriptorPool() const
{
    return this->g_textureDescriptorPool;
}
const DescriptorPool& Context::getTransformDescriptorPool() const
{
    return this->g_transformDescriptorPool;
}

VmaAllocator Context::getAllocator() const
{
    return this->g_allocator;
}

void Context::createCommandPool()
{
    auto queueFamilyIndices = this->g_physicalDevice.findQueueFamilies(this->g_surface.getSurface());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices._graphicsFamily.value();

    if (vkCreateCommandPool(this->g_logicalDevice.getDevice(), &poolInfo, nullptr, &this->g_commandPool) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create command pool!");
    }
}
void Context::createMultiUseDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(3);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 1;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[2].descriptorCount = 1;

    this->g_multiUseDescriptorPool.create(std::move(poolSizes), 128, false, true);
}
void Context::createTextureDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(1);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 1;

    this->g_textureDescriptorPool.create(std::move(poolSizes), 128, false, true);
}
void Context::createTransformDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(1);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;

    this->g_transformDescriptorPool.create(std::move(poolSizes), 128, false, true);
}

} // namespace fge::vulkan