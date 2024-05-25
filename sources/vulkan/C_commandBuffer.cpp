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

CommandBuffer::CommandBuffer(Context const& context) :
        ContextAware(context),
        g_commandBuffer(VK_NULL_HANDLE),
        g_commandPool(VK_NULL_HANDLE),
        g_level(VK_COMMAND_BUFFER_LEVEL_PRIMARY),
        g_renderPassScope(RenderPassScopes::BOTH),
        g_queueType(SUPPORTED_QUEUE_ALL),
        g_recordedCommands(0),
        g_isEnded(false)
{}
CommandBuffer::CommandBuffer(Context const& context, VkCommandBufferLevel level, VkCommandPool commandPool) :
        ContextAware(context),
        g_commandBuffer(VK_NULL_HANDLE),
        g_commandPool(VK_NULL_HANDLE),
        g_level(level),
        g_renderPassScope(RenderPassScopes::BOTH),
        g_queueType(SUPPORTED_QUEUE_ALL),
        g_recordedCommands(0),
        g_isEnded(false)
{
    this->create(level, commandPool);
}
CommandBuffer::CommandBuffer(Context const& context,
                             VkCommandBufferLevel level,
                             VkCommandBuffer commandBuffer,
                             VkCommandPool commandPool) :
        ContextAware(context),
        g_commandBuffer(commandBuffer),
        g_commandPool(commandPool),
        g_level(level),
        g_renderPassScope(RenderPassScopes::BOTH),
        g_queueType(SUPPORTED_QUEUE_ALL),
        g_recordedCommands(0),
        g_isEnded(false)
{
    this->create(level, commandBuffer, commandPool);
}
CommandBuffer::CommandBuffer(CommandBuffer&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_commandBuffer(r.g_commandBuffer),
        g_commandPool(r.g_commandPool),
        g_level(r.g_level),
        g_renderPassScope(r.g_renderPassScope),
        g_queueType(r.g_queueType),
        g_recordedCommands(r.g_recordedCommands),
        g_isEnded(r.g_isEnded)
{
    r.g_commandBuffer = VK_NULL_HANDLE;
    r.g_commandPool = VK_NULL_HANDLE;
    r.g_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    r.g_renderPassScope = RenderPassScopes::BOTH;
    r.g_queueType = SUPPORTED_QUEUE_ALL;
    r.g_recordedCommands = 0;
    r.g_isEnded = false;
}
CommandBuffer::~CommandBuffer()
{
    this->destroy();
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& r) noexcept
{
    this->verifyContext(r);
    this->destroy();

    this->g_commandBuffer = r.g_commandBuffer;
    this->g_commandPool = r.g_commandPool;
    this->g_level = r.g_level;
    this->g_renderPassScope = r.g_renderPassScope;
    this->g_queueType = r.g_queueType;
    this->g_recordedCommands = r.g_recordedCommands;
    this->g_isEnded = r.g_isEnded;

    r.g_commandBuffer = VK_NULL_HANDLE;
    r.g_commandPool = VK_NULL_HANDLE;
    r.g_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    r.g_renderPassScope = RenderPassScopes::BOTH;
    r.g_queueType = SUPPORTED_QUEUE_ALL;
    r.g_recordedCommands = 0;
    r.g_isEnded = false;
    return *this;
}

void CommandBuffer::create(VkCommandBufferLevel level, VkCommandPool commandPool)
{
    this->destroy();

    this->g_level = level;
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
void CommandBuffer::create(VkCommandBufferLevel level, VkCommandBuffer commandBuffer, VkCommandPool commandPool)
{
    this->destroy();

    if (commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("can't create a command buffer from a null handle!");
    }

    this->g_level = level;
    this->g_commandPool = commandPool;
    this->g_commandBuffer = commandBuffer;
}
void CommandBuffer::destroy()
{
    if (this->g_commandBuffer != VK_NULL_HANDLE)
    {
        if (this->g_commandPool != VK_NULL_HANDLE)
        {
            this->getContext()._garbageCollector.push(fge::vulkan::GarbageCommandBuffer(
                    this->g_commandPool, this->g_commandBuffer, this->getContext().getLogicalDevice().getDevice()));
        }
        this->g_commandBuffer = VK_NULL_HANDLE;
        this->g_commandPool = VK_NULL_HANDLE;
        this->g_queueType = SUPPORTED_QUEUE_ALL;
        this->g_renderPassScope = RenderPassScopes::BOTH;
        this->g_recordedCommands = 0;
        this->g_isEnded = false;
    }
}
std::pair<VkCommandBuffer, VkCommandPool> CommandBuffer::release()
{
    if (this->g_commandBuffer != VK_NULL_HANDLE)
    {
        auto buffer = this->g_commandBuffer;
        auto pool = this->g_commandPool;

        this->g_commandBuffer = VK_NULL_HANDLE;
        this->g_commandPool = VK_NULL_HANDLE;
        this->g_queueType = SUPPORTED_QUEUE_ALL;
        this->g_renderPassScope = RenderPassScopes::BOTH;
        this->g_recordedCommands = 0;
        this->g_isEnded = false;

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

    this->g_lastBoundPipeline = VK_NULL_HANDLE;
    this->g_lastSetViewport = {};
    this->g_lastSetScissor = {};
    this->g_lastBoundDescriptorSets.clear();
}
void CommandBuffer::begin(VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo const* inheritanceInfo)
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
    beginInfo.pInheritanceInfo = inheritanceInfo;

    if (vkBeginCommandBuffer(this->g_commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw fge::Exception("failed to begin recording command buffer!");
    }
}
void CommandBuffer::end()
{
    if (this->g_isEnded)
    {
        return;
    }

    if (vkEndCommandBuffer(this->g_commandBuffer) != VK_SUCCESS)
    {
        throw fge::Exception("failed to record command buffer!");
    }
    this->g_isEnded = true;
}

VkCommandBuffer CommandBuffer::get() const
{
    return this->g_commandBuffer;
}
VkCommandBuffer const* CommandBuffer::getPtr() const
{
    return &this->g_commandBuffer;
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
uint32_t CommandBuffer::getRecordedCommandsCount() const
{
    return this->g_recordedCommands;
}
bool CommandBuffer::isEnded() const
{
    return this->g_isEnded;
}

void CommandBuffer::forceEnd()
{
    this->g_isEnded = true;
}
void CommandBuffer::forceRenderPassScope(RenderPassScopes scope)
{
    this->g_renderPassScope = scope;
}
void CommandBuffer::forceSupportedQueues(SupportedQueueTypes_t queues)
{
    this->g_queueType = queues;
}
void CommandBuffer::forceRecordedCommandsCount(uint32_t count)
{
    this->g_recordedCommands = count;
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
    ++this->g_recordedCommands;
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
    ++this->g_recordedCommands;
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
    ++this->g_recordedCommands;
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
    ++this->g_recordedCommands;
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
    ++this->g_recordedCommands;
}

void CommandBuffer::pushConstants(VkPipelineLayout pipelineLayout,
                                  VkShaderStageFlags stageFlags,
                                  uint32_t offset,
                                  uint32_t size,
                                  void const* pValues)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if ((this->g_queueType & (SUPPORTED_QUEUE_COMPUTE | SUPPORTED_QUEUE_GRAPHICS)) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    vkCmdPushConstants(this->g_commandBuffer, pipelineLayout, stageFlags, offset, size, pValues);
    this->g_queueType &= SUPPORTED_QUEUE_COMPUTE | SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}

void CommandBuffer::beginRenderPass(VkRenderPass renderPass,
                                    VkFramebuffer framebuffer,
                                    VkExtent2D extent,
                                    VkClearValue clearColor,
                                    VkSubpassContents contents)
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
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(this->g_commandBuffer, &renderPassInfo, contents);
    this->g_renderPassScope = RenderPassScopes::INSIDE;
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}
void CommandBuffer::endRenderPass()
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if (this->g_renderPassScope == RenderPassScopes::OUTSIDE)
    {
        throw fge::Exception("Command executed outside a render pass !");
    }
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    vkCmdEndRenderPass(this->g_commandBuffer);
    this->g_renderPassScope = RenderPassScopes::OUTSIDE;
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}

void CommandBuffer::bindDescriptorSets(VkPipelineLayout pipelineLayout,
                                       VkPipelineBindPoint pipelineBindPoint,
                                       VkDescriptorSet const* descriptorSet,
                                       uint32_t descriptorCount,
                                       uint32_t firstSet)
{
    this->bindDescriptorSets(pipelineLayout, pipelineBindPoint, descriptorSet, descriptorCount, 0, nullptr, firstSet);
}

void CommandBuffer::bindDescriptorSets(VkPipelineLayout pipelineLayout,
                                       VkPipelineBindPoint pipelineBindPoint,
                                       VkDescriptorSet const* descriptorSet,
                                       uint32_t descriptorCount,
                                       uint32_t dynamicOffsetCount,
                                       uint32_t const* pDynamicOffsets,
                                       uint32_t firstSet)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if ((this->g_queueType & (SUPPORTED_QUEUE_COMPUTE | SUPPORTED_QUEUE_GRAPHICS)) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    if (descriptorCount == 1 && dynamicOffsetCount == 0)
    {
        if (!this->g_lastBoundDescriptorSets.emplace(pipelineLayout, pipelineBindPoint, *descriptorSet, firstSet).second)
        {
            return;
        }
    }

    vkCmdBindDescriptorSets(this->g_commandBuffer, pipelineBindPoint, pipelineLayout, firstSet, descriptorCount,
                            descriptorSet, dynamicOffsetCount, pDynamicOffsets);
    this->g_queueType &= SUPPORTED_QUEUE_COMPUTE | SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}

void CommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if ((this->g_queueType & (SUPPORTED_QUEUE_COMPUTE | SUPPORTED_QUEUE_GRAPHICS)) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    if (this->g_lastBoundPipeline == pipeline)
    {
        return;
    }
    this->g_lastBoundPipeline = pipeline;

    vkCmdBindPipeline(this->g_commandBuffer, pipelineBindPoint, pipeline);
    this->g_queueType &= SUPPORTED_QUEUE_COMPUTE | SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}

void CommandBuffer::setViewport(uint32_t firstViewport, uint32_t viewportCount, VkViewport const* pViewports)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    if (firstViewport == 0 && viewportCount == 1)
    {
        if (this->g_lastSetViewport.height == pViewports->height &&
            this->g_lastSetViewport.width == pViewports->width && this->g_lastSetViewport.x == pViewports->x &&
            this->g_lastSetViewport.y == pViewports->y && this->g_lastSetViewport.minDepth == pViewports->minDepth &&
            this->g_lastSetViewport.maxDepth == pViewports->maxDepth)
        {
            return;
        }
        this->g_lastSetViewport = *pViewports;
    }

    vkCmdSetViewport(this->g_commandBuffer, firstViewport, viewportCount, pViewports);
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}
void CommandBuffer::setScissor(uint32_t firstScissor, uint32_t scissorCount, VkRect2D const* pScissors)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    if (firstScissor == 0 && scissorCount == 1)
    {
        if (this->g_lastSetScissor.extent.height == pScissors->extent.height &&
            this->g_lastSetScissor.extent.width == pScissors->extent.width &&
            this->g_lastSetScissor.offset.x == pScissors->offset.x &&
            this->g_lastSetScissor.offset.y == pScissors->offset.y)
        {
            return;
        }
        this->g_lastSetScissor = *pScissors;
    }

    vkCmdSetScissor(this->g_commandBuffer, firstScissor, scissorCount, pScissors);
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}

void CommandBuffer::bindVertexBuffers(uint32_t firstBinding,
                                      uint32_t bindingCount,
                                      VkBuffer const* pBuffers,
                                      VkDeviceSize const* pOffsets)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    vkCmdBindVertexBuffers(this->g_commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}
void CommandBuffer::bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    vkCmdBindIndexBuffer(this->g_commandBuffer, buffer, offset, indexType);
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    ++this->g_recordedCommands;
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if (this->g_renderPassScope == RenderPassScopes::OUTSIDE)
    {
        throw fge::Exception("Command executed outside a render pass !");
    }
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    vkCmdDraw(this->g_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    this->g_renderPassScope = RenderPassScopes::INSIDE;
    ++this->g_recordedCommands;
}
void CommandBuffer::drawIndexed(uint32_t indexCount,
                                uint32_t instanceCount,
                                uint32_t firstIndex,
                                int32_t vertexOffset,
                                uint32_t firstInstance)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if (this->g_renderPassScope == RenderPassScopes::OUTSIDE)
    {
        throw fge::Exception("Command executed outside a render pass !");
    }
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    vkCmdDrawIndexed(this->g_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    this->g_renderPassScope = RenderPassScopes::INSIDE;
    ++this->g_recordedCommands;
}
void CommandBuffer::drawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    if (this->g_commandBuffer == VK_NULL_HANDLE)
    {
        throw fge::Exception("CommandBuffer not created !");
    }
    if (this->g_isEnded)
    {
        throw fge::Exception("CommandBuffer already ended !");
    }
    if (this->g_renderPassScope == RenderPassScopes::OUTSIDE)
    {
        throw fge::Exception("Command executed outside a render pass !");
    }
    if ((this->g_queueType & SUPPORTED_QUEUE_GRAPHICS) == 0)
    {
        throw fge::Exception("Unsupported queue type for this command buffer !");
    }

    vkCmdDrawIndirect(this->g_commandBuffer, buffer, offset, drawCount, stride);
    this->g_queueType &= SUPPORTED_QUEUE_GRAPHICS;
    this->g_renderPassScope = RenderPassScopes::INSIDE;
    ++this->g_recordedCommands;
}

} // namespace fge::vulkan