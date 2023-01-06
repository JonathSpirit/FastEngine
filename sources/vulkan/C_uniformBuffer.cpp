/*
 * Copyright 2022 Guillaume Guillet
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

UniformBuffer::UniformBuffer() :
        g_uniformBuffer(VK_NULL_HANDLE),
        g_uniformBufferMemory(VK_NULL_HANDLE),
        g_uniformBufferMapped(nullptr),
        g_bufferSize(0),

        g_context(nullptr)
{}
UniformBuffer::UniformBuffer([[maybe_unused]] const UniformBuffer& r) : ///TODO: better copy
        UniformBuffer()
{}
UniformBuffer::UniformBuffer(UniformBuffer&& r) noexcept :
        g_uniformBuffer(r.g_uniformBuffer),
        g_uniformBufferMemory(r.g_uniformBufferMemory),
        g_uniformBufferMapped(r.g_uniformBufferMapped),
        g_bufferSize(r.g_bufferSize),

        g_context(r.g_context)
{
    r.g_uniformBuffer = VK_NULL_HANDLE;
    r.g_uniformBufferMemory = VK_NULL_HANDLE;
    r.g_uniformBufferMapped = nullptr;
    r.g_bufferSize = 0;

    r.g_context = nullptr;
}
UniformBuffer::~UniformBuffer()
{
    this->destroy();
}

void UniformBuffer::create(const Context& context, VkDeviceSize bufferSize)
{
    CreateBuffer(context.getLogicalDevice(), context.getPhysicalDevice(),
                 bufferSize,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 this->g_uniformBuffer, this->g_uniformBufferMemory);

    this->g_bufferSize = bufferSize;
    this->g_context = &context;

    vkMapMemory(context.getLogicalDevice().getDevice(), this->g_uniformBufferMemory, 0, bufferSize, 0, &this->g_uniformBufferMapped);
}
void UniformBuffer::destroy()
{
    if (this->g_uniformBuffer != VK_NULL_HANDLE)
    {
        vkUnmapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_uniformBufferMemory);
        this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_uniformBuffer,
                                                                                             this->g_uniformBufferMemory,
                                                                                             this->g_context->getLogicalDevice().getDevice()));

        this->g_uniformBuffer = VK_NULL_HANDLE;
        this->g_uniformBufferMemory = VK_NULL_HANDLE;
        this->g_uniformBufferMapped = nullptr;
        this->g_bufferSize = 0;

        this->g_context = nullptr;
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
VkDeviceSize UniformBuffer::getBufferSize() const
{
    return this->g_bufferSize;
}

const Context* UniformBuffer::getContext() const
{
    return this->g_context;
}

void UniformBuffer::copyData(const void* data, std::size_t size) const
{
    memcpy(this->g_uniformBufferMapped, data, size);
}

}//end fge::vulkan