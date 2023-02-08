/*
 * Copyright 2022 Guillaume Guillet
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

#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <stdexcept>
#include <vector>

namespace fge::vulkan
{

LogicalDevice::LogicalDevice() :
        g_device(VK_NULL_HANDLE),
        g_graphicQueue(VK_NULL_HANDLE),
        g_presentQueue(VK_NULL_HANDLE)
{}
LogicalDevice::LogicalDevice(LogicalDevice&& r) noexcept :
        g_device(r.g_device),
        g_graphicQueue(r.g_graphicQueue),
        g_presentQueue(r.g_presentQueue)
{
    r.g_device = VK_NULL_HANDLE;
    r.g_graphicQueue = VK_NULL_HANDLE;
    r.g_presentQueue = VK_NULL_HANDLE;
}
LogicalDevice::~LogicalDevice()
{
    this->destroy();
}

void LogicalDevice::create(PhysicalDevice& physicalDevice, VkSurfaceKHR surface)
{
    auto indices = physicalDevice.findQueueFamilies(surface);

    const std::vector<uint32_t> uniqueQueueFamilies = {indices._graphicsFamily.value(), indices._presentFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    const float queuePriority = 1.0f;
    for (auto queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

#ifdef NDEBUG
    createInfo.enabledLayerCount = 0;
#else
    createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
    createInfo.ppEnabledLayerNames = ValidationLayers.data();
#endif

    const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicStateFeatures{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT, nullptr, VK_TRUE};

    createInfo.pNext = &dynamicStateFeatures;

    if (vkCreateDevice(physicalDevice.getDevice(), &createInfo, nullptr, &this->g_device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(this->g_device, indices._graphicsFamily.value(), 0, &this->g_graphicQueue);
    vkGetDeviceQueue(this->g_device, indices._presentFamily.value(), 0, &this->g_presentQueue);
}
void LogicalDevice::destroy()
{
    if (this->g_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(this->g_device, nullptr);
        this->g_device = VK_NULL_HANDLE;
        this->g_graphicQueue = VK_NULL_HANDLE;
        this->g_presentQueue = VK_NULL_HANDLE;
    }
}

VkDevice LogicalDevice::getDevice() const
{
    return this->g_device;
}
VkQueue LogicalDevice::getGraphicQueue() const
{
    return this->g_graphicQueue;
}
VkQueue LogicalDevice::getPresentQueue() const
{
    return this->g_presentQueue;
}

} // namespace fge::vulkan