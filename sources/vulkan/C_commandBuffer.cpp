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

#include "FastEngine/vulkan/C_commandBuffer.hpp"
#include "FastEngine/vulkan/C_context.hpp"

namespace fge::vulkan
{

CommandBuffer::CommandBuffer(Context const& context,
                             VkCommandBufferLevel level,
                             SubmitTypes type,
                             VkCommandPool commandPool) :
        ContextAware(context),
        g_type(type),
        g_commandBuffer(VK_NULL_HANDLE),
        g_commandPool(VK_NULL_HANDLE),
        g_level(level),
        g_renderPassScope(RenderPassScopes::BOTH),
        g_queueType(SUPPORTED_QUEUE_ALL),
        g_isEnded(false)
{
    this->create(level, type, commandPool);
}
CommandBuffer::~CommandBuffer()
{
    this->destroy();
}

void CommandBuffer::create(VkCommandBufferLevel level, SubmitTypes type, VkCommandPool commandPool)
{
    this->destroy();

    this->g_level = level;
    this->g_type = type;
    this->g_commandPool = commandPool;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = level;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(this->getContext().getLogicalDevice().getDevice(), &allocInfo,
                                 &this->g_commandBuffer) != VK_SUCCESS)
    {
        throw fge::Exception("failed to allocate command buffer!");
    }
}
void CommandBuffer::destroy()
{
    if (this->g_commandBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(this->getContext().getLogicalDevice().getDevice(), this->g_commandPool, 1,
                             &this->g_commandBuffer);
        this->g_commandBuffer = VK_NULL_HANDLE;
        this->g_commandPool = VK_NULL_HANDLE, this->g_queueType = SUPPORTED_QUEUE_ALL,
        this->g_renderPassScope = RenderPassScopes::BOTH, this->g_isEnded = false;
    }
}
std::pair<VkCommandBuffer, VkCommandPool> CommandBuffer::release()
{
    if (this->g_commandBuffer != VK_NULL_HANDLE)
    {
        auto* buffer = this->g_commandBuffer;
        auto* pool = this->g_commandPool;

        this->g_commandBuffer = VK_NULL_HANDLE;
        this->g_commandPool = VK_NULL_HANDLE, this->g_queueType = SUPPORTED_QUEUE_ALL,
        this->g_renderPassScope = RenderPassScopes::BOTH, this->g_isEnded = false;

        return {buffer, pool};
    }
    return {VK_NULL_HANDLE, VK_NULL_HANDLE};
}

void CommandBuffer::reset()
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }

    vkResetCommandBuffer(this->g_commandBuffer, 0);
    this->g_queueType = SUPPORTED_QUEUE_ALL, this->g_renderPassScope = RenderPassScopes::BOTH, this->g_isEnded = false;
}
void CommandBuffer::begin(VkCommandBufferUsageFlags flags)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags;

    vkBeginCommandBuffer(this->g_commandBuffer, &beginInfo);
}
void CommandBuffer::end()
{
    if (this->g_isEnded)
    {
        return;
    }

    vkEndCommandBuffer(this->g_commandBuffer);
    this->g_isEnded = true;
}

CommandBuffer::SubmitTypes CommandBuffer::getSubmitType() const
{
    return this->g_type;
}
VkCommandBuffer CommandBuffer::get() const
{
    return this->g_commandBuffer;
}
VkCommandPool CommandBuffer::getPool() const
{
    return this->g_commandPool;
}
VkCommandBufferLevel CommandBuffer::getLevel() const
{
    return this->g_level;
}
CommandBuffer::RenderPassScopes CommandBuffer::getRenderPassScope() const
{
    return this->g_renderPassScope;
}
CommandBuffer::SupportedQueueTypes_t CommandBuffer::getSupportedQueues() const
{
    return this->g_queueType;
}
bool CommandBuffer::isEnded() const
{
    return this->g_isEnded;
}

void CommandBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if (this->g_renderPassScope == RenderPassScopes::INSIDE)
    {
        throw fge::Exception("Command executed inside a render pass !");
    }

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(this->g_commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    this->g_renderPassScope = RenderPassScopes::OUTSIDE;
}

void CommandBuffer::transitionImageLayout(VkImage image,
                                          [[maybe_unused]] VkFormat format,
                                          VkImageLayout oldLayout,
                                          VkImageLayout newLayout,
                                          uint32_t mipLevels)
{ ///TODO: format
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
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

    vkCmdPipelineBarrier(this->g_commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void CommandBuffer::copyBufferToImage(VkBuffer buffer,
                                      VkImage image,
                                      uint32_t width,
                                      uint32_t height,
                                      int32_t offsetX,
                                      int32_t offsetY)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if (this->g_renderPassScope == RenderPassScopes::INSIDE)
    {
        throw fge::Exception("Command executed inside a render pass !");
    }

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

    vkCmdCopyBufferToImage(this->g_commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    this->g_renderPassScope = RenderPassScopes::OUTSIDE;
}
void CommandBuffer::copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if (this->g_renderPassScope == RenderPassScopes::INSIDE)
    {
        throw fge::Exception("Command executed inside a render pass !");
    }

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

    vkCmdCopyImageToBuffer(this->g_commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);

    this->g_renderPassScope = RenderPassScopes::OUTSIDE;
}
void CommandBuffer::copyImageToImage(VkImage srcImage,
                                     VkImage dstImage,
                                     uint32_t width,
                                     uint32_t height,
                                     int32_t offsetX,
                                     int32_t offsetY)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if (this->g_renderPassScope == RenderPassScopes::INSIDE)
    {
        throw fge::Exception("Command executed inside a render pass !");
    }

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

    vkCmdCopyImage(this->g_commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    this->g_renderPassScope = RenderPassScopes::OUTSIDE;
}

} // namespace fge::vulkan