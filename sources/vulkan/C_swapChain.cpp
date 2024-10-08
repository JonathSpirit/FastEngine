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

#include "FastEngine/vulkan/C_swapChain.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/C_surface.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <algorithm>
#include <limits>

namespace fge::vulkan
{

SwapChain::SwapChain() :
        g_swapChain(VK_NULL_HANDLE),
        g_swapChainImageFormat(VK_FORMAT_UNDEFINED),
        g_swapChainExtent(),
        g_presentMode(VK_PRESENT_MODE_FIFO_KHR),
        g_logicalDevice(nullptr)
{}
SwapChain::SwapChain(SwapChain&& r) noexcept :
        g_swapChain(r.g_swapChain),
        g_swapChainImages(std::move(r.g_swapChainImages)),
        g_swapChainImageFormat(r.g_swapChainImageFormat),
        g_swapChainExtent(r.g_swapChainExtent),
        g_swapChainImageViews(std::move(r.g_swapChainImageViews)),
        g_presentMode(r.g_presentMode),
        g_logicalDevice(r.g_logicalDevice)
{
    r.g_swapChain = VK_NULL_HANDLE;
    r.g_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    r.g_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    r.g_logicalDevice = nullptr;
}
SwapChain::~SwapChain()
{
    this->destroy();
}

void SwapChain::create(VkExtent2D actualExtent,
                       LogicalDevice const& logicalDevice,
                       PhysicalDevice const& physicalDevice,
                       Surface const& surface,
                       VkPresentModeKHR wantedPresentMode)
{
    auto swapChainSupport = physicalDevice.querySwapChainSupport(surface.get());

    VkSurfaceFormatKHR const surfaceFormat = SwapChain::chooseSwapSurfaceFormat(swapChainSupport._formats);
    this->g_presentMode = SwapChain::chooseSwapPresentMode(swapChainSupport._presentModes, wantedPresentMode);
    VkExtent2D const extent = SwapChain::chooseSwapExtent(swapChainSupport._capabilities, actualExtent);

    uint32_t imageCount = swapChainSupport._capabilities.minImageCount + 1;

    if (swapChainSupport._capabilities.maxImageCount > 0 && imageCount > swapChainSupport._capabilities.maxImageCount)
    {
        imageCount = swapChainSupport._capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface.get();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    auto indices = physicalDevice.findQueueFamilies(surface.get());
    uint32_t queueFamilyIndices[] = {indices._graphicsFamily.value(), indices._presentFamily.value()};

    if (indices._graphicsFamily != indices._presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport._capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = this->g_presentMode;
    createInfo.clipped = VK_TRUE;

    auto oldSwapChain = this->g_swapChain;
    createInfo.oldSwapchain = oldSwapChain;

    if (vkCreateSwapchainKHR(logicalDevice.getDevice(), &createInfo, nullptr, &this->g_swapChain) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create swap chain!");
    }

    if (oldSwapChain != VK_NULL_HANDLE)
    {
        for (auto imageView: this->g_swapChainImageViews)
        {
            vkDestroyImageView(this->g_logicalDevice->getDevice(), imageView, nullptr);
        }
        vkDestroySwapchainKHR(logicalDevice.getDevice(), oldSwapChain, nullptr);
        this->g_swapChainImageViews.clear();
    }

    vkGetSwapchainImagesKHR(logicalDevice.getDevice(), this->g_swapChain, &imageCount, nullptr);
    this->g_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice.getDevice(), this->g_swapChain, &imageCount, this->g_swapChainImages.data());

    this->g_swapChainImageFormat = surfaceFormat.format;
    this->g_swapChainExtent = extent;

    this->g_logicalDevice = &logicalDevice;

    this->createImageViews();
}
void SwapChain::destroy()
{
    if (this->g_swapChain != VK_NULL_HANDLE)
    {
        for (auto imageView: this->g_swapChainImageViews)
        {
            vkDestroyImageView(this->g_logicalDevice->getDevice(), imageView, nullptr);
        }
        vkDestroySwapchainKHR(this->g_logicalDevice->getDevice(), this->g_swapChain, nullptr);

        this->g_swapChain = VK_NULL_HANDLE;
        this->g_swapChainImages.clear();
        this->g_swapChainImageFormat = VK_FORMAT_UNDEFINED;

        this->g_swapChainImageViews.clear();

        this->g_presentMode = VK_PRESENT_MODE_FIFO_KHR;

        this->g_logicalDevice = nullptr;
    }
}

VkSwapchainKHR SwapChain::getSwapChain() const
{
    return this->g_swapChain;
}
std::vector<VkImage> const& SwapChain::getSwapChainImages() const
{
    return this->g_swapChainImages;
}
VkFormat SwapChain::getSwapChainImageFormat() const
{
    return this->g_swapChainImageFormat;
}
VkExtent2D SwapChain::getSwapChainExtent() const
{
    return this->g_swapChainExtent;
}

std::vector<VkImageView> const& SwapChain::getSwapChainImageViews() const
{
    return this->g_swapChainImageViews;
}

LogicalDevice const* SwapChain::getLogicalDevice() const
{
    return this->g_logicalDevice;
}

void SwapChain::createImageViews()
{
    this->g_swapChainImageViews.resize(this->g_swapChainImages.size());

    for (size_t i = 0; i < this->g_swapChainImages.size(); i++)
    {
        this->g_swapChainImageViews[i] =
                CreateImageView(*this->g_logicalDevice, this->g_swapChainImages[i], this->g_swapChainImageFormat, 1);
    }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats)
{
    for (auto const& availableFormat: availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}
VkPresentModeKHR SwapChain::chooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes,
                                                  VkPresentModeKHR wantedPresentMode)
{
    for (auto const& availablePresentMode: availablePresentModes)
    {
        if (availablePresentMode == wantedPresentMode)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D SwapChain::chooseSwapExtent(VkSurfaceCapabilitiesKHR const& capabilities, VkExtent2D actualExtent)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

} // namespace fge::vulkan