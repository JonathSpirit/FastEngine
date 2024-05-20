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

#ifndef _FGE_VULKAN_C_COMMANDBUFFER_HPP_INCLUDED
#define _FGE_VULKAN_C_COMMANDBUFFER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <utility>

namespace fge::vulkan
{

/**
 * \class CommandBuffer
 * \brief Vulkan command buffer wrapper
 * \ingroup vulkan
 */
class FGE_API CommandBuffer : public ContextAware
{
public:
    enum class RenderPassScopes
    {
        INSIDE,
        OUTSIDE,
        BOTH
    };

    enum SupportedQueueTypes : uint32_t
    {
        SUPPORTED_QUEUE_GRAPHICS = 1 << 0,
        SUPPORTED_QUEUE_COMPUTE = 1 << 1,
        SUPPORTED_QUEUE_TRANSFER = 1 << 2,

        SUPPORTED_QUEUE_ALL = SUPPORTED_QUEUE_GRAPHICS | SUPPORTED_QUEUE_COMPUTE | SUPPORTED_QUEUE_TRANSFER
    };
    using SupportedQueueTypes_t = std::underlying_type_t<SupportedQueueTypes>;

    CommandBuffer(Context const& context);
    CommandBuffer(Context const& context, VkCommandBufferLevel level, VkCommandPool commandPool);
    CommandBuffer(Context const& context,
                  VkCommandBufferLevel level,
                  VkCommandBuffer commandBuffer,
                  VkCommandPool commandPool);
    CommandBuffer(CommandBuffer const& r) = delete;
    CommandBuffer(CommandBuffer&& r) noexcept;
    ~CommandBuffer() override;

    CommandBuffer& operator=(CommandBuffer const& r) = delete;
    CommandBuffer& operator=(CommandBuffer&& r) noexcept;

    void create(VkCommandBufferLevel level, VkCommandPool commandPool);
    void create(VkCommandBufferLevel level, VkCommandBuffer commandBuffer, VkCommandPool commandPool);
    void destroy() final;
    [[nodiscard]] std::pair<VkCommandBuffer, VkCommandPool> release();

    void reset();
    void begin(VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo const* inheritanceInfo = nullptr);
    void end();

    [[nodiscard]] VkCommandBuffer get() const;
    [[nodiscard]] VkCommandBuffer const* getPtr() const;
    [[nodiscard]] VkCommandPool getPool() const;
    [[nodiscard]] VkCommandBufferLevel getLevel() const;
    [[nodiscard]] RenderPassScopes getRenderPassScope() const;
    [[nodiscard]] SupportedQueueTypes_t getSupportedQueues() const;
    [[nodiscard]] uint32_t getRecordedCommandsCount() const;
    [[nodiscard]] bool isEnded() const;

    void forceEnd();
    void forceRenderPassScope(RenderPassScopes scope);
    void forceSupportedQueues(SupportedQueueTypes_t queues);
    void forceRecordedCommandsCount(uint32_t count);

    /**
     * \brief Copy a buffer to another
     *
     * Fill a command buffer with a copy command in order to copy a buffer to another.
     *
     * \param srcBuffer The source buffer
     * \param dstBuffer The destination buffer
     * \param size The size of the buffer to copy
     */
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    /**
     * \brief Transition an image layout
     *
     * Fill a command buffer with a transition command in order to transition an image layout.
     *
     * \param image The image
     * \param format The format of the image
     * \param oldLayout The old layout
     * \param newLayout The new layout
     * \param mipLevels The number of mip the image have
     */
    void transitionImageLayout(VkImage image,
                               VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout,
                               uint32_t mipLevels);
    /**
     * \brief Copy a buffer to an image
     *
     * Fill a command buffer with a copy command in order to copy a buffer to an image.
     *
     * \param buffer The buffer
     * \param image The image
     * \param width Width of the image
     * \param height Height of the image
     * \param offsetX An offset on the X axis
     * \param offsetY An offset on the Y axis
     */
    void copyBufferToImage(VkBuffer buffer,
                           VkImage image,
                           uint32_t width,
                           uint32_t height,
                           int32_t offsetX = 0,
                           int32_t offsetY = 0);
    /**
     * \brief Copy an image to a buffer
     *
     * Fill a command buffer with a copy command in order to copy an image to a buffer.
     *
     * \param image The image
     * \param buffer The buffer
     * \param width The width of the image
     * \param height The height of the image
     */
    void copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height);
    /**
     * \brief Copy an image to another image
     *
     * Fill a command buffer with a copy command in order to copy an image to another image.
     *
     * \param srcImage The source image
     * \param dstImage The destination image
     * \param width The width of the image
     * \param height The height of the image
     * \param offsetX An offset on the X axis
     * \param offsetY An offset on the Y axis
     */
    void copyImageToImage(VkImage srcImage,
                          VkImage dstImage,
                          uint32_t width,
                          uint32_t height,
                          int32_t offsetX = 0,
                          int32_t offsetY = 0);

    /**
     * \brief Push constants to the pipeline
     *
     * \param pipelineLayout The pipeline layout
     * \param stageFlags The stage flags
     * \param offset The offset
     * \param size The size
     * \param pValues The values
     */
    void pushConstants(VkPipelineLayout pipelineLayout,
                       VkShaderStageFlags stageFlags,
                       uint32_t offset,
                       uint32_t size,
                       void const* pValues);

    /**
     * \brief Begin a render pass
     *
     * \param renderPass The render pass
     * \param framebuffer The framebuffer
     * \param extent The extent
     * \param clearColor The clear color
     * \param contents The subpass contents
     */
    void beginRenderPass(VkRenderPass renderPass,
                         VkFramebuffer framebuffer,
                         VkExtent2D extent,
                         VkClearValue clearColor,
                         VkSubpassContents contents);

    /**
     * \brief End a render pass
     *
     * \warning You should call this function only if you have called beginRenderPass before
     */
    void endRenderPass();

    /**
     * \brief Bind descriptor sets without dynamic parameters
     *
     * \param pipelineLayout The pipeline layout
     * \param pipelineBindPoint The pipeline bind point
     * \param descriptorSet The descriptor set
     * \param descriptorCount The descriptor count
     * \param firstSet The first set
     */
    void bindDescriptorSets(VkPipelineLayout pipelineLayout,
                            VkPipelineBindPoint pipelineBindPoint,
                            VkDescriptorSet const* descriptorSet,
                            uint32_t descriptorCount,
                            uint32_t firstSet);

    /**
     * \brief Bind descriptor sets with dynamic parameters
     *
     * \param pipelineLayout The pipeline layout
     * \param pipelineBindPoint The pipeline bind point
     * \param descriptorSet The descriptor set
     * \param descriptorCount The descriptor count
     * \param dynamicOffsetCount The dynamic offset count
     * \param pDynamicOffsets The dynamic offsets
     * \param firstSet The first set
     */
    void bindDescriptorSets(VkPipelineLayout pipelineLayout,
                            VkPipelineBindPoint pipelineBindPoint,
                            VkDescriptorSet const* descriptorSet,
                            uint32_t descriptorCount,
                            uint32_t dynamicOffsetCount,
                            uint32_t const* pDynamicOffsets,
                            uint32_t firstSet);

    /**
     * \brief Bind a pipeline
     *
     * \param pipelineBindPoint The pipeline bind point
     * \param pipeline The pipeline
     */
    void bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);

    /**
     * \brief Set the viewport dynamically
     *
     * \warning You should use this function only if the graphic pipeline has the dynamic viewport state
     *
     * \param firstViewport The first viewport
     * \param viewportCount The viewport count
     * \param pViewports The viewports
     */
    void setViewport(uint32_t firstViewport, uint32_t viewportCount, VkViewport const* pViewports);
    /**
     * \brief Set the scissor dynamically
     *
     * \warning You should use this function only if the graphic pipeline has the dynamic scissor state
     *
     * \param firstScissor The first scissor
     * \param scissorCount The scissor count
     * \param pScissors The scissors
     */
    void setScissor(uint32_t firstScissor, uint32_t scissorCount, VkRect2D const* pScissors);

    /**
     * \brief Bind vertex buffers
     *
     * \param firstBinding The first binding
     * \param bindingCount The binding count
     * \param pBuffers The buffers
     * \param pOffsets The offsets
     */
    void bindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           VkBuffer const* pBuffers,
                           VkDeviceSize const* pOffsets);
    /**
     * \brief Bind an index buffer
     *
     * \param buffer The buffer
     * \param offset The offset
     * \param indexType The index type
     */
    void bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);

    /**
     * \brief Draw
     *
     * \param vertexCount The vertex count
     * \param instanceCount The instance count
     * \param firstVertex The first vertex
     * \param firstInstance The first instance
     */
    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    /**
     * \brief Draw indexed
     *
     * \param indexCount The index count
     * \param instanceCount The instance count
     * \param firstIndex The first index
     * \param vertexOffset The vertex offset
     * \param firstInstance The first instance
     */
    void drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance);
    /**
     * \brief Draw indirect
     *
     * \param buffer The buffer
     * \param offset The offset
     * \param drawCount The draw count
     * \param stride The stride
     */
    void drawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

private:
    VkCommandBuffer g_commandBuffer;
    VkCommandPool g_commandPool;
    VkCommandBufferLevel g_level;
    RenderPassScopes g_renderPassScope;
    SupportedQueueTypes_t g_queueType;
    uint32_t g_recordedCommands;
    bool g_isEnded;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_COMMANDBUFFER_HPP_INCLUDED
