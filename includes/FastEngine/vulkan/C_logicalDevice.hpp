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

#ifndef _FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED
#define _FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"

namespace fge::vulkan
{

class PhysicalDevice;

class FGE_API LogicalDevice
{
public:
    LogicalDevice();
    LogicalDevice(const LogicalDevice& r) = delete;
    LogicalDevice(LogicalDevice&& r) noexcept;
    ~LogicalDevice();

    LogicalDevice& operator=(const LogicalDevice& r) = delete;
    LogicalDevice& operator=(LogicalDevice&& r) noexcept = delete;

    void create(PhysicalDevice& physicalDevice, VkSurfaceKHR surface);
    void destroy();

    [[nodiscard]] VkDevice getDevice() const;
    [[nodiscard]] VkQueue getGraphicQueue() const;
    [[nodiscard]] VkQueue getPresentQueue() const;

private:
    VkDevice g_device;
    VkQueue g_graphicQueue;
    VkQueue g_presentQueue;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED
