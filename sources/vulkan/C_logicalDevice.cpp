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

#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <cstring>
#include <set>
#include <vector>

namespace fge::vulkan
{

LogicalDevice::LogicalDevice() :
        g_device(VK_NULL_HANDLE),
        g_graphicQueue(VK_NULL_HANDLE),
        g_computeQueue(VK_NULL_HANDLE),
        g_transferQueue(VK_NULL_HANDLE),
        g_presentQueue(VK_NULL_HANDLE)
{}
LogicalDevice::LogicalDevice(LogicalDevice&& r) noexcept :
        g_device(r.g_device),
        g_graphicQueue(r.g_graphicQueue),
        g_computeQueue(r.g_computeQueue),
        g_transferQueue(r.g_transferQueue),
        g_presentQueue(r.g_presentQueue)
{
    r.g_device = VK_NULL_HANDLE;
    r.g_graphicQueue = VK_NULL_HANDLE;
    r.g_computeQueue = VK_NULL_HANDLE;
    r.g_transferQueue = VK_NULL_HANDLE;
    r.g_presentQueue = VK_NULL_HANDLE;
}
LogicalDevice::~LogicalDevice()
{
    this->destroy();
}

void LogicalDevice::create(PhysicalDevice& physicalDevice, VkSurfaceKHR surface)
{
    auto indices = physicalDevice.findQueueFamilies(surface);

    std::set<uint32_t> uniqueQueueFamilies = {indices._graphicsFamily.value()};

    if (indices._presentFamily)
    {
        uniqueQueueFamilies.insert(indices._presentFamily.value());
    }
    if (indices._transferFamily)
    {
        uniqueQueueFamilies.insert(indices._transferFamily.value());
    }
    if (indices._computeFamily)
    {
        uniqueQueueFamilies.insert(indices._computeFamily.value());
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float const queuePriority = 1.0f;
    for (auto queueFamily: uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    std::memset(&this->g_enabledFeatures, 0, sizeof(this->g_enabledFeatures));
    auto const availableFeatures = physicalDevice.getFeatures();

    if (availableFeatures.samplerAnisotropy == VK_FALSE)
    {
        throw fge::Exception("Device does not support samplerAnisotropy feature !");
    }
    this->g_enabledFeatures.samplerAnisotropy = VK_TRUE;
    this->g_enabledFeatures.geometryShader = availableFeatures.geometryShader;
    this->g_enabledFeatures.multiDrawIndirect = availableFeatures.multiDrawIndirect;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &this->g_enabledFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    //https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
    //Device Layer Deprecation, but we still need this for compatibility
    createInfo.enabledLayerCount = static_cast<uint32_t>(InstanceLayers.size());
    createInfo.ppEnabledLayerNames = InstanceLayers.empty() ? nullptr : InstanceLayers.data();

    // Extended features goes here

    auto const availableRobustness2Features = physicalDevice.getRobustness2Features();
    VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
            .pNext = nullptr,
            .robustBufferAccess2 = VK_FALSE,
            .robustImageAccess2 = VK_FALSE,
            .nullDescriptor = availableRobustness2Features.nullDescriptor};

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
            .pNext = &robustness2Features,
            .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
            .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
            .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
            .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderSampledImageArrayNonUniformIndexing = VK_FALSE,
            .shaderStorageBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderStorageImageArrayNonUniformIndexing = VK_FALSE,
            .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
            .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
            .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
            .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_FALSE,
            .descriptorBindingStorageImageUpdateAfterBind = VK_FALSE,
            .descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
            .descriptorBindingUpdateUnusedWhilePending = VK_FALSE,
            .descriptorBindingPartiallyBound = VK_FALSE,
            .descriptorBindingVariableDescriptorCount = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE};

    createInfo.pNext = &descriptorIndexingFeatures;

    if (vkCreateDevice(physicalDevice.getDevice(), &createInfo, nullptr, &this->g_device) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create logical device!");
    }

    this->g_presentQueue = VK_NULL_HANDLE;
    if (indices._presentFamily)
    {
        vkGetDeviceQueue(this->g_device, indices._presentFamily.value(), 0, &this->g_presentQueue);
    }
    this->g_transferQueue = VK_NULL_HANDLE;
    if (indices._transferFamily)
    {
        vkGetDeviceQueue(this->g_device, indices._transferFamily.value(), 0, &this->g_transferQueue);
    }
    this->g_computeQueue = VK_NULL_HANDLE;
    if (indices._computeFamily)
    {
        vkGetDeviceQueue(this->g_device, indices._computeFamily.value(), 0, &this->g_computeQueue);
    }
    vkGetDeviceQueue(this->g_device, indices._graphicsFamily.value(), 0, &this->g_graphicQueue);
}
void LogicalDevice::destroy()
{
    if (this->g_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(this->g_device, nullptr);
        this->g_device = VK_NULL_HANDLE;
        this->g_graphicQueue = VK_NULL_HANDLE;
        this->g_computeQueue = VK_NULL_HANDLE;
        this->g_transferQueue = VK_NULL_HANDLE;
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
VkQueue LogicalDevice::getComputeQueue() const
{
    return this->g_computeQueue;
}
VkQueue LogicalDevice::getTransferQueue() const
{
    return this->g_transferQueue;
}
VkQueue LogicalDevice::getPresentQueue() const
{
    return this->g_presentQueue;
}
VkPhysicalDeviceFeatures LogicalDevice::getEnabledFeatures() const
{
    return this->g_enabledFeatures;
}

VkImageView LogicalDevice::createImageView(VkImage image, VkFormat format, uint32_t mipLevels) const
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
    if (vkCreateImageView(this->g_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }
    return imageView;
}

} // namespace fge::vulkan