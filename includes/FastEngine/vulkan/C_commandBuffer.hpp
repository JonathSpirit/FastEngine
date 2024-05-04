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
    enum class SubmitTypes
    {
        DIRECT_EXECUTION,
        INDIRECT_EXECUTION,
        INDIRECT_ISOLATED_EXECUTION
    };

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

    CommandBuffer(Context const& context, VkCommandBufferLevel level, SubmitTypes type, VkCommandPool commandPool);
    CommandBuffer(CommandBuffer const& r) = delete;
    CommandBuffer(CommandBuffer&& r) noexcept = delete;
    ~CommandBuffer() override;

    CommandBuffer& operator=(CommandBuffer const& r) = delete;
    CommandBuffer& operator=(CommandBuffer&& r) noexcept = delete;

    void create(VkCommandBufferLevel level, SubmitTypes type, VkCommandPool commandPool);
    void destroy() final;
    [[nodiscard]] std::pair<VkCommandBuffer, VkCommandPool> release();

    void reset();
    void begin(VkCommandBufferUsageFlags flags);
    void end();

    [[nodiscard]] SubmitTypes getSubmitType() const;
    [[nodiscard]] VkCommandBuffer get() const;
    [[nodiscard]] VkCommandPool getPool() const;
    [[nodiscard]] VkCommandBufferLevel getLevel() const;
    [[nodiscard]] RenderPassScopes getRenderPassScope() const;
    [[nodiscard]] SupportedQueueTypes_t getSupportedQueues() const;
    [[nodiscard]] bool isEnded() const;

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

private:
    SubmitTypes g_type;
    VkCommandBuffer g_commandBuffer;
    VkCommandPool g_commandPool;
    VkCommandBufferLevel g_level;
    RenderPassScopes g_renderPassScope;
    SupportedQueueTypes_t g_queueType;
    bool g_isEnded;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_COMMANDBUFFER_HPP_INCLUDED
