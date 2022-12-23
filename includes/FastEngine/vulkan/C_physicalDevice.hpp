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

#ifndef _FGE_VULKAN_C_PHYSICALDEVICE_HPP_INCLUDED
#define _FGE_VULKAN_C_PHYSICALDEVICE_HPP_INCLUDED

#include <vector>
#include <optional>
#include "SDL_vulkan.h"
#include "volk.h"

namespace fge::vulkan
{

class PhysicalDevice
{
public:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> _graphicsFamily;
        std::optional<uint32_t> _presentFamily;

        [[nodiscard]] inline bool isComplete() const
        {
            return _graphicsFamily.has_value() && _presentFamily.has_value();
        }
    };
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR _capabilities;

        std::vector<VkSurfaceFormatKHR> _formats;
        std::vector<VkFormatProperties> _formatProperties;

        std::vector<VkPresentModeKHR> _presentModes;
    };

    explicit PhysicalDevice(VkPhysicalDevice device=VK_NULL_HANDLE);
    PhysicalDevice(const PhysicalDevice& r) = default;
    PhysicalDevice(PhysicalDevice&& r) noexcept;
    ~PhysicalDevice() = default;

    PhysicalDevice& operator=(const PhysicalDevice& r) = default;
    PhysicalDevice& operator=(PhysicalDevice&& r) noexcept;

    [[nodiscard]] VkPhysicalDevice getDevice() const;

    [[nodiscard]] bool checkDeviceExtensionSupport() const;

    [[nodiscard]] unsigned int rateDeviceSuitability(VkSurfaceKHR surface) const;
    [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface) const;
    [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface) const;

    [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

private:
    void updateDeviceExtensionSupport();

    VkPhysicalDevice g_device{VK_NULL_HANDLE};
    bool g_extensionSupport{false};
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_PHYSICALDEVICE_HPP_INCLUDED
