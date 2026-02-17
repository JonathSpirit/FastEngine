/*
 * Copyright 2026 Guillaume Guillet
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

#ifndef _FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED
#define _FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"

namespace fge::vulkan
{

class PhysicalDevice;

/**
 * \class LogicalDevice
 * \ingroup vulkan
 * \brief Logical device abstraction
 *
 * This class is used to create a Vulkan logical device and get the
 * graphic and present queues.
 */
class FGE_API LogicalDevice
{
public:
    LogicalDevice();
    LogicalDevice(LogicalDevice const& r) = delete;
    LogicalDevice(LogicalDevice&& r) noexcept;
    ~LogicalDevice();

    LogicalDevice& operator=(LogicalDevice const& r) = delete;
    LogicalDevice& operator=(LogicalDevice&& r) noexcept = delete;

    void create(PhysicalDevice& physicalDevice, VkSurfaceKHR surface);
    void destroy();

    [[nodiscard]] VkDevice getDevice() const;
    [[nodiscard]] VkQueue getGraphicQueue() const;
    [[nodiscard]] VkQueue getComputeQueue() const;
    [[nodiscard]] VkQueue getTransferQueue() const;
    [[nodiscard]] VkQueue getPresentQueue() const;
    [[nodiscard]] VkPhysicalDeviceFeatures getEnabledFeatures() const;

    [[nodiscard]] VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels) const;

private:
    VkDevice g_device;
    VkQueue g_graphicQueue;
    VkQueue g_computeQueue;
    VkQueue g_transferQueue;
    VkQueue g_presentQueue;
    VkPhysicalDeviceFeatures g_enabledFeatures;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED
