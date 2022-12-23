#include "FastEngine/vulkan/C_uniformBuffer.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <cstring>

namespace fge::vulkan
{

UniformBuffer::UniformBuffer() :
        g_uniformBuffer(VK_NULL_HANDLE),
        g_uniformBufferMemory(VK_NULL_HANDLE),
        g_uniformBufferMapped(nullptr),
        g_bufferSize(0),

        g_logicalDevice(nullptr)
{}
UniformBuffer::UniformBuffer([[maybe_unused]] const UniformBuffer& r) : ///TODO: better copy
        UniformBuffer()
{}
UniformBuffer::UniformBuffer(UniformBuffer&& r) noexcept :
        g_uniformBuffer(r.g_uniformBuffer),
        g_uniformBufferMemory(r.g_uniformBufferMemory),
        g_uniformBufferMapped(r.g_uniformBufferMapped),
        g_bufferSize(r.g_bufferSize),

        g_logicalDevice(r.g_logicalDevice)
{
    r.g_uniformBuffer = VK_NULL_HANDLE;
    r.g_uniformBufferMemory = VK_NULL_HANDLE;
    r.g_uniformBufferMapped = nullptr;
    r.g_bufferSize = 0;

    r.g_logicalDevice = nullptr;
}
UniformBuffer::~UniformBuffer()
{
    this->destroy();
}

void UniformBuffer::create(const LogicalDevice& logicalDevice, const PhysicalDevice& physicalDevice, VkDeviceSize bufferSize)
{
    CreateBuffer(logicalDevice, physicalDevice,
                 bufferSize,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 this->g_uniformBuffer, this->g_uniformBufferMemory);

    this->g_bufferSize = bufferSize;
    this->g_logicalDevice = &logicalDevice;

    vkMapMemory(logicalDevice.getDevice(), this->g_uniformBufferMemory, 0, bufferSize, 0, &this->g_uniformBufferMapped);
}
void UniformBuffer::destroy()
{
    if (this->g_uniformBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(this->g_logicalDevice->getDevice(), this->g_uniformBuffer, nullptr);
        vkFreeMemory(this->g_logicalDevice->getDevice(), this->g_uniformBufferMemory, nullptr);

        this->g_uniformBuffer = VK_NULL_HANDLE;
        this->g_uniformBufferMemory = VK_NULL_HANDLE;
        this->g_uniformBufferMapped = nullptr;
        this->g_bufferSize = 0;

        this->g_logicalDevice = nullptr;
    }
}

VkBuffer UniformBuffer::getBuffer() const
{
    return this->g_uniformBuffer;
}
VkDeviceMemory UniformBuffer::getBufferMemory() const
{
    return this->g_uniformBufferMemory;
}
void* UniformBuffer::getBufferMapped() const
{
    return this->g_uniformBufferMapped;
}
std::size_t UniformBuffer::getBufferSize() const
{
    return this->g_bufferSize;
}

void UniformBuffer::copyData(const void* data, std::size_t size) const
{
    memcpy(this->g_uniformBufferMapped, data, size);
}

}//end fge::vulkan