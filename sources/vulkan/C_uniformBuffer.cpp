/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/vulkan/C_uniformBuffer.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <cstring>

namespace fge::vulkan
{

UniformBuffer::UniformBuffer(Context const& context) :
#ifndef FGE_DEF_SERVER
        ContextAware(context),
        g_uniformBuffer(VK_NULL_HANDLE),
        g_uniformBufferAllocation(VK_NULL_HANDLE),
        g_uniformBufferMapped(nullptr),
        g_bufferSize(0)
#else
        ContextAware(context)
#endif
{}
UniformBuffer::UniformBuffer([[maybe_unused]] const UniformBuffer& r) : ///TODO: better copy
        UniformBuffer(r.getContext())
{}
UniformBuffer::UniformBuffer(UniformBuffer&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
#ifndef FGE_DEF_SERVER
        g_uniformBuffer(r.g_uniformBuffer),
        g_uniformBufferAllocation(r.g_uniformBufferAllocation),
        g_uniformBufferMapped(r.g_uniformBufferMapped),
        g_bufferSize(r.g_bufferSize)
#else
        g_uniformBuffer(std::move(r.g_uniformBuffer))
#endif
{
#ifndef FGE_DEF_SERVER
    r.g_uniformBuffer = VK_NULL_HANDLE;
    r.g_uniformBufferAllocation = VK_NULL_HANDLE;
    r.g_uniformBufferMapped = nullptr;
    r.g_bufferSize = 0;
#endif
}
UniformBuffer::~UniformBuffer()
{
    this->destroy();
}

void UniformBuffer::create(VkDeviceSize bufferSize, [[maybe_unused]] bool isStorageBuffer)
{
    if (bufferSize == 0)
    {
        this->destroy();
        return;
    }

#ifdef FGE_DEF_SERVER
    this->destroy();
    this->g_uniformBuffer.resize(static_cast<std::size_t>(bufferSize));
#else
    this->destroy();

    CreateBuffer(this->getContext(), bufferSize,
                 isStorageBuffer ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->g_uniformBuffer,
                 this->g_uniformBufferAllocation);

    this->g_bufferSize = bufferSize;

    vmaMapMemory(this->getContext().getAllocator(), this->g_uniformBufferAllocation, &this->g_uniformBufferMapped);
#endif
}
void UniformBuffer::destroy()
{
#ifdef FGE_DEF_SERVER
    this->g_uniformBuffer.clear();
#else
    if (this->g_uniformBuffer != VK_NULL_HANDLE)
    {
        vmaUnmapMemory(this->getContext().getAllocator(), this->g_uniformBufferAllocation);
        this->getContext()._garbageCollector.push(fge::vulkan::GarbageBuffer(
                this->g_uniformBuffer, this->g_uniformBufferAllocation, this->getContext().getAllocator()));

        this->g_uniformBuffer = VK_NULL_HANDLE;
        this->g_uniformBufferAllocation = VK_NULL_HANDLE;
        this->g_uniformBufferMapped = nullptr;
        this->g_bufferSize = 0;
    }
#endif
}

#ifndef FGE_DEF_SERVER
VkBuffer UniformBuffer::getBuffer() const
{
    return this->g_uniformBuffer;
}
VmaAllocation UniformBuffer::getBufferAllocation() const
{
    return this->g_uniformBufferAllocation;
}
void* UniformBuffer::getBufferMapped() const
{
    return this->g_uniformBufferMapped;
}
VkDeviceSize UniformBuffer::getBufferSize() const
{
    return this->g_bufferSize;
}
#else
VkBuffer UniformBuffer::getBuffer() const
{
    return VK_NULL_HANDLE;
}
VmaAllocation UniformBuffer::getBufferAllocation() const
{
    return VK_NULL_HANDLE;
}
void* UniformBuffer::getBufferMapped() const
{
    return this->g_uniformBuffer.data();
}
VkDeviceSize UniformBuffer::getBufferSize() const
{
    return static_cast<VkDeviceSize>(this->g_uniformBuffer.size());
}
#endif

void UniformBuffer::copyData(const void* data, std::size_t size) const
{
    memcpy(this->getBufferMapped(), data, size);
}

} // namespace fge::vulkan