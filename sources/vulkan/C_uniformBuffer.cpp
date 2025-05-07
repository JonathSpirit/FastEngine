/*
 * Copyright 2025 Guillaume Guillet
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

UniformBuffer::UniformBuffer(Context const& context, Types type) :
#ifndef FGE_DEF_SERVER
        ContextAware(context),
        g_uniformBufferMapped(nullptr),
        g_bufferSize(0),
        g_bufferCapacity(0),
        g_type(type)
#else
        ContextAware(context),
        g_type(type)
#endif
{}
UniformBuffer::UniformBuffer(UniformBuffer const& r) :
        UniformBuffer(r.getContext())
{
    this->create(r.getBufferSize(), r.getType());
    this->copyData(r.getBufferMapped(), static_cast<std::size_t>(r.getBufferSize()));
}
UniformBuffer::UniformBuffer(UniformBuffer&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
#ifndef FGE_DEF_SERVER
        g_uniformBufferInfo(r.g_uniformBufferInfo),
        g_uniformBufferMapped(r.g_uniformBufferMapped),
        g_bufferSize(r.g_bufferSize),
        g_bufferCapacity(r.g_bufferCapacity),
        g_type(r.g_type)
#else
        g_uniformBuffer(std::move(r.g_uniformBuffer)),
        g_type(r.g_type)
#endif
{
#ifndef FGE_DEF_SERVER
    r.g_uniformBufferInfo.clear();
    r.g_uniformBufferMapped = nullptr;
    r.g_bufferSize = 0;
    r.g_bufferCapacity = 0;
#endif
    r.g_type = Types::UNIFORM_BUFFER;
}
UniformBuffer::~UniformBuffer()
{
    this->destroy();
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer const& r)
{
    this->verifyContext(r);
    this->create(r.getBufferSize(), r.getType());
    this->copyData(r.getBufferMapped(), static_cast<std::size_t>(r.getBufferSize()));
    return *this;
}
UniformBuffer& UniformBuffer::operator=(UniformBuffer&& r) noexcept
{
    this->verifyContext(r);
    this->destroy();
#ifndef FGE_DEF_SERVER
    this->g_uniformBufferInfo = r.g_uniformBufferInfo;
    this->g_uniformBufferMapped = r.g_uniformBufferMapped;
    this->g_bufferSize = r.g_bufferSize;
    this->g_bufferCapacity = r.g_bufferCapacity;

    r.g_uniformBufferInfo.clear();
    r.g_uniformBufferMapped = nullptr;
    r.g_bufferSize = 0;
    r.g_bufferCapacity = 0;
#else
    this->g_uniformBuffer = std::move(r.g_uniformBuffer);
#endif
    this->g_type = r.g_type;
    r.g_type = Types::UNIFORM_BUFFER;
    return *this;
}

void UniformBuffer::create(VkDeviceSize bufferSize, Types type)
{
    this->g_type = type;

#ifdef FGE_DEF_SERVER
    if (static_cast<std::size_t>(bufferSize) == 0)
    {
        this->destroy();
        return;
    }

    this->destroy();
    this->g_uniformBuffer.resize(static_cast<std::size_t>(bufferSize));
#else
    if (bufferSize == 0)
    {
        this->destroy();
        return;
    }

    this->destroy();

    this->g_uniformBufferInfo = this->createBuffer(bufferSize);
    this->g_bufferSize = bufferSize;
    this->g_bufferCapacity = bufferSize;

    vmaMapMemory(this->getContext().getAllocator(), this->g_uniformBufferInfo._allocation,
                 &this->g_uniformBufferMapped);
#endif
}
void UniformBuffer::resize(VkDeviceSize bufferSize, bool shrink)
{
#ifdef FGE_DEF_SERVER
    this->g_uniformBuffer.resize(static_cast<std::size_t>(bufferSize));
    if (shrink)
    {
        this->shrinkToFit();
    }
#else
    if (bufferSize == 0)
    {
        this->g_bufferSize = 0;
        if (shrink)
        {
            this->shrinkToFit();
        }
        return;
    }

    if (bufferSize <= this->g_bufferCapacity)
    {
        this->g_bufferSize = bufferSize;
        if (shrink)
        {
            this->shrinkToFit();
        }
        return;
    }

    if (this->g_bufferCapacity == 0)
    {
        this->create(bufferSize, this->g_type);
    }
    else
    {
        auto const newBufferInfo = this->createBuffer(bufferSize);

        void* newBufferMapped{nullptr};
        vmaMapMemory(this->getContext().getAllocator(), newBufferInfo._allocation, &newBufferMapped);
        std::memcpy(newBufferMapped, this->g_uniformBufferMapped, static_cast<std::size_t>(this->g_bufferSize));

        vmaUnmapMemory(this->getContext().getAllocator(), this->g_uniformBufferInfo._allocation);
        this->getContext()._garbageCollector.push(
                GarbageBuffer(this->g_uniformBufferInfo, this->getContext().getAllocator()));

        this->g_uniformBufferInfo = newBufferInfo;
        this->g_uniformBufferMapped = newBufferMapped;
        this->g_bufferSize = bufferSize;
        this->g_bufferCapacity = bufferSize;
    }
#endif
}
void UniformBuffer::shrinkToFit()
{
#ifdef FGE_DEF_SERVER
    this->g_uniformBuffer.shrink_to_fit();
#else
    if (this->g_bufferSize == this->g_bufferCapacity)
    {
        return;
    }
    if (this->g_bufferSize == 0)
    {
        this->destroy();
        return;
    }

    auto const newBufferInfo = this->createBuffer(this->g_bufferSize);

    void* newBufferMapped{nullptr};
    vmaMapMemory(this->getContext().getAllocator(), newBufferInfo._allocation, &newBufferMapped);
    std::memcpy(newBufferMapped, this->g_uniformBufferMapped, static_cast<std::size_t>(this->g_bufferSize));

    vmaUnmapMemory(this->getContext().getAllocator(), this->g_uniformBufferInfo._allocation);
    this->g_uniformBufferMapped = newBufferMapped;

    this->getContext()._garbageCollector.push(
            GarbageBuffer(this->g_uniformBufferInfo, this->getContext().getAllocator()));

    this->g_uniformBufferInfo = newBufferInfo;
    this->g_bufferCapacity = this->g_bufferSize;
#endif
}

void UniformBuffer::destroy()
{
#ifdef FGE_DEF_SERVER
    this->g_uniformBuffer.clear();
    this->g_type = Types::UNIFORM_BUFFER;
#else
    if (this->g_uniformBufferInfo.valid())
    {
        vmaUnmapMemory(this->getContext().getAllocator(), this->g_uniformBufferInfo._allocation);
        this->getContext()._garbageCollector.push(
                GarbageBuffer(this->g_uniformBufferInfo, this->getContext().getAllocator()));

        this->g_uniformBufferInfo.clear();
        this->g_uniformBufferMapped = nullptr;
        this->g_bufferSize = 0;
        this->g_bufferCapacity = 0;
        this->g_type = Types::UNIFORM_BUFFER;
    }
#endif
}

#ifndef FGE_DEF_SERVER
VkBuffer UniformBuffer::getBuffer() const
{
    return this->g_uniformBufferInfo._buffer;
}
VmaAllocation UniformBuffer::getBufferAllocation() const
{
    return this->g_uniformBufferInfo._allocation;
}
void* UniformBuffer::getBufferMapped() const
{
    return this->g_uniformBufferMapped;
}
VkDeviceSize UniformBuffer::getBufferSize() const
{
    return this->g_bufferSize;
}
VkDeviceSize UniformBuffer::getBufferCapacity() const
{
    return this->g_bufferCapacity;
}

BufferInfo UniformBuffer::createBuffer(VkDeviceSize bufferSize) const
{
    VkBufferUsageFlags usageFlags{0};
    switch (this->g_type)
    {
    case Types::UNIFORM_BUFFER:
        usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case Types::STORAGE_BUFFER:
        usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    case Types::INDIRECT_BUFFER:
        usageFlags = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        break;
    }

    return *this->getContext().createBuffer(
            bufferSize, usageFlags, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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
VkDeviceSize UniformBuffer::getBufferCapacity() const
{
    return static_cast<VkDeviceSize>(this->g_uniformBuffer.capacity());
}
#endif

UniformBuffer::Types UniformBuffer::getType() const
{
    return this->g_type;
}

void UniformBuffer::copyData(void const* data, std::size_t size) const
{
    if (data != nullptr && size > 0 && size <= static_cast<std::size_t>(this->getBufferSize()))
    {
        std::memcpy(this->getBufferMapped(), data, size);
    }
}

} // namespace fge::vulkan