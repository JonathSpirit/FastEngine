#ifndef _FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED

#include "SDL_vulkan.h"
#include "volk.h"
#include <variant>
#include <utility>

namespace fge::vulkan
{

class LogicalDevice;
class DescriptorSetLayout;
class DescriptorPool;
class UniformBuffer;
class TextureImage;

class DescriptorSet
{
public:
    struct Descriptor
    {
        Descriptor() = default;
        Descriptor(const UniformBuffer& uniformBuffer, uint32_t binding);
        Descriptor(const TextureImage& textureImage, uint32_t binding);

        std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo> _data;
        uint32_t _binding{0};
    };

    DescriptorSet();
    DescriptorSet(const DescriptorSet& r);
    DescriptorSet(DescriptorSet&& r) noexcept;
    ~DescriptorSet();

    DescriptorSet& operator=(const DescriptorSet& r);
    DescriptorSet& operator=(DescriptorSet&& r) noexcept = delete;

    void create(const LogicalDevice& logicalDevice,
                const DescriptorSetLayout* layouts,
                std::size_t layoutSize,
                const DescriptorPool& pool,
                bool freeFromPool=true);
    void destroy();

    [[nodiscard]] VkDescriptorSet getDescriptorSet() const;
    [[nodiscard]] const DescriptorPool* getDescriptorPool() const;
    [[nodiscard]] bool isFreeFromPool() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

    void updateDescriptorSet(const Descriptor* descriptors, std::size_t descriptorSize);

private:
    std::pair<VkDescriptorSet, VkDescriptorPool> g_descriptorSet;

    const DescriptorPool* g_descriptorPool;
    bool g_freeFromPool;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORSET_HPP_INCLUDED
