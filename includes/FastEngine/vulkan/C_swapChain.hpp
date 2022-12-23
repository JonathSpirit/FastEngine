#ifndef _FGE_VULKAN_C_SWAPCHAIN_HPP_INCLUDED
#define _FGE_VULKAN_C_SWAPCHAIN_HPP_INCLUDED

#include "SDL_vulkan.h"
#include "volk.h"
#include <vector>

namespace fge::vulkan
{

class PhysicalDevice;
class LogicalDevice;
class Surface;

class SwapChain
{
public:
    SwapChain();
    SwapChain(const SwapChain& r) = delete;
    SwapChain(SwapChain&& r) noexcept;
    ~SwapChain();

    SwapChain& operator=(const SwapChain& r) = delete;
    SwapChain& operator=(SwapChain&& r) noexcept = delete;

    void create(SDL_Window* window, const LogicalDevice& logicalDevice, const PhysicalDevice& physicalDevice, const Surface& surface);
    void destroy();

    [[nodiscard]] VkSwapchainKHR getSwapChain() const;
    [[nodiscard]] const std::vector<VkImage>& getSwapChainImages() const;
    [[nodiscard]] VkFormat getSwapChainImageFormat() const;
    [[nodiscard]] VkExtent2D getSwapChainExtent() const;

    [[nodiscard]] const std::vector<VkImageView>& getSwapChainImageViews() const;

    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window* window);

private:
    void createImageViews();

    VkSwapchainKHR g_swapChain;
    std::vector<VkImage> g_swapChainImages;
    VkFormat g_swapChainImageFormat;
    VkExtent2D g_swapChainExtent;

    std::vector<VkImageView> g_swapChainImageViews;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_SWAPCHAIN_HPP_INCLUDED
