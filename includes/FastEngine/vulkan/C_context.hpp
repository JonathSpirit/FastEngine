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

#ifndef _FGE_VULKAN_C_CONTEXT_HPP_INCLUDED
#define _FGE_VULKAN_C_CONTEXT_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <map>
#include <vector>

#include "FastEngine/vulkan/C_descriptorPool.hpp"
#include "FastEngine/vulkan/C_descriptorSetLayout.hpp"
#include "FastEngine/vulkan/C_garbageCollector.hpp"
#include "FastEngine/vulkan/C_graphicPipeline.hpp"
#include "FastEngine/vulkan/C_instance.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/C_surface.hpp"
#include "FastEngine/vulkan/C_swapChain.hpp"
#include "FastEngine/vulkan/C_textureImage.hpp"
#include "FastEngine/vulkan/C_uniformBuffer.hpp"

#define FGE_VULKAN_TEXTURE_BINDING 0
#define FGE_VULKAN_TRANSFORM_BINDING 0

namespace fge::vulkan
{

/**
 * \class Context
 * \brief Vulkan context
 * \ingroup vulkan
 *
 * This class is the main starting point for Vulkan usage.
 */
class FGE_API Context
{
public:
    Context();
    Context(Context const& r) = delete;
    Context(Context&& r) noexcept = delete;
    ~Context();

    Context& operator=(Context const& r) = delete;
    Context& operator=(Context&& r) noexcept = delete;

    void destroy();

    /**
     * \brief Begin a single time command
     *
     * This return a command buffer that is ready to be used.
     *
     * \warning This function must be pared with endSingleTimeCommands()
     *
     * \return The command buffer
     */
    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;
    /**
     * \brief End a single time command
     *
     * The command is queued to the graphics queue and is destroyed.
     *
     * \todo vkQueueWaitIdle is called but should be removed
     *
     * \param commandBuffer The command buffer to end
     */
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    /**
     * \brief Initialize Volk (Vulkan loader)
     *
     * \warning This function must be called once before any other graphics usage, generally at the start of the program
     */
    static void initVolk();

    /**
     * \brief Initialize Vulkan
     *
     * Once a SDL window is correctly created, this function must be called to initialize Vulkan.
     *
     * \param window The SDL window
     */
    void initVulkan(SDL_Window* window);

    /**
     * \brief Enumerate to standard output the available extensions
     *
     * \see retrieveExtensions()
     */
    static void enumerateExtensions();
    /**
     * \brief Retrieve the available extensions
     *
     * \return The available extensions
     */
    [[nodiscard]] static std::vector<std::string> retrieveExtensions();

    /**
     * \brief Wait for the device to be idle
     *
     * This is generally called before any new commands submission.
     * Also when the program is about to exit, this function must be called to make sure that all commands are finished.
     */
    void waitIdle();

    [[nodiscard]] const Instance& getInstance() const;
    [[nodiscard]] const Surface& getSurface() const;
    [[nodiscard]] const LogicalDevice& getLogicalDevice() const;
    [[nodiscard]] const PhysicalDevice& getPhysicalDevice() const;

    /**
     * \brief Retrieve a command pool for graphics commands
     *
     * This command pool is used to create command buffers that will be used to submit commands to the graphics queue.
     *
     * This command pool is created with the following flags:
     * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
     *
     * \return The command pool
     */
    [[nodiscard]] VkCommandPool getGraphicsCommandPool() const;
    /**
     * \brief Allocate graphics command buffers
     *
     * This is a shortcut for vkAllocateCommandBuffers with the graphics command pool.
     *
     * \see getGraphicsCommandPool()
     *
     * \param level The level of the command buffers (primary or secondary)
     * \param commandBuffers An array of VkCommandBuffer structures in which the resulting command buffer objects are returned
     * \param commandBufferCount The number of command buffers to allocate
     */
    void allocateGraphicsCommandBuffers(VkCommandBufferLevel level,
                                        VkCommandBuffer commandBuffers[],
                                        uint32_t commandBufferCount) const;

    /**
     * \brief Copy a buffer to another
     *
     * Fill a command buffer with a copy command in order to copy a buffer to another.
     *
     * \param srcBuffer The source buffer
     * \param dstBuffer The destination buffer
     * \param size The size of the buffer to copy
     */
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    /**
     * \brief Transition an image layout
     *
     * Fill a command buffer with a transition command in order to transition an image layout.
     *
     * \param image The image
     * \param format The format of the image
     * \param oldLayout The old layout
     * \param newLayout The new layout
     */
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
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
                           int32_t offsetY = 0) const;
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
    void copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height) const;
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
                          int32_t offsetY = 0) const;

    /**
     * \brief Retrieve or create a descriptor set layout from a key
     *
     * Certain objects need a custom descriptor set layout to be created for custom shaders.
     *
     * If the descriptor set layout is not already created, it will be created and cached and
     * you will be able to fill it with the necessary bindings.
     *
     * \param key The key to retrieve the descriptor set layout
     * \return The descriptor set layout
     */
    [[nodiscard]] fge::vulkan::DescriptorSetLayout& getCacheLayout(std::string_view key) const;
    /**
     * \brief Retrieve a "multi-usage" descriptor pool
     *
     * This pool was created with the following types:
     * VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
     * VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
     * VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
     *
     * \return The descriptor pool
     */
    [[nodiscard]] const DescriptorPool& getMultiUseDescriptorPool() const;

    /**
     * \brief Retrieve a "texture" descriptor set layout
     *
     * This layout is used with default provided shaders.
     *
     * This layout was created with the following:
     * binding: FGE_VULKAN_TEXTURE_BINDING
     * type: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
     * stage: VK_SHADER_STAGE_FRAGMENT_BIT
     *
     * \return The descriptor set layout
     */
    [[nodiscard]] const fge::vulkan::DescriptorSetLayout& getTextureLayout() const;
    /**
     * \brief Retrieve a "transform" descriptor set layout
     *
     * This layout is used with default provided shaders.
     *
     * This layout was created with the following:
     * binding: FGE_VULKAN_TRANSFORM_BINDING
     * type: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
     * stage: VK_SHADER_STAGE_VERTEX_BIT
     *
     * \return The descriptor set layout
     */
    [[nodiscard]] const fge::vulkan::DescriptorSetLayout& getTransformLayout() const;
    /**
     * \brief Retrieve a "texture" descriptor pool
     *
     * This pool can only contain VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER type.
     *
     * \return The descriptor pool
     */
    [[nodiscard]] const DescriptorPool& getTextureDescriptorPool() const;
    /**
     * \brief Retrieve a "transform" descriptor pool
     *
     * This pool can only contain VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER type.
     *
     * \return The descriptor pool
     */
    [[nodiscard]] const DescriptorPool& getTransformDescriptorPool() const;

    /**
     * \brief Retrieve the VMA (Vulkan Memory Allocator)
     *
     * \return The allocator
     */
    [[nodiscard]] VmaAllocator getAllocator() const;

    fge::vulkan::GarbageCollector _garbageCollector;

private:
    void createCommandPool();
    void createMultiUseDescriptorPool();
    void createTextureDescriptorPool();
    void createTransformDescriptorPool();

    Instance g_instance;
    PhysicalDevice g_physicalDevice;
    LogicalDevice g_logicalDevice;
    Surface g_surface;

    mutable std::map<std::string, fge::vulkan::DescriptorSetLayout, std::less<>> g_cacheLayouts;
    DescriptorPool g_multiUseDescriptorPool;

    fge::vulkan::DescriptorSetLayout g_textureLayout;
    fge::vulkan::DescriptorSetLayout g_transformLayout;
    DescriptorPool g_textureDescriptorPool;
    DescriptorPool g_transformDescriptorPool;

    mutable VmaAllocator g_allocator;

    VkCommandPool g_graphicsCommandPool;
    bool g_isCreated;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_CONTEXT_HPP_INCLUDED
