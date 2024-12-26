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

#ifndef _FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED
#define _FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#define VMA_NULLABLE
#define VMA_NOT_NULL
#define VMA_NULLABLE_NON_DISPATCHABLE
#include "vk_mem_alloc.h"

#include <vector>

//TODO: In order to have more than 1 frames in flight :
// Avoid simultaneous buffers access by having multiple ones
#define FGE_MAX_FRAMES_IN_FLIGHT 1

namespace fge::vulkan
{

class LogicalDevice;
class PhysicalDevice;
class Context;

struct BufferInfo
{
    VkBuffer _buffer{VK_NULL_HANDLE};
    VmaAllocation _allocation{VK_NULL_HANDLE};

    [[nodiscard]] inline constexpr bool valid() const
    {
        return this->_buffer != VK_NULL_HANDLE && this->_allocation != VK_NULL_HANDLE;
    }
    inline constexpr void clear()
    {
        this->_buffer = VK_NULL_HANDLE;
        this->_allocation = VK_NULL_HANDLE;
    }
};

FGE_API extern std::vector<char const*> InstanceLayers;
FGE_API extern std::vector<char const*> DeviceExtensions;
FGE_API extern std::vector<char const*> InstanceExtensions;

FGE_API extern Context& GetActiveContext();
FGE_API extern void SetActiveContext(Context& context);

FGE_API bool CheckInstanceLayerSupport(char const* layerName);

[[deprecated("see Context::createBuffer()")]] FGE_API void CreateBuffer(Context const& context,
                                                                        VkDeviceSize size,
                                                                        VkBufferUsageFlags usage,
                                                                        VkMemoryPropertyFlags properties,
                                                                        VkBuffer& buffer,
                                                                        VmaAllocation& allocation);

FGE_API void CreateImage(Context const& context,
                         uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         uint32_t mipLevels,
                         VkImage& image,
                         VmaAllocation& allocation);

FGE_API VkImageView CreateImageView(LogicalDevice const& logicalDevice,
                                    VkImage image,
                                    VkFormat format,
                                    uint32_t mipLevels);

} // namespace fge::vulkan

#endif //_FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED
