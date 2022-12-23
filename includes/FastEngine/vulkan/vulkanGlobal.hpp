#ifndef _FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED
#define _FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED

#include <vector>
#include "volk.h"

namespace fge::vulkan
{

class LogicalDevice;
class PhysicalDevice;

extern const std::vector<const char*> validationLayers;
extern const std::vector<const char*> deviceExtensions;

void CreateBuffer(const LogicalDevice& logicalDevice, const PhysicalDevice& physicalDevice,
                  VkDeviceSize size,
                  VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkBuffer& buffer, VkDeviceMemory& bufferMemory);

void CreateImage(const LogicalDevice& logicalDevice, const PhysicalDevice& physicalDevice,
                 uint32_t width, uint32_t height, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                 VkImage& image, VkDeviceMemory& imageMemory);

VkImageView CreateImageView(const LogicalDevice& logicalDevice, VkImage image, VkFormat format);

}//end fge::vulkan

#endif //_FGE_VULKAN_VULKANGLOBAL_HPP_INCLUDED
