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

#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#ifdef FGE_DEF_DEBUG
    #include <iostream>
#endif
#include <set>
#include <stdexcept>
#include <string>

namespace fge::vulkan
{

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device) :
        g_device(device),
        g_extensionSupport(false)
{
    if (device != VK_NULL_HANDLE)
    {
        this->updateDeviceExtensionSupport();
    }
}
PhysicalDevice::PhysicalDevice(PhysicalDevice&& r) noexcept :
        g_device(r.g_device),
        g_extensionSupport(r.g_extensionSupport)
{
    r.g_device = VK_NULL_HANDLE;
    r.g_extensionSupport = false;
}

PhysicalDevice& PhysicalDevice::operator=(PhysicalDevice&& r) noexcept
{
    this->g_device = r.g_device;
    this->g_extensionSupport = r.g_extensionSupport;

    r.g_device = VK_NULL_HANDLE;
    r.g_extensionSupport = false;
    return *this;
}

VkPhysicalDevice PhysicalDevice::getDevice() const
{
    return this->g_device;
}

bool PhysicalDevice::checkDeviceExtensionSupport() const
{
    return this->g_extensionSupport;
}
void PhysicalDevice::updateDeviceExtensionSupport()
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(this->g_device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(this->g_device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    for (const auto& extension: availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    this->g_extensionSupport = requiredExtensions.empty();
}

unsigned int PhysicalDevice::rateDeviceSuitability(VkSurfaceKHR surface) const
{
    if (this->g_device == VK_NULL_HANDLE)
    {
        return 0;
    }

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(this->g_device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(this->g_device, &deviceFeatures);

    unsigned int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader)
    {
        return 0;
    }

    QueueFamilyIndices indices = this->findQueueFamilies(surface);
    if (!indices.isComplete())
    {
        return 0;
    }

    if (!this->checkDeviceExtensionSupport())
    {
        return 0;
    }

    SwapChainSupportDetails swapChainSupport = this->querySwapChainSupport(surface);
    bool swapChainAdequate = !swapChainSupport._formats.empty() && !swapChainSupport._presentModes.empty();
    if (!swapChainAdequate)
    {
        return 0;
    }

    return score;
}
PhysicalDevice::QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkSurfaceKHR surface) const
{
    QueueFamilyIndices indices;
    // Logic to find queue family indices to populate struct with

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(this->g_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(this->g_device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily: queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices._graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(this->g_device, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices._presentFamily = i;
        }

        i++;
    }

    return indices;
}
PhysicalDevice::SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkSurfaceKHR surface) const
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->g_device, surface, &details._capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(this->g_device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details._formats.resize(formatCount);
        details._formatProperties.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(this->g_device, surface, &formatCount, details._formats.data());

#ifdef FGE_DEF_DEBUG
        std::cout << "Available formats: " << std::endl;
#endif
        for (uint32_t i = 0; i < formatCount; ++i)
        {
#ifdef FGE_DEF_DEBUG
            std::cout << "\tSurface format: color space = " << details._formats[i].colorSpace
                      << ", pixel format = " << details._formats[i].format << std::endl;
#endif

            VkFormatProperties2 formatProperties2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2, nullptr, {}};
            vkGetPhysicalDeviceFormatProperties2(this->g_device, details._formats[i].format, &formatProperties2);
            details._formatProperties[i] = formatProperties2.formatProperties;

#ifdef FGE_DEF_DEBUG
            std::cout << "\t\tSurface properties: linearTilingFeatures = "
                      << formatProperties2.formatProperties.linearTilingFeatures
                      << ", optimalTilingFeatures = " << formatProperties2.formatProperties.optimalTilingFeatures
                      << ", bufferFeatures = " << formatProperties2.formatProperties.bufferFeatures << std::endl;
#endif
        }
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(this->g_device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details._presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(this->g_device, surface, &presentModeCount,
                                                  details._presentModes.data());
    }

    return details;
}

uint32_t PhysicalDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(this->g_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

uint32_t PhysicalDevice::getMaxImageDimension2D() const
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(this->g_device, &deviceProperties);
    return deviceProperties.limits.maxImageDimension2D;
}
uint32_t PhysicalDevice::getMinUniformBufferOffsetAlignment() const
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(this->g_device, &deviceProperties);
    return deviceProperties.limits.minUniformBufferOffsetAlignment;
}
VkDeviceSize PhysicalDevice::getMaxMemoryAllocationSize() const
{
    VkPhysicalDeviceMaintenance3Properties deviceMaintenance3Properties{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES,
            nullptr,
            {},
            {}};
    VkPhysicalDeviceProperties2 deviceProperties2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                                                  &deviceMaintenance3Properties,
                                                  {}};
    vkGetPhysicalDeviceProperties2(this->g_device, &deviceProperties2);
    return deviceMaintenance3Properties.maxMemoryAllocationSize;
}
uint32_t PhysicalDevice::getMaxMemoryAllocationCount() const
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(this->g_device, &deviceProperties);
    return deviceProperties.limits.maxMemoryAllocationCount;
}

} // namespace fge::vulkan