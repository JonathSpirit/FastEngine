#ifndef _FGE_VULKAN_C_CONTEXT_HPP_INCLUDED
#define _FGE_VULKAN_C_CONTEXT_HPP_INCLUDED

#include <vector>
#include "volk.h"

#include "FastEngine/vulkan/C_instance.hpp"
#include "FastEngine/vulkan/C_surface.hpp"
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_swapChain.hpp"
#include "FastEngine/vulkan/C_graphicPipeline.hpp"
#include "FastEngine/vulkan/C_uniformBuffer.hpp"
#include "FastEngine/vulkan/C_textureImage.hpp"
#include "FastEngine/vulkan/C_descriptorSetLayout.hpp"
#include "FastEngine/vulkan/C_descriptorPool.hpp"

namespace fge::vulkan
{

class Context
{
public:
    Context();
    Context(const Context& r) = delete;
    Context(Context&& r) noexcept = delete;
    ~Context();

    Context& operator=(const Context& r) = delete;
    Context& operator=(Context&& r) noexcept = delete;

    void destroy();

    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    static void initVolk();

    void initVulkan(SDL_Window* window);
    static void enumerateExtensions();

    void waitIdle();

    [[nodiscard]] const Instance& getInstance() const;
    [[nodiscard]] const Surface& getSurface() const;
    [[nodiscard]] const LogicalDevice& getLogicalDevice() const;
    [[nodiscard]] const PhysicalDevice& getPhysicalDevice() const;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;

    [[nodiscard]] const fge::vulkan::DescriptorSetLayout& getDescriptorSetLayout() const;
    [[nodiscard]] const DescriptorPool& getTextureDescriptorPool() const;
    [[nodiscard]] const DescriptorPool& getTransformDescriptorPool() const;

private:
    void createCommandPool();
    void createTextureDescriptorPool();
    void createTransformDescriptorPool();

    Instance g_instance;
    PhysicalDevice g_physicalDevice;
    LogicalDevice g_logicalDevice;
    Surface g_surface;

    fge::vulkan::DescriptorSetLayout g_descriptorSetLayout;
    DescriptorPool g_textureDescriptorPool;
    DescriptorPool g_transformDescriptorPool;

    VkCommandPool g_commandPool;
    bool g_isCreated;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_CONTEXT_HPP_INCLUDED
