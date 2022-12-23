#ifndef _FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED
#define _FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED

#include "SDL_vulkan.h"
#include "volk.h"
#include <cstdint>

namespace fge::vulkan
{

class LogicalDevice;
class PhysicalDevice;

class UniformBuffer
{
public:
    UniformBuffer();
    UniformBuffer(const UniformBuffer& r);
    UniformBuffer(UniformBuffer&& r) noexcept;
    ~UniformBuffer();

    UniformBuffer& operator=(const UniformBuffer& r) = delete;
    UniformBuffer& operator=(UniformBuffer&& r) noexcept = delete;

    void create(const LogicalDevice& logicalDevice, const PhysicalDevice& physicalDevice, VkDeviceSize bufferSize);
    void destroy();

    [[nodiscard]] VkBuffer getBuffer() const;
    [[nodiscard]] VkDeviceMemory getBufferMemory() const;
    [[nodiscard]] void* getBufferMapped() const;
    [[nodiscard]] VkDeviceSize getBufferSize() const;

    void copyData(const void* data, std::size_t size) const;

private:
    VkBuffer g_uniformBuffer;
    VkDeviceMemory g_uniformBufferMemory;
    void* g_uniformBufferMapped;
    VkDeviceSize g_bufferSize;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED
