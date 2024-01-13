/*
 * Copyright 2024 Guillaume Guillet
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
        g_currentFrame(0),
        g_graphicsCommandPool(VK_NULL_HANDLE),
        g_isCreated(false)
{
    this->g_outsideRenderScopeCommandBuffers.fill(VK_NULL_HANDLE);
    this->g_outsideRenderScopeCommandBuffersEmpty.fill(true);
    this->g_outsideRenderScopeFinishedSemaphores.fill(VK_NULL_HANDLE);
}
Context::~Context()
{
    this->destroy();
}

void Context::destroy()
{
    if (this->g_isCreated)
    {
        for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(this->g_logicalDevice.getDevice(), this->g_outsideRenderScopeFinishedSemaphores[i],
                               nullptr);
        }

        this->g_cacheLayouts.clear();
        this->g_textureLayout.destroy();
        this->g_transformLayout.destroy();

        this->g_multiUseDescriptorPool.destroy();
        this->g_textureDescriptorPool.destroy();
        this->g_transformDescriptorPool.destroy();

        vkDestroyCommandPool(this->g_logicalDevice.getDevice(), this->g_graphicsCommandPool, nullptr);

        vmaDestroyAllocator(this->g_allocator);

        this->g_surface.destroy();
        this->g_logicalDevice.destroy();
        this->g_instance.destroy();

        this->g_outsideRenderScopeCommandBuffers.fill(VK_NULL_HANDLE);
        this->g_outsideRenderScopeFinishedSemaphores.fill(VK_NULL_HANDLE);
        this->g_outsideRenderScopeCommandBuffersEmpty.fill(true);
        this->g_currentFrame = 0;

        this->g_graphicsCommandPool = VK_NULL_HANDLE;
        this->g_isCreated = false;
    }
}

Context::SingleTimeCommand Context::beginSingleTimeCommands(SingleTimeCommandTypes type) const
{
    switch (type)
    {
    case SingleTimeCommandTypes::DIRECT_EXECUTION:
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = this->g_graphicsCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        vkAllocateCommandBuffers(this->g_logicalDevice.getDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return {type, commandBuffer};
    }
    break;
    case SingleTimeCommandTypes::INDIRECT_OUTSIDE_RENDER_SCOPE_EXECUTION:
    {
        if (this->g_outsideRenderScopeCommandBuffersEmpty[this->g_currentFrame])
        {
            vkResetCommandBuffer(this->g_outsideRenderScopeCommandBuffers[this->g_currentFrame], 0);
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(this->g_outsideRenderScopeCommandBuffers[this->g_currentFrame], &beginInfo);

            this->g_outsideRenderScopeCommandBuffersEmpty[this->g_currentFrame] = false;
        }

        return {type, this->g_outsideRenderScopeCommandBuffers[this->g_currentFrame]};
    }
    break;
    }

    throw fge::Exception("Should be unreachable !");
}
void Context::endSingleTimeCommands(SingleTimeCommand command) const
{
    switch (command._type)
    {
    case SingleTimeCommandTypes::DIRECT_EXECUTION:
    {
        vkEndCommandBuffer(command._commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command._commandBuffer;

        vkQueueSubmit(this->g_logicalDevice.getGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(this->g_logicalDevice.getGraphicQueue());

        vkFreeCommandBuffers(this->g_logicalDevice.getDevice(), this->g_graphicsCommandPool, 1,
                             &command._commandBuffer);
    }
    break;
    case SingleTimeCommandTypes::INDIRECT_OUTSIDE_RENDER_SCOPE_EXECUTION:
        break;
    }
}

VkSemaphore Context::getOutsideRenderScopeSemaphore() const
{
    return this->g_outsideRenderScopeCommandBuffersEmpty[this->g_currentFrame]
                   ? VK_NULL_HANDLE
                   : this->g_outsideRenderScopeFinishedSemaphores[this->g_currentFrame];
}
void Context::submit() const
{
    if (this->g_outsideRenderScopeCommandBuffersEmpty[this->g_currentFrame])
    {
        return;
    }

    if (vkEndCommandBuffer(this->g_outsideRenderScopeCommandBuffers[this->g_currentFrame]) != VK_SUCCESS)
    {
        throw fge::Exception("failed to record command buffer!");
    }
    this->g_outsideRenderScopeCommandBuffersEmpty[this->g_currentFrame] = true;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &this->g_outsideRenderScopeCommandBuffers[this->g_currentFrame];

    VkSemaphore signalSemaphores[] = {this->g_outsideRenderScopeFinishedSemaphores[this->g_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(this->g_logicalDevice.getGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw fge::Exception("failed to submit outside render scope command buffer!");
    }

    this->g_currentFrame = (this->g_currentFrame + 1) % FGE_MAX_FRAMES_IN_FLIGHT;
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
    this->createSyncObjects();

    this->allocateGraphicsCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                         this->g_outsideRenderScopeCommandBuffers.data(), FGE_MAX_FRAMES_IN_FLIGHT);

    this->g_textureLayout.create({fge::vulkan::CreateSimpleLayoutBinding(
            FGE_VULKAN_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)});
    this->g_transformLayout.create({fge::vulkan::CreateSimpleLayoutBinding(
            FGE_VULKAN_TRANSFORM_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)});
}
void Context::enumerateExtensions()
{
    auto extensions = Context::retrieveExtensions();

    std::cout << "available extensions:\n";

    for (auto const& extension: extensions)
    {
        std::cout << '\t' << extension << '\n';
    }
    std::cout << std::flush;
}
std::vector<std::string> Context::retrieveExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::vector<std::string> result;
    result.reserve(extensionCount);

    for (auto const& extension: extensions)
    {
        result.emplace_back(extension.extensionName);
    }

    return result;
}

void Context::waitIdle()
{
    vkDeviceWaitIdle(this->g_logicalDevice.getDevice());
}

Instance const& Context::getInstance() const
{
    return this->g_instance;
}
Surface const& Context::getSurface() const
{
    return this->g_surface;
}
LogicalDevice const& Context::getLogicalDevice() const
{
    return this->g_logicalDevice;
}
PhysicalDevice const& Context::getPhysicalDevice() const
{
    return this->g_physicalDevice;
}

VkCommandPool Context::getGraphicsCommandPool() const
{
    return this->g_graphicsCommandPool;
}
void Context::allocateGraphicsCommandBuffers(VkCommandBufferLevel level,
                                             VkCommandBuffer commandBuffers[],
                                             uint32_t commandBufferCount) const
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = this->g_graphicsCommandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = commandBufferCount;

    if (vkAllocateCommandBuffers(this->g_logicalDevice.getDevice(), &allocInfo, commandBuffers) != VK_SUCCESS)
    {
        throw fge::Exception("failed to allocate command buffers!");
    }
}

void Context::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    auto command = this->beginSingleTimeCommands(SingleTimeCommandTypes::INDIRECT_OUTSIDE_RENDER_SCOPE_EXECUTION);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(command._commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    this->endSingleTimeCommands(command);
}

void Context::transitionImageLayout(VkImage image,
                                    [[maybe_unused]] VkFormat format,
                                    VkImageLayout oldLayout,
                                    VkImageLayout newLayout) const
{ ///TODO: format
    auto command = beginSingleTimeCommands(SingleTimeCommandTypes::DIRECT_EXECUTION);

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

    vkCmdPipelineBarrier(command._commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(command);
}

void Context::copyBufferToImage(VkBuffer buffer,
                                VkImage image,
                                uint32_t width,
                                uint32_t height,
                                int32_t offsetX,
                                int32_t offsetY) const
{
    auto command = beginSingleTimeCommands(SingleTimeCommandTypes::DIRECT_EXECUTION);

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

    vkCmdCopyBufferToImage(command._commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(command);
}
void Context::copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height) const
{
    auto command = beginSingleTimeCommands(SingleTimeCommandTypes::DIRECT_EXECUTION);

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

    vkCmdCopyImageToBuffer(command._commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);

    endSingleTimeCommands(command);
}
void Context::copyImageToImage(VkImage srcImage,
                               VkImage dstImage,
                               uint32_t width,
                               uint32_t height,
                               int32_t offsetX,
                               int32_t offsetY) const
{
    auto command = beginSingleTimeCommands(SingleTimeCommandTypes::DIRECT_EXECUTION);

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

    vkCmdCopyImage(command._commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(command);
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
DescriptorPool const& Context::getMultiUseDescriptorPool() const
{
    return this->g_multiUseDescriptorPool;
}

fge::vulkan::DescriptorSetLayout const& Context::getTextureLayout() const
{
    return this->g_textureLayout;
}
fge::vulkan::DescriptorSetLayout const& Context::getTransformLayout() const
{
    return this->g_transformLayout;
}
DescriptorPool const& Context::getTextureDescriptorPool() const
{
    return this->g_textureDescriptorPool;
}
DescriptorPool const& Context::getTransformDescriptorPool() const
{
    return this->g_transformDescriptorPool;
}

VmaAllocator Context::getAllocator() const
{
    return this->g_allocator;
}

void Context::pushGraphicsCommandBuffer(VkCommandBuffer commandBuffer) const
{
    this->g_executableGraphicsCommandBuffers.push_back(commandBuffer);
}
std::vector<VkCommandBuffer> const& Context::getGraphicsCommandBuffers() const
{
    return this->g_executableGraphicsCommandBuffers;
}
void Context::clearGraphicsCommandBuffers() const
{
    this->g_executableGraphicsCommandBuffers.clear();
}

void Context::createCommandPool()
{
    auto queueFamilyIndices = this->g_physicalDevice.findQueueFamilies(this->g_surface.getSurface());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices._graphicsFamily.value();

    if (vkCreateCommandPool(this->g_logicalDevice.getDevice(), &poolInfo, nullptr, &this->g_graphicsCommandPool) !=
        VK_SUCCESS)
    {
        throw fge::Exception("failed to create command pool!");
    }
}
void Context::createMultiUseDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(3);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = FGE_MULTIUSE_POOL_MAX_COMBINED_IMAGE_SAMPLER;

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
void Context::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(this->g_logicalDevice.getDevice(), &semaphoreInfo, nullptr,
                              &this->g_outsideRenderScopeFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw fge::Exception("failed to create semaphores!");
        }
    }
}

} // namespace fge::vulkan