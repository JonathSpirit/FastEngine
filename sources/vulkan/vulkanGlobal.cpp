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

#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include <cstring>

extern "C" {
#include "volk.c" //Including the volk implementation
}

namespace fge::vulkan
{

#ifdef FGE_ENABLE_VALIDATION_LAYERS
std::vector<char const*> InstanceLayers = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"};
#else
std::vector<char const*> InstanceLayers = {};
#endif

std::vector<char const*> DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                             VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};

std::vector<char const*> InstanceExtensions = {};

namespace
{

Context* gActiveContext{nullptr};

} // namespace

Context& GetActiveContext()
{
#if defined(FGE_DEF_DEBUG) && !defined(FGE_DEF_SERVER)
    if (gActiveContext == nullptr)
    {
        throw fge::Exception("No active context !");
    }
#endif

    return *gActiveContext;
}
void SetActiveContext(Context& context)
{
    gActiveContext = &context;
}

bool CheckInstanceLayerSupport(char const* layerName)
{
    static std::vector<VkLayerProperties> availableLayers;

    if (availableLayers.empty())
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        availableLayers.resize(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    }

    for (auto const& layerProperties: availableLayers)
    {
        if (std::strcmp(layerName, layerProperties.layerName) == 0)
        {
            return true;
        }
    }
    return false;
}

void CreateBuffer(Context const& context,
                  VkDeviceSize size,
                  VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkBuffer& buffer,
                  VmaAllocation& allocation)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.requiredFlags = properties;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;

    auto result =
            vmaCreateBuffer(context.getAllocator(), &bufferInfo, &allocationCreateInfo, &buffer, &allocation, nullptr);

    if (result != VK_SUCCESS)
    {
        throw fge::Exception("failed to create buffer!");
    }
}

void CreateImage(Context const& context,
                 uint32_t width,
                 uint32_t height,
                 VkFormat format,
                 VkImageTiling tiling,
                 VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties,
                 uint32_t mipLevels,
                 VkImage& image,
                 VmaAllocation& allocation)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;

    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocationCreateInfo.requiredFlags = properties;
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;

    auto result =
            vmaCreateImage(context.getAllocator(), &imageInfo, &allocationCreateInfo, &image, &allocation, nullptr);

    if (result != VK_SUCCESS)
    {
        throw fge::Exception("failed to create image!");
    }
}

VkImageView CreateImageView(LogicalDevice const& logicalDevice, VkImage image, VkFormat format, uint32_t mipLevels)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView = VK_NULL_HANDLE;
    if (vkCreateImageView(logicalDevice.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create texture image view!");
    }

    return imageView;
}

} // namespace fge::vulkan