#ifndef _FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED

#include "SDL_vulkan.h"
#include "volk.h"
#include <vector>
#include <utility>

namespace fge::vulkan
{

class LogicalDevice;

class DescriptorPool
{
public:
    struct Pool
    {
        VkDescriptorPool _pool;
        uint32_t _count;
    };

    DescriptorPool();
    DescriptorPool(const DescriptorPool& r) = delete;
    DescriptorPool(DescriptorPool&& r) noexcept;
    ~DescriptorPool();

    DescriptorPool& operator=(const DescriptorPool& r) = delete;
    DescriptorPool& operator=(DescriptorPool&& r) noexcept = delete;

    void create(const LogicalDevice& logicalDevice,
                std::vector<VkDescriptorPoolSize>&& descriptorPoolSizes,
                uint32_t maxSetsPerPool, bool isUnique);
    void destroy();

    [[nodiscard]] std::pair<VkDescriptorSet, VkDescriptorPool> allocateDescriptorSets(const VkDescriptorSetLayout* setLayouts, uint32_t descriptorSetCount) const;
    void freeDescriptorSets(VkDescriptorSet descriptorSet, VkDescriptorPool descriptorPool, bool dontFreeButDecrementCount) const;
    void resetDescriptorPool(VkDescriptorPool descriptorPool) const;
    void resetDescriptorPool() const;

    [[nodiscard]] uint32_t getMaxSetsPerPool() const;
    [[nodiscard]] bool isUnique() const;
    [[nodiscard]] bool isCreated() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

private:
    [[nodiscard]] Pool createPool() const;

    std::vector<VkDescriptorPoolSize> g_descriptorPoolSizes;

    uint32_t g_maxSetsPerPool;
    mutable std::vector<Pool> g_descriptorPools;
    bool g_isUnique;
    bool g_isCreated;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORPOOL_HPP_INCLUDED
