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

#include "FastEngine/vulkan/C_vertexBuffer.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <stdexcept>
#include <cstring>

namespace fge::vulkan
{

VertexBuffer::VertexBuffer() :
        g_buffer(VK_NULL_HANDLE),
        g_stagingBuffer(VK_NULL_HANDLE),
        g_bufferAllocation(VK_NULL_HANDLE),
        g_stagingBufferAllocation(VK_NULL_HANDLE),
        g_bufferCapacity(0),

        g_needUpdate(true),

        g_type(BufferTypes::UNINITIALIZED),

        g_primitiveTopology(FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY),

        g_context(nullptr)
{}
VertexBuffer::VertexBuffer(const VertexBuffer& r) :
        g_vertices(r.g_vertices),

        g_buffer(VK_NULL_HANDLE),
        g_stagingBuffer(VK_NULL_HANDLE),
        g_bufferAllocation(VK_NULL_HANDLE),
        g_stagingBufferAllocation(VK_NULL_HANDLE),
        g_bufferCapacity(0),

        g_needUpdate(true),

        g_type(r.g_type),

        g_primitiveTopology(r.g_primitiveTopology),

        g_context(r.g_context)
{}
VertexBuffer::VertexBuffer(VertexBuffer&& r) noexcept :
        g_vertices(std::move(r.g_vertices)),

        g_buffer(r.g_buffer),
        g_stagingBuffer(r.g_stagingBuffer),
        g_bufferAllocation(r.g_bufferAllocation),
        g_stagingBufferAllocation(r.g_stagingBufferAllocation),
        g_bufferCapacity(r.g_bufferCapacity),

        g_needUpdate(r.g_needUpdate),

        g_type(r.g_type),

        g_primitiveTopology(r.g_primitiveTopology),

        g_context(r.g_context)
{
    r.g_buffer = VK_NULL_HANDLE;
    r.g_stagingBuffer = VK_NULL_HANDLE;
    r.g_bufferAllocation = VK_NULL_HANDLE;
    r.g_stagingBufferAllocation = VK_NULL_HANDLE;
    r.g_bufferCapacity = 0;

    r.g_needUpdate = true;

    r.g_type = BufferTypes::UNINITIALIZED;

    r.g_primitiveTopology = FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY;

    r.g_context = nullptr;
}
VertexBuffer::~VertexBuffer()
{
    this->destroy();
}

VertexBuffer& VertexBuffer::operator=(const VertexBuffer& r)
{
    if (this->g_type != r.g_type ||
        this->g_context != r.g_context)
    {
        this->destroy();
    }

    this->g_vertices = r.g_vertices;
    
    this->g_needUpdate = true;

    this->g_type = r.g_type;
    
    this->g_primitiveTopology = r.g_primitiveTopology;

    this->g_context = r.g_context;

    return *this;
}
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& r) noexcept
{
    this->destroy();

    this->g_vertices = std::move(r.g_vertices);
    
    this->g_buffer = r.g_buffer;
    this->g_stagingBuffer = r.g_stagingBuffer;
    this->g_bufferAllocation = r.g_bufferAllocation;
    this->g_stagingBufferAllocation = r.g_stagingBufferAllocation;
    this->g_bufferCapacity = r.g_bufferCapacity;

    this->g_needUpdate = r.g_needUpdate;

    this->g_type = r.g_type;
    
    this->g_primitiveTopology = r.g_primitiveTopology;

    this->g_context = r.g_context;

    r.g_buffer = VK_NULL_HANDLE;
    r.g_stagingBuffer = VK_NULL_HANDLE;
    r.g_bufferAllocation = VK_NULL_HANDLE;
    r.g_stagingBufferAllocation = VK_NULL_HANDLE;
    r.g_bufferCapacity = 0;

    r.g_needUpdate = true;

    r.g_type = BufferTypes::UNINITIALIZED;
    
    r.g_primitiveTopology = FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY;

    r.g_context = nullptr;

    return *this;
}

void VertexBuffer::create(const Context& context, std::size_t vertexSize, VkPrimitiveTopology topology, BufferTypes type)
{
    this->g_primitiveTopology = topology;

    if (type == BufferTypes::UNINITIALIZED)
    {
        this->destroy();
        return;
    }

    if (type != this->g_type ||
        (this->g_context != nullptr && &context != this->g_context))
    {
        this->destroy();
        this->g_type = type;
        this->g_context = &context;
    }

    this->resize(vertexSize);
}

void VertexBuffer::clear()
{
    this->resize(0);
}
void VertexBuffer::resize(std::size_t vertexSize)
{
    if (this->g_type == BufferTypes::UNINITIALIZED)
    {
        return;
    }
    
    if (this->g_vertices.size() != vertexSize)
    {
        this->g_vertices.resize(vertexSize);
        this->g_needUpdate = true;
    }
}
void VertexBuffer::append(const Vertex& vertex)
{
    this->g_vertices.push_back(vertex);
    this->g_needUpdate = true;
}

void VertexBuffer::destroy()
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->cleanBuffer();

        this->g_vertices.clear();

        this->g_needUpdate = true;

        this->g_type = BufferTypes::UNINITIALIZED;
        
        this->g_primitiveTopology = FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY;

        this->g_context = nullptr;
    }
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer) const
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->updateBuffer();

        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->g_buffer, offsets);
    }
}

std::size_t VertexBuffer::getCount() const
{
    return this->g_vertices.size();
}

Vertex* VertexBuffer::getVertices()
{
    this->g_needUpdate = true;
    return this->g_vertices.empty() ? nullptr : this->g_vertices.data();
}
const Vertex* VertexBuffer::getVertices() const
{
    this->g_needUpdate = true;
    return this->g_vertices.empty() ? nullptr : this->g_vertices.data();
}

Vertex& VertexBuffer::operator[](std::size_t index)
{
    this->g_needUpdate = true;
    return this->g_vertices[index];
}
const Vertex& VertexBuffer::operator[](std::size_t index) const
{
    this->g_needUpdate = true;
    return this->g_vertices[index];
}

void VertexBuffer::setPrimitiveTopology(VkPrimitiveTopology topology)
{
    this->g_primitiveTopology = topology;
}
VkPrimitiveTopology VertexBuffer::getPrimitiveTopology() const
{
    return this->g_primitiveTopology;
}

VkBuffer VertexBuffer::getVerticesBuffer() const
{
    return this->g_buffer;
}
VmaAllocation VertexBuffer::getVerticesBufferAllocation() const
{
    return this->g_bufferAllocation;
}
const Context* VertexBuffer::getContext() const
{
    return this->g_context;
}

BufferTypes VertexBuffer::getType() const
{
    return this->g_type;
}

fge::RectFloat VertexBuffer::getBounds() const
{
    if (!this->g_vertices.empty())
    {
        float left   = this->g_vertices[0]._position.x;
        float top    = this->g_vertices[0]._position.y;
        float right  = this->g_vertices[0]._position.x;
        float bottom = this->g_vertices[0]._position.y;

        for (std::size_t i = 1; i < this->g_vertices.size(); ++i)
        {
            auto position = this->g_vertices[i]._position;

            // Update left and right
            if (position.x < left)
            {
                left = position.x;
            }
            else if (position.x > right)
            {
                right = position.x;
            }

            // Update top and bottom
            if (position.y < top)
            {
                top = position.y;
            }
            else if (position.y > bottom)
            {
                bottom = position.y;
            }
        }

        return {{left, top}, {right - left, bottom - top}};
    }
    // Array is empty
    return {};
}

void VertexBuffer::mapBuffer() const
{
    if (!this->g_needUpdate)
    {
        return;
    }

    this->g_needUpdate = false;

    const std::size_t size = sizeof(Vertex) * this->g_vertices.size();

    if (size == 0)
    {
        return;
    }

    void* data = nullptr;

    switch (this->g_type)
    {
    case BufferTypes::LOCAL:
        vmaMapMemory(this->g_context->getAllocator(), this->g_bufferAllocation, &data);
        memcpy(data, this->g_vertices.data(), size);
        vmaUnmapMemory(this->g_context->getAllocator(), this->g_bufferAllocation);
        break;
    case BufferTypes::DEVICE:
        vmaMapMemory(this->g_context->getAllocator(), this->g_stagingBufferAllocation, &data);
        memcpy(data, this->g_vertices.data(), size);
        vmaUnmapMemory(this->g_context->getAllocator(), this->g_stagingBufferAllocation);

        this->g_context->copyBuffer(this->g_stagingBuffer, this->g_buffer, size);
        break;
    default:
        return;
    }
}
void VertexBuffer::cleanBuffer() const
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_buffer,
                                                                                             this->g_bufferAllocation,
                                                                                             this->g_context->getAllocator()));
        this->g_buffer = VK_NULL_HANDLE;
        this->g_bufferAllocation = VK_NULL_HANDLE;

        if (this->g_type == BufferTypes::DEVICE)
        {
            this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_stagingBuffer,
                                                                                                 this->g_stagingBufferAllocation,
                                                                                                 this->g_context->getAllocator()));

            this->g_stagingBuffer = VK_NULL_HANDLE;
            this->g_stagingBufferAllocation = VK_NULL_HANDLE;
        }

        this->g_bufferCapacity = 0;
    }
}
void VertexBuffer::updateBuffer() const
{
    if (this->g_type == BufferTypes::UNINITIALIZED)
    {
        return;
    }

    if (this->g_vertices.size() > this->g_bufferCapacity ||
        this->g_buffer == VK_NULL_HANDLE)
    {
        this->cleanBuffer();

        this->g_bufferCapacity = this->g_vertices.capacity();

        const std::size_t bufferSize = sizeof(Vertex) * (this->g_vertices.empty() ? 1 : this->g_vertices.capacity());

        switch (this->g_type)
        {
        case BufferTypes::LOCAL:
            CreateBuffer(*this->g_context,
                         bufferSize,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         this->g_buffer, this->g_bufferAllocation);
            break;
        case BufferTypes::DEVICE:
            CreateBuffer(*this->g_context,
                         bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         this->g_stagingBuffer, this->g_stagingBufferAllocation);

            CreateBuffer(*this->g_context,
                         bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         this->g_buffer, this->g_bufferAllocation);
            break;
        default:
            throw std::runtime_error("unexpected code path !");
        }

        this->g_needUpdate = true;
    }

    this->mapBuffer();
}

//IndexBuffer

IndexBuffer::IndexBuffer() :
        g_buffer(VK_NULL_HANDLE),
        g_stagingBuffer(VK_NULL_HANDLE),
        g_bufferAllocation(VK_NULL_HANDLE),
        g_stagingBufferAllocation(VK_NULL_HANDLE),
        g_bufferCapacity(0),

        g_needUpdate(true),

        g_type(BufferTypes::UNINITIALIZED),

        g_context(nullptr)
{}
IndexBuffer::IndexBuffer(const IndexBuffer& r) :
        g_indices(r.g_indices),

        g_buffer(VK_NULL_HANDLE),
        g_stagingBuffer(VK_NULL_HANDLE),
        g_bufferAllocation(VK_NULL_HANDLE),
        g_stagingBufferAllocation(VK_NULL_HANDLE),
        g_bufferCapacity(0),

        g_needUpdate(true),

        g_type(r.g_type),

        g_context(r.g_context)
{}
IndexBuffer::IndexBuffer(IndexBuffer&& r) noexcept :
        g_indices(std::move(r.g_indices)),

        g_buffer(r.g_buffer),
        g_stagingBuffer(r.g_stagingBuffer),
        g_bufferAllocation(r.g_bufferAllocation),
        g_stagingBufferAllocation(r.g_stagingBufferAllocation),
        g_bufferCapacity(r.g_bufferCapacity),

        g_needUpdate(r.g_needUpdate),

        g_type(r.g_type),

        g_context(r.g_context)
{
    r.g_buffer = VK_NULL_HANDLE;
    r.g_stagingBuffer = VK_NULL_HANDLE;
    r.g_bufferAllocation = VK_NULL_HANDLE;
    r.g_stagingBufferAllocation = VK_NULL_HANDLE;
    r.g_bufferCapacity = 0;

    r.g_needUpdate = true;

    r.g_type = BufferTypes::UNINITIALIZED;

    r.g_context = nullptr;
}
IndexBuffer::~IndexBuffer()
{
    this->destroy();
}

IndexBuffer& IndexBuffer::operator=(const IndexBuffer& r)
{
    if (this->g_type != r.g_type ||
        this->g_context != r.g_context)
    {
        this->destroy();
    }

    this->g_indices = r.g_indices;

    this->g_needUpdate = true;

    this->g_type = r.g_type;

    this->g_context = r.g_context;

    return *this;
}
IndexBuffer& IndexBuffer::operator=(IndexBuffer&& r) noexcept
{
    this->destroy();

    this->g_indices = std::move(r.g_indices);

    this->g_buffer = r.g_buffer;
    this->g_stagingBuffer = r.g_stagingBuffer;
    this->g_bufferAllocation = r.g_bufferAllocation;
    this->g_stagingBufferAllocation = r.g_stagingBufferAllocation;
    this->g_bufferCapacity = r.g_bufferCapacity;

    this->g_needUpdate = r.g_needUpdate;

    this->g_type = r.g_type;

    this->g_context = r.g_context;

    r.g_buffer = VK_NULL_HANDLE;
    r.g_stagingBuffer = VK_NULL_HANDLE;
    r.g_bufferAllocation = VK_NULL_HANDLE;
    r.g_stagingBufferAllocation = VK_NULL_HANDLE;
    r.g_bufferCapacity = 0;

    r.g_needUpdate = true;

    r.g_type = BufferTypes::UNINITIALIZED;

    r.g_context = nullptr;

    return *this;
}

void IndexBuffer::create(const Context& context, std::size_t indexSize, BufferTypes type)
{
    if (type == BufferTypes::UNINITIALIZED)
    {
        this->destroy();
        return;
    }

    if (type != this->g_type ||
        (this->g_context != nullptr && &context != this->g_context))
    {
        this->destroy();
        this->g_type = type;
        this->g_context = &context;
    }

    this->resize(indexSize);
}

void IndexBuffer::clear()
{
    this->resize(0);
}
void IndexBuffer::resize(std::size_t indexSize)
{
    if (this->g_type == BufferTypes::UNINITIALIZED)
    {
        return;
    }

    if (this->g_indices.size() != indexSize)
    {
        this->g_indices.resize(indexSize);
        this->g_needUpdate = true;
    }
}
void IndexBuffer::append(uint16_t index)
{
    this->g_indices.push_back(index>=static_cast<uint16_t>(this->g_indices.size()) ?
                             static_cast<uint16_t>(this->g_indices.size()-1) : index);
    this->g_needUpdate = true;
}

void IndexBuffer::destroy()
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->cleanBuffer();

        this->g_indices.clear();

        this->g_needUpdate = true;

        this->g_type = BufferTypes::UNINITIALIZED;

        this->g_context = nullptr;
    }
}

void IndexBuffer::bind(VkCommandBuffer commandBuffer) const
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->updateBuffer();
        vkCmdBindIndexBuffer(commandBuffer, this->g_buffer, 0, VK_INDEX_TYPE_UINT16);
    }
}

std::size_t IndexBuffer::getCount() const
{
    return this->g_indices.size();
}

uint16_t* IndexBuffer::getIndices()
{
    this->g_needUpdate = true;
    return this->g_indices.empty() ? nullptr : this->g_indices.data();
}
const uint16_t* IndexBuffer::getIndices() const
{
    this->g_needUpdate = true;
    return this->g_indices.empty() ? nullptr : this->g_indices.data();
}

uint16_t& IndexBuffer::operator[](std::size_t index)
{
    this->g_needUpdate = true;
    return this->g_indices[index];
}
const uint16_t& IndexBuffer::operator[](std::size_t index) const
{
    this->g_needUpdate = true;
    return this->g_indices[index];
}

VkBuffer IndexBuffer::getIndicesBuffer() const
{
    return this->g_buffer;
}
VmaAllocation IndexBuffer::getIndicesBufferAllocation() const
{
    return this->g_bufferAllocation;
}
const Context* IndexBuffer::getContext() const
{
    return this->g_context;
}

BufferTypes IndexBuffer::getType() const
{
    return this->g_type;
}

void IndexBuffer::mapBuffer() const
{
    if (!this->g_needUpdate)
    {
        return;
    }

    this->g_needUpdate = false;

    const std::size_t size = sizeof(uint16_t) * this->g_indices.size();

    if (size == 0)
    {
        return;
    }

    void* data = nullptr;

    switch (this->g_type)
    {
    case BufferTypes::LOCAL:
        vmaMapMemory(this->g_context->getAllocator(), this->g_bufferAllocation, &data);
        memcpy(data, this->g_indices.data(), size);
        vmaUnmapMemory(this->g_context->getAllocator(), this->g_bufferAllocation);
        break;
    case BufferTypes::DEVICE:
        vmaMapMemory(this->g_context->getAllocator(), this->g_stagingBufferAllocation, &data);
        memcpy(data, this->g_indices.data(), size);
        vmaUnmapMemory(this->g_context->getAllocator(), this->g_stagingBufferAllocation);

        this->g_context->copyBuffer(this->g_stagingBuffer, this->g_buffer, size);
        break;
    default:
        return;
    }
}
void IndexBuffer::cleanBuffer() const
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_buffer,
                                                                                             this->g_bufferAllocation,
                                                                                             this->g_context->getAllocator()));
        this->g_buffer = VK_NULL_HANDLE;
        this->g_bufferAllocation = VK_NULL_HANDLE;

        if (this->g_type == BufferTypes::DEVICE)
        {
            this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_stagingBuffer,
                                                                                                 this->g_stagingBufferAllocation,
                                                                                                 this->g_context->getAllocator()));

            this->g_stagingBuffer = VK_NULL_HANDLE;
            this->g_stagingBufferAllocation = VK_NULL_HANDLE;
        }

        this->g_bufferCapacity = 0;
    }
}
void IndexBuffer::updateBuffer() const
{
    if (this->g_type == BufferTypes::UNINITIALIZED)
    {
        return;
    }

    if (this->g_indices.size() > this->g_bufferCapacity ||
        this->g_buffer == VK_NULL_HANDLE)
    {
        this->cleanBuffer();

        this->g_bufferCapacity = this->g_indices.capacity();

        const std::size_t bufferSize = sizeof(uint16_t) * (this->g_indices.empty() ? 1 : this->g_indices.capacity());

        switch (this->g_type)
        {
        case BufferTypes::LOCAL:
            CreateBuffer(*this->g_context,
                         bufferSize,
                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         this->g_buffer, this->g_bufferAllocation);
            break;
        case BufferTypes::DEVICE:
            CreateBuffer(*this->g_context,
                         bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         this->g_stagingBuffer, this->g_stagingBufferAllocation);

            CreateBuffer(*this->g_context,
                         bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         this->g_buffer, this->g_bufferAllocation);
            break;
        default:
            throw std::runtime_error("unexpected code path !");
        }

        this->g_needUpdate = true;
    }

    this->mapBuffer();
}

}//end fge::vulkan