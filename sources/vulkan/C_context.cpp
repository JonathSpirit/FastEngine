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

#include "FastEngine/vulkan/C_context.hpp"
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>
#include "SDL_events.h"

//https://docs.tizen.org/application/native/guides/graphics/vulkan/

//https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules

namespace fge::vulkan
{

Context::Context() :
        g_textureDescriptorPool(),
        g_transformDescriptorPool(),
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
        this->g_descriptorSetLayout.destroy();
        this->g_textureDescriptorPool.destroy();
        this->g_transformDescriptorPool.destroy();

        vkDestroyCommandPool(this->g_logicalDevice.getDevice(), this->g_commandPool, nullptr);

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
    vkQueueWaitIdle(this->g_logicalDevice.getGraphicQueue());

    vkFreeCommandBuffers(this->g_logicalDevice.getDevice(), this->g_commandPool, 1, &commandBuffer);
}

void Context::initVolk()
{
    auto result = volkInitialize();
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error{"Can't init volk!"};
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
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    this->g_logicalDevice.create(this->g_physicalDevice, this->g_surface.getSurface());

    this->createCommandPool();
    this->createTextureDescriptorPool();
    this->createTransformDescriptorPool();
    this->g_descriptorSetLayout.create(this->g_logicalDevice,{
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}
    });
}
void Context::enumerateExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "available extensions:\n";

    for (const auto& extension : extensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }
    std::cout << std::endl;
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

void Context::transitionImageLayout(VkImage image, [[maybe_unused]] VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const
{///TODO: format
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
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void Context::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, int32_t offsetX, int32_t offsetY) const
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
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

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
    region.imageExtent = {
            width,
            height,
            1
    };

    vkCmdCopyImageToBuffer(
            commandBuffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            buffer,
            1,
            &region
    );

    endSingleTimeCommands(commandBuffer);
}
void Context::copyImageToImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height, int32_t offsetX, int32_t offsetY) const
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

    vkCmdCopyImage(
            commandBuffer,
            srcImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
    );

    endSingleTimeCommands(commandBuffer);
}

const fge::vulkan::DescriptorSetLayout& Context::getDescriptorSetLayout() const
{
    return this->g_descriptorSetLayout;
}
const DescriptorPool& Context::getTextureDescriptorPool() const
{
    return this->g_textureDescriptorPool;
}
const DescriptorPool& Context::getTransformDescriptorPool() const
{
    return this->g_transformDescriptorPool;
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
        throw std::runtime_error("failed to create command pool!");
    }
}
void Context::createTextureDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(1);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 1;

    this->g_textureDescriptorPool.create(this->g_logicalDevice, std::move(poolSizes), 128, false);
}
void Context::createTransformDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(1);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;

    this->g_transformDescriptorPool.create(this->g_logicalDevice, std::move(poolSizes), 128, false);
}

}//end fge::vulkan