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

#ifndef _FGE_VULKAN_C_PHYSICALDEVICE_HPP_INCLUDED
#define _FGE_VULKAN_C_PHYSICALDEVICE_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"
#include "SDL_vulkan.h"
#include <optional>
#include <vector>

namespace fge::vulkan
{

/**
 * \class PhysicalDevice
 * \ingroup vulkan
 * \brief Vulkan physical device abstraction
 */
class FGE_API PhysicalDevice
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

    explicit PhysicalDevice(VkPhysicalDevice device = VK_NULL_HANDLE);
    PhysicalDevice(PhysicalDevice const& r) = default;
    PhysicalDevice(PhysicalDevice&& r) noexcept;
    ~PhysicalDevice() = default;

    PhysicalDevice& operator=(PhysicalDevice const& r) = default;
    PhysicalDevice& operator=(PhysicalDevice&& r) noexcept;

    [[nodiscard]] VkPhysicalDevice getDevice() const;

    /**
     * \brief Check if the device support the required extensions
     *
     * This will check extensions in fge::vulkan::DeviceExtensions.
     *
     * \return \b true if the device support the required extensions, \b false otherwise
     */
    [[nodiscard]] bool checkDeviceExtensionSupport() const;

    /**
     * \brief Give a score to the device
     *
     * This score is used to choose the best device, the higher the score, the better the device.
     * A discrete GPU will have a higher score than an integrated GPU.
     *
     * A score of 0 means the device is not suitable.
     *
     * \param surface The associated surface
     * \return The score of the device
     */
    [[nodiscard]] unsigned int rateDeviceSuitability(VkSurfaceKHR surface) const;
    /**
     * \brief Retrieve the QueueFamilyIndices
     *
     * A QueueFamilyIndices is a struct containing the index of the graphic and present queue families
     * if they exist.
     *
     * \param surface The associated surface
     * \return The QueueFamilyIndices
     */
    [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface) const;
    [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface) const;

    [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    [[nodiscard]] uint32_t getMaxImageDimension2D() const;
    [[nodiscard]] uint32_t getMinUniformBufferOffsetAlignment() const;
    [[nodiscard]] VkDeviceSize getMaxMemoryAllocationSize() const;
    [[nodiscard]] uint32_t getMaxMemoryAllocationCount() const;

private:
    void updateDeviceExtensionSupport();

    VkPhysicalDevice g_device;
    bool g_extensionSupport;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_PHYSICALDEVICE_HPP_INCLUDED
