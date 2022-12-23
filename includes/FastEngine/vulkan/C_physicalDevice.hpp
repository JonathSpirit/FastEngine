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
