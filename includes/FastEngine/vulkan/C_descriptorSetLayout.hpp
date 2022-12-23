#ifndef _FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED

#include "SDL_vulkan.h"
#include "volk.h"
#include <initializer_list>

namespace fge::vulkan
{

class LogicalDevice;

class DescriptorSetLayout
{
public:
    struct Layout
    {
        VkDescriptorType _type;
        uint32_t _binding;
        VkShaderStageFlags _stage;
    };

    DescriptorSetLayout();
    DescriptorSetLayout(const DescriptorSetLayout& r) = delete;
    DescriptorSetLayout(DescriptorSetLayout&& r) noexcept;
    ~DescriptorSetLayout();

    DescriptorSetLayout& operator=(const DescriptorSetLayout& r) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& r) noexcept = delete;

    void create(const LogicalDevice& logicalDevice, std::initializer_list<Layout> layouts);
    void destroy();

    [[nodiscard]] VkDescriptorSetLayout getLayout() const;
    [[nodiscard]] std::size_t getLayoutSize() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

private:
    VkDescriptorSetLayout g_descriptorSetLayout;
    std::size_t g_layoutSize;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED
