#ifndef _FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED
#define _FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED

#include "SDL_vulkan.h"
#include "volk.h"

namespace fge::vulkan
{

class PhysicalDevice;

class LogicalDevice
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

}//end fge::vulkan

#endif //_FGE_VULKAN_C_LOGICALDEVICE_HPP_INCLUDED
