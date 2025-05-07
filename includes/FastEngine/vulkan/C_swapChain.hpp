/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_VULKAN_C_SWAPCHAIN_HPP_INCLUDED
#define _FGE_VULKAN_C_SWAPCHAIN_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include <vector>

namespace fge::vulkan
{

class PhysicalDevice;
class LogicalDevice;
class Surface;

class FGE_API SwapChain
{
public:
    SwapChain();
    SwapChain(SwapChain const& r) = delete;
    SwapChain(SwapChain&& r) noexcept;
    ~SwapChain();

    SwapChain& operator=(SwapChain const& r) = delete;
    SwapChain& operator=(SwapChain&& r) noexcept = delete;

    void create(VkExtent2D actualExtent,
                LogicalDevice const& logicalDevice,
                PhysicalDevice const& physicalDevice,
                Surface const& surface,
                VkPresentModeKHR wantedPresentMode);
    void destroy();

    [[nodiscard]] VkSwapchainKHR getSwapChain() const;
    [[nodiscard]] std::vector<VkImage> const& getSwapChainImages() const;
    [[nodiscard]] VkFormat getSwapChainImageFormat() const;
    [[nodiscard]] VkExtent2D getSwapChainExtent() const;

    [[nodiscard]] std::vector<VkImageView> const& getSwapChainImageViews() const;

    [[nodiscard]] LogicalDevice const* getLogicalDevice() const;

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> const& availablePresentModes,
                                                  VkPresentModeKHR wantedPresentMode);
    static VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR const& capabilities, VkExtent2D actualExtent);

private:
    void createImageViews();

    VkSwapchainKHR g_swapChain;
    std::vector<VkImage> g_swapChainImages;
    VkFormat g_swapChainImageFormat;
    VkExtent2D g_swapChainExtent;

    std::vector<VkImageView> g_swapChainImageViews;

    VkPresentModeKHR g_presentMode;

    LogicalDevice const* g_logicalDevice;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_SWAPCHAIN_HPP_INCLUDED
