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

#ifndef _FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED
#define _FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"
#include "vk_mem_alloc.h"
#include <vector>

#define FGE_MAX_FRAMES_IN_FLIGHT 2

namespace fge::vulkan
{

class LogicalDevice;
class PhysicalDevice;
class Context;

FGE_API extern const std::vector<const char*> ValidationLayers;
FGE_API extern const std::vector<const char*> DeviceExtensions;

FGE_API extern Context* GlobalContext;

FGE_API bool CheckValidationLayerSupport(const char* layerName);

FGE_API void CreateBuffer(const Context& context,
                          VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer& buffer,
                          VmaAllocation& allocation);

FGE_API void CreateImage(const Context& context,
                         uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage& image,
                         VmaAllocation& allocation);

FGE_API VkImageView CreateImageView(const LogicalDevice& logicalDevice, VkImage image, VkFormat format);

} // namespace fge::vulkan

#endif //_FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED
