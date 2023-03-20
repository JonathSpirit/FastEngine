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

class FGE_API Context
{
public:
    Context();
    Context(const Context& r) = delete;
    Context(Context&& r) noexcept = delete;
    ~Context();

    Context& operator=(const Context& r) = delete;
    Context& operator=(Context&& r) noexcept = delete;

    void destroy();

    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    static void initVolk();

    void initVulkan(SDL_Window* window);
    static void enumerateExtensions();

    void waitIdle();

    [[nodiscard]] const Instance& getInstance() const;
    [[nodiscard]] const Surface& getSurface() const;
    [[nodiscard]] const LogicalDevice& getLogicalDevice() const;
    [[nodiscard]] const PhysicalDevice& getPhysicalDevice() const;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
    void copyBufferToImage(VkBuffer buffer,
                           VkImage image,
                           uint32_t width,
                           uint32_t height,
                           int32_t offsetX = 0,
                           int32_t offsetY = 0) const;
    void copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height) const;
    void copyImageToImage(VkImage srcImage,
                          VkImage dstImage,
                          uint32_t width,
                          uint32_t height,
                          int32_t offsetX = 0,
                          int32_t offsetY = 0) const;

    [[nodiscard]] fge::vulkan::DescriptorSetLayout& getCacheLayout(std::string_view key) const;
    [[nodiscard]] const DescriptorPool& getMultiUseDescriptorPool() const;

    [[nodiscard]] const fge::vulkan::DescriptorSetLayout& getTextureLayout() const;
    [[nodiscard]] const fge::vulkan::DescriptorSetLayout& getTransformLayout() const;
    [[nodiscard]] const fge::vulkan::DescriptorSetLayout& getTransformBatchesLayout() const;
    [[nodiscard]] const DescriptorPool& getTextureDescriptorPool() const;
    [[nodiscard]] const DescriptorPool& getTransformDescriptorPool() const;
    [[nodiscard]] const DescriptorPool& getTransformBatchesDescriptorPool() const;

    [[nodiscard]] VmaAllocator getAllocator() const;

    fge::vulkan::GarbageCollector _garbageCollector;

private:
    void createCommandPool();
    void createMultiUseDescriptorPool();
    void createTextureDescriptorPool();
    void createTransformDescriptorPool();
    void createTransformBatchesDescriptorPool();

    Instance g_instance;
    PhysicalDevice g_physicalDevice;
    LogicalDevice g_logicalDevice;
    Surface g_surface;

    mutable std::map<std::string, fge::vulkan::DescriptorSetLayout, std::less<>> g_cacheLayouts;
    DescriptorPool g_multiUseDescriptorPool;

    fge::vulkan::DescriptorSetLayout g_textureLayout;
    fge::vulkan::DescriptorSetLayout g_transformLayout;
    fge::vulkan::DescriptorSetLayout g_transformBatchesLayout;
    DescriptorPool g_textureDescriptorPool;
    DescriptorPool g_transformDescriptorPool;
    DescriptorPool g_transformBatchesDescriptorPool;

    mutable VmaAllocator g_allocator;

    VkCommandPool g_commandPool;
    bool g_isCreated;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_CONTEXT_HPP_INCLUDED
