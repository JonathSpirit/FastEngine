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

#include "FastEngine/vulkan/C_vertexBuffer.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <cstring>

namespace fge::vulkan
{

VertexBuffer::VertexBuffer(Context const& context) :
        ContextAware(context),
        g_bufferCapacity(0),

        g_needUpdate(true),

        g_type(BufferTypes::UNINITIALIZED),

        g_primitiveTopology(FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY)
{}
VertexBuffer::VertexBuffer(VertexBuffer const& r) :
        ContextAware(r),
        g_vertices(r.g_vertices),

        g_bufferInfo({VK_NULL_HANDLE, VK_NULL_HANDLE}),
        g_stagingBufferInfo({VK_NULL_HANDLE, VK_NULL_HANDLE}),
        g_bufferCapacity(0),

        g_needUpdate(true),

        g_type(r.g_type),

        g_primitiveTopology(r.g_primitiveTopology)
{}
VertexBuffer::VertexBuffer(VertexBuffer&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_vertices(std::move(r.g_vertices)),

        g_bufferInfo(r.g_bufferInfo),
        g_stagingBufferInfo(r.g_stagingBufferInfo),
        g_bufferCapacity(r.g_bufferCapacity),

        g_needUpdate(r.g_needUpdate),

        g_type(r.g_type),

        g_primitiveTopology(r.g_primitiveTopology)
{
    r.g_bufferInfo.clear();
    r.g_stagingBufferInfo.clear();
    r.g_bufferCapacity = 0;

    r.g_needUpdate = true;

    r.g_type = BufferTypes::UNINITIALIZED;

    r.g_primitiveTopology = FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY;
}
VertexBuffer::~VertexBuffer()
{
    this->destroy();
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer const& r)
{
    this->verifyContext(r);

    if (this->g_type != r.g_type)
    {
        this->destroy();
    }

    this->g_vertices = r.g_vertices;

    this->g_needUpdate = true;

    this->g_type = r.g_type;

    this->g_primitiveTopology = r.g_primitiveTopology;

    return *this;
}
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& r) noexcept
{
    this->verifyContext(r);
    this->destroy();

    this->g_vertices = std::move(r.g_vertices);

    this->g_bufferInfo = r.g_bufferInfo;
    this->g_stagingBufferInfo = r.g_stagingBufferInfo;
    this->g_bufferCapacity = r.g_bufferCapacity;

    this->g_needUpdate = r.g_needUpdate;

    this->g_type = r.g_type;

    this->g_primitiveTopology = r.g_primitiveTopology;

    r.g_bufferInfo.clear();
    r.g_stagingBufferInfo.clear();
    r.g_bufferCapacity = 0;

    r.g_needUpdate = true;

    r.g_type = BufferTypes::UNINITIALIZED;

    r.g_primitiveTopology = FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY;

    return *this;
}

void VertexBuffer::create(std::size_t vertexSize, VkPrimitiveTopology topology, BufferTypes type)
{
    this->g_primitiveTopology = topology;

    if (type == BufferTypes::UNINITIALIZED)
    {
        this->destroy();
        return;
    }

    if (type != this->g_type)
    {
        this->destroy();
        this->g_type = type;
    }

    this->resize(vertexSize);
}

void VertexBuffer::clear()
{
    if (this->g_type == BufferTypes::UNINITIALIZED)
    {
        return;
    }

    if (!this->g_vertices.empty())
    {
        this->g_vertices.clear();
        this->g_needUpdate = true;
    }
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
void VertexBuffer::append(Vertex const& vertex)
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
    }
}

void VertexBuffer::bind(CommandBuffer& commandBuffer) const
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->updateBuffer();

        VkDeviceSize const offsets[] = {0};
        commandBuffer.bindVertexBuffers(0, 1, &this->g_bufferInfo._buffer, offsets);
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
Vertex const* VertexBuffer::getVertices() const
{
    this->g_needUpdate = true;
    return this->g_vertices.empty() ? nullptr : this->g_vertices.data();
}

Vertex& VertexBuffer::operator[](std::size_t index)
{
    this->g_needUpdate = true;
    return this->g_vertices[index];
}
Vertex const& VertexBuffer::operator[](std::size_t index) const
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
    return this->g_bufferInfo._buffer;
}
VmaAllocation VertexBuffer::getVerticesBufferAllocation() const
{
    return this->g_bufferInfo._allocation;
}

BufferTypes VertexBuffer::getType() const
{
    return this->g_type;
}

fge::RectFloat VertexBuffer::getBounds() const
{
    if (!this->g_vertices.empty())
    {
        float left = this->g_vertices[0]._position.x;
        float top = this->g_vertices[0]._position.y;
        float right = this->g_vertices[0]._position.x;
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
#ifndef FGE_DEF_SERVER
    if (!this->g_needUpdate)
    {
        return;
    }

    this->g_needUpdate = false;

    std::size_t const size = sizeof(Vertex) * this->g_vertices.size();

    if (size == 0)
    {
        return;
    }

    void* data = nullptr;

    switch (this->g_type)
    {
    case BufferTypes::LOCAL:
        vmaMapMemory(this->getContext().getAllocator(), this->g_bufferInfo._allocation, &data);
        memcpy(data, this->g_vertices.data(), size);
        vmaUnmapMemory(this->getContext().getAllocator(), this->g_bufferInfo._allocation);
        break;
    case BufferTypes::DEVICE:
        vmaMapMemory(this->getContext().getAllocator(), this->g_stagingBufferInfo._allocation, &data);
        memcpy(data, this->g_vertices.data(), size);
        vmaUnmapMemory(this->getContext().getAllocator(), this->g_stagingBufferInfo._allocation);

        {
            ///TODO: add submit type choice to the user
            auto buffer = this->getContext().beginCommands(Context::SubmitTypes::INDIRECT_EXECUTION,
                                                           CommandBuffer::RenderPassScopes::OUTSIDE,
                                                           CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);
            buffer.copyBuffer(this->g_stagingBufferInfo._buffer, this->g_bufferInfo._buffer, size);
            this->getContext().submitCommands(std::move(buffer));
        }
        break;
    default:
        return;
    }
#endif //FGE_DEF_SERVER
}
void VertexBuffer::cleanBuffer() const
{
#ifndef FGE_DEF_SERVER
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->getContext()._garbageCollector.push(GarbageBuffer(this->g_bufferInfo, this->getContext().getAllocator()));
        this->g_bufferInfo.clear();

        if (this->g_type == BufferTypes::DEVICE)
        {
            this->getContext()._garbageCollector.push(
                    GarbageBuffer(this->g_stagingBufferInfo, this->getContext().getAllocator()));
            this->g_stagingBufferInfo.clear();
        }

        this->g_bufferCapacity = 0;
    }
#endif //FGE_DEF_SERVER
}
void VertexBuffer::updateBuffer() const
{
#ifndef FGE_DEF_SERVER
    if (this->g_type == BufferTypes::UNINITIALIZED)
    {
        return;
    }

    if (this->g_vertices.size() > this->g_bufferCapacity || !this->g_bufferInfo.valid())
    {
        this->cleanBuffer();

        this->g_bufferCapacity = this->g_vertices.size() * 2;

        std::size_t const bufferSize = sizeof(Vertex) * (this->g_vertices.empty() ? 1 : this->g_vertices.size() * 2);

        switch (this->g_type)
        {
        case BufferTypes::LOCAL:
            this->g_bufferInfo = *this->getContext().createBuffer(
                    bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            break;
        case BufferTypes::DEVICE:
            this->g_stagingBufferInfo = *this->getContext().createBuffer(
                    bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            this->g_bufferInfo = *this->getContext().createBuffer(
                    bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            break;
        default:
            throw fge::Exception("unexpected code path !");
        }

        this->g_needUpdate = true;
    }

    this->mapBuffer();
#endif //FGE_DEF_SERVER
}

//IndexBuffer

IndexBuffer::IndexBuffer(Context const& context) :
        ContextAware(context),
        g_bufferCapacity(0),

        g_needUpdate(true),

        g_type(BufferTypes::UNINITIALIZED)
{}
IndexBuffer::IndexBuffer(IndexBuffer const& r) :
        ContextAware(r),
        g_indices(r.g_indices),

        g_bufferInfo({VK_NULL_HANDLE, VK_NULL_HANDLE}),
        g_stagingBufferInfo({VK_NULL_HANDLE, VK_NULL_HANDLE}),
        g_bufferCapacity(0),

        g_needUpdate(true),

        g_type(r.g_type)
{}
IndexBuffer::IndexBuffer(IndexBuffer&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_indices(std::move(r.g_indices)),

        g_bufferInfo(r.g_bufferInfo),
        g_stagingBufferInfo(r.g_stagingBufferInfo),
        g_bufferCapacity(r.g_bufferCapacity),

        g_needUpdate(r.g_needUpdate),

        g_type(r.g_type)
{
    r.g_bufferInfo.clear();
    r.g_stagingBufferInfo.clear();
    r.g_bufferCapacity = 0;

    r.g_needUpdate = true;

    r.g_type = BufferTypes::UNINITIALIZED;
}
IndexBuffer::~IndexBuffer()
{
    this->destroy();
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer const& r)
{
    this->verifyContext(r);

    if (this->g_type != r.g_type)
    {
        this->destroy();
    }

    this->g_indices = r.g_indices;

    this->g_needUpdate = true;

    this->g_type = r.g_type;

    return *this;
}
IndexBuffer& IndexBuffer::operator=(IndexBuffer&& r) noexcept
{
    this->verifyContext(r);
    this->destroy();

    this->g_indices = std::move(r.g_indices);

    this->g_bufferInfo = r.g_bufferInfo;
    this->g_stagingBufferInfo = r.g_stagingBufferInfo;
    this->g_bufferCapacity = r.g_bufferCapacity;

    this->g_needUpdate = r.g_needUpdate;

    this->g_type = r.g_type;

    r.g_bufferInfo.clear();
    r.g_stagingBufferInfo.clear();
    r.g_bufferCapacity = 0;

    r.g_needUpdate = true;

    r.g_type = BufferTypes::UNINITIALIZED;

    return *this;
}

void IndexBuffer::create(std::size_t indexSize, BufferTypes type)
{
    if (type == BufferTypes::UNINITIALIZED)
    {
        this->destroy();
        return;
    }

    if (type != this->g_type)
    {
        this->destroy();
        this->g_type = type;
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
    this->g_indices.push_back(index >= static_cast<uint16_t>(this->g_indices.size())
                                      ? static_cast<uint16_t>(this->g_indices.size() - 1)
                                      : index);
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
    }
}

void IndexBuffer::bind(CommandBuffer& commandBuffer) const
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->updateBuffer();
        commandBuffer.bindIndexBuffer(this->g_bufferInfo._buffer, 0, VK_INDEX_TYPE_UINT16);
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
uint16_t const* IndexBuffer::getIndices() const
{
    this->g_needUpdate = true;
    return this->g_indices.empty() ? nullptr : this->g_indices.data();
}

uint16_t& IndexBuffer::operator[](std::size_t index)
{
    this->g_needUpdate = true;
    return this->g_indices[index];
}
uint16_t const& IndexBuffer::operator[](std::size_t index) const
{
    this->g_needUpdate = true;
    return this->g_indices[index];
}

VkBuffer IndexBuffer::getIndicesBuffer() const
{
    return this->g_bufferInfo._buffer;
}
VmaAllocation IndexBuffer::getIndicesBufferAllocation() const
{
    return this->g_bufferInfo._allocation;
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

    std::size_t const size = sizeof(uint16_t) * this->g_indices.size();

    if (size == 0)
    {
        return;
    }

    void* data = nullptr;

    switch (this->g_type)
    {
    case BufferTypes::LOCAL:
        vmaMapMemory(this->getContext().getAllocator(), this->g_bufferInfo._allocation, &data);
        memcpy(data, this->g_indices.data(), size);
        vmaUnmapMemory(this->getContext().getAllocator(), this->g_bufferInfo._allocation);
        break;
    case BufferTypes::DEVICE:
        vmaMapMemory(this->getContext().getAllocator(), this->g_stagingBufferInfo._allocation, &data);
        memcpy(data, this->g_indices.data(), size);
        vmaUnmapMemory(this->getContext().getAllocator(), this->g_stagingBufferInfo._allocation);

        {
            ///TODO: add submit type choice to the user
            auto buffer = this->getContext().beginCommands(Context::SubmitTypes::INDIRECT_EXECUTION,
                                                           CommandBuffer::RenderPassScopes::OUTSIDE,
                                                           CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);
            buffer.copyBuffer(this->g_stagingBufferInfo._buffer, this->g_bufferInfo._buffer, size);
            this->getContext().submitCommands(std::move(buffer));
        }
        break;
    default:
        return;
    }
}
void IndexBuffer::cleanBuffer() const
{
    if (this->g_type != BufferTypes::UNINITIALIZED)
    {
        this->getContext()._garbageCollector.push(GarbageBuffer(this->g_bufferInfo, this->getContext().getAllocator()));
        this->g_bufferInfo.clear();

        if (this->g_type == BufferTypes::DEVICE)
        {
            this->getContext()._garbageCollector.push(
                    GarbageBuffer(this->g_stagingBufferInfo, this->getContext().getAllocator()));
            this->g_stagingBufferInfo.clear();
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

    if (this->g_indices.size() > this->g_bufferCapacity || !this->g_bufferInfo.valid())
    {
        this->cleanBuffer();

        this->g_bufferCapacity = this->g_indices.capacity();

        std::size_t const bufferSize = sizeof(uint16_t) * (this->g_indices.empty() ? 1 : this->g_indices.capacity());

        switch (this->g_type)
        {
        case BufferTypes::LOCAL:
            this->g_bufferInfo = *this->getContext().createBuffer(
                    bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            break;
        case BufferTypes::DEVICE:
            this->g_stagingBufferInfo = *this->getContext().createBuffer(
                    bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            this->g_bufferInfo = *this->getContext().createBuffer(
                    bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            break;
        default:
            throw fge::Exception("unexpected code path !");
        }

        this->g_needUpdate = true;
    }

    this->mapBuffer();
}

} // namespace fge::vulkan