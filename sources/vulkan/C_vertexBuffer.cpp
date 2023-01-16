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

namespace fge::vulkan
{

VertexBuffer::VertexBuffer() :
        g_vertexBuffer(VK_NULL_HANDLE),
        g_vertexStagingBuffer(VK_NULL_HANDLE),
        g_vertexBufferMemory(VK_NULL_HANDLE),
        g_vertexStagingBufferMemory(VK_NULL_HANDLE),
        g_vertexBufferCapacity(0),

        g_indexBuffer(VK_NULL_HANDLE),
        g_indexStagingBuffer(VK_NULL_HANDLE),
        g_indexBufferMemory(VK_NULL_HANDLE),
        g_indexStagingBufferMemory(VK_NULL_HANDLE),
        g_indexBufferCapacity(0),

        g_vertexNeedUpdate(true),
        g_indexNeedUpdate(true),

        g_type(Types::UNINITIALIZED),
        g_useIndexBuffer(true),

        g_primitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),

        g_context(nullptr)
{}
VertexBuffer::VertexBuffer(const VertexBuffer& r) :
        g_vertices(r.g_vertices),
        g_vertexBuffer(VK_NULL_HANDLE),
        g_vertexStagingBuffer(VK_NULL_HANDLE),
        g_vertexBufferMemory(VK_NULL_HANDLE),
        g_vertexStagingBufferMemory(VK_NULL_HANDLE),
        g_vertexBufferCapacity(0),

        g_indices(r.g_indices),
        g_indexBuffer(VK_NULL_HANDLE),
        g_indexStagingBuffer(VK_NULL_HANDLE),
        g_indexBufferMemory(VK_NULL_HANDLE),
        g_indexStagingBufferMemory(VK_NULL_HANDLE),
        g_indexBufferCapacity(0),

        g_vertexNeedUpdate(true),
        g_indexNeedUpdate(true),

        g_type(r.g_type),
        g_useIndexBuffer(r.g_useIndexBuffer),

        g_primitiveTopology(r.g_primitiveTopology),

        g_context(r.g_context)
{}
VertexBuffer::VertexBuffer(VertexBuffer&& r) noexcept :
        g_vertices(std::move(r.g_vertices)),
        g_vertexBuffer(r.g_vertexBuffer),
        g_vertexStagingBuffer(r.g_vertexStagingBuffer),
        g_vertexBufferMemory(r.g_vertexBufferMemory),
        g_vertexStagingBufferMemory(r.g_vertexStagingBufferMemory),
        g_vertexBufferCapacity(r.g_vertexBufferCapacity),

        g_indices(std::move(r.g_indices)),
        g_indexBuffer(r.g_indexBuffer),
        g_indexStagingBuffer(r.g_indexStagingBuffer),
        g_indexBufferMemory(r.g_indexBufferMemory),
        g_indexStagingBufferMemory(r.g_indexStagingBufferMemory),
        g_indexBufferCapacity(r.g_indexBufferCapacity),

        g_vertexNeedUpdate(r.g_vertexNeedUpdate),
        g_indexNeedUpdate(r.g_indexNeedUpdate),

        g_type(r.g_type),
        g_useIndexBuffer(r.g_useIndexBuffer),

        g_primitiveTopology(r.g_primitiveTopology),

        g_context(r.g_context)
{
    r.g_vertexBuffer = VK_NULL_HANDLE;
    r.g_vertexStagingBuffer = VK_NULL_HANDLE;
    r.g_vertexBufferMemory = VK_NULL_HANDLE;
    r.g_vertexStagingBufferMemory = VK_NULL_HANDLE;
    r.g_vertexBufferCapacity = 0;

    r.g_indexBuffer = VK_NULL_HANDLE;
    r.g_indexStagingBuffer = VK_NULL_HANDLE;
    r.g_indexBufferMemory = VK_NULL_HANDLE;
    r.g_indexStagingBufferMemory = VK_NULL_HANDLE;
    r.g_indexBufferCapacity = 0;

    r.g_vertexNeedUpdate = true;
    r.g_indexNeedUpdate = true;

    r.g_type = Types::UNINITIALIZED;
    r.g_useIndexBuffer = true;

    r.g_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    r.g_context = nullptr;
}
VertexBuffer::~VertexBuffer()
{
    this->destroy();
}

VertexBuffer& VertexBuffer::operator=(const VertexBuffer& r)
{
    this->destroy();

    this->g_vertices = r.g_vertices;
    this->g_vertexBuffer = VK_NULL_HANDLE;
    this->g_vertexStagingBuffer = VK_NULL_HANDLE;
    this->g_vertexBufferMemory = VK_NULL_HANDLE;
    this->g_vertexStagingBufferMemory = VK_NULL_HANDLE;
    this->g_vertexBufferCapacity = 0;

    this->g_indices = r.g_indices;
    this->g_indexBuffer = VK_NULL_HANDLE;
    this->g_indexStagingBuffer = VK_NULL_HANDLE;
    this->g_indexBufferMemory = VK_NULL_HANDLE;
    this->g_indexStagingBufferMemory = VK_NULL_HANDLE;
    this->g_indexBufferCapacity = 0;

    this->g_vertexNeedUpdate = true;
    this->g_indexNeedUpdate = true;

    this->g_type = r.g_type;
    this->g_useIndexBuffer = r.g_useIndexBuffer;
    this->g_primitiveTopology = r.g_primitiveTopology;

    this->g_context = r.g_context;

    return *this;
}
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& r) noexcept
{
    this->destroy();

    this->g_vertices = std::move(r.g_vertices);
    this->g_vertexBuffer = r.g_vertexBuffer;
    this->g_vertexStagingBuffer = r.g_vertexStagingBuffer;
    this->g_vertexBufferMemory = r.g_vertexBufferMemory;
    this->g_vertexStagingBufferMemory = r.g_vertexStagingBufferMemory;
    this->g_vertexBufferCapacity = r.g_vertexBufferCapacity;

    this->g_indices = std::move(r.g_indices);
    this->g_indexBuffer = r.g_indexBuffer;
    this->g_indexStagingBuffer = r.g_indexStagingBuffer;
    this->g_indexBufferMemory = r.g_indexBufferMemory;
    this->g_indexStagingBufferMemory = r.g_indexStagingBufferMemory;
    this->g_indexBufferCapacity = r.g_indexBufferCapacity;

    this->g_vertexNeedUpdate = r.g_vertexNeedUpdate;
    this->g_indexNeedUpdate = r.g_indexNeedUpdate;

    this->g_type = r.g_type;
    this->g_useIndexBuffer = r.g_useIndexBuffer;
    this->g_primitiveTopology = r.g_primitiveTopology;

    this->g_context = r.g_context;

    r.g_vertexBuffer = VK_NULL_HANDLE;
    r.g_vertexStagingBuffer = VK_NULL_HANDLE;
    r.g_vertexBufferMemory = VK_NULL_HANDLE;
    r.g_vertexStagingBufferMemory = VK_NULL_HANDLE;
    r.g_vertexBufferCapacity = 0;

    r.g_indexBuffer = VK_NULL_HANDLE;
    r.g_indexStagingBuffer = VK_NULL_HANDLE;
    r.g_indexBufferMemory = VK_NULL_HANDLE;
    r.g_indexStagingBufferMemory = VK_NULL_HANDLE;
    r.g_indexBufferCapacity = 0;

    r.g_vertexNeedUpdate = true;
    r.g_indexNeedUpdate = true;

    r.g_type = Types::UNINITIALIZED;
    r.g_useIndexBuffer = true;
    r.g_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    r.g_context = nullptr;

    return *this;
}

void VertexBuffer::create(const Context& context, std::size_t vertexSize, std::size_t indexSize, bool useIndexBuffer, VkPrimitiveTopology topology, Types type)
{
    this->g_primitiveTopology = topology;

    if (type == Types::UNINITIALIZED)
    {
        this->destroy();
        return;
    }

    if (type != this->g_type ||
        useIndexBuffer != this->g_useIndexBuffer)
    {
        this->destroy();
        this->g_useIndexBuffer = useIndexBuffer;
        this->g_type = type;
    }

    this->g_context = &context;

    this->resize(vertexSize, indexSize);
}
void VertexBuffer::create(const Context& context, std::size_t vertexSize, VkPrimitiveTopology topology, Types type)
{
    this->g_primitiveTopology = topology;

    if (type == Types::UNINITIALIZED)
    {
        this->destroy();
        return;
    }

    if (type != this->g_type ||
        this->g_useIndexBuffer)
    {
        this->destroy();
        this->g_useIndexBuffer = false;
        this->g_type = type;
    }

    this->g_context = &context;

    this->resize(vertexSize, 0);
}

void VertexBuffer::clear()
{
    this->resize(0, 0);
}
void VertexBuffer::resize(std::size_t vertexSize, std::size_t indexSize)
{
    if (this->g_type == Types::UNINITIALIZED)
    {
        return;
    }

    this->g_vertices.resize(vertexSize);
    if (this->g_useIndexBuffer)
    {
        this->g_indices.resize(indexSize);
    }

    this->g_vertexNeedUpdate = true;
    this->g_indexNeedUpdate = true;
}
void VertexBuffer::append(const Vertex& vertex)
{
    this->g_vertices.push_back(vertex);

    this->g_vertexNeedUpdate = true;
}
void VertexBuffer::appendIndex(uint16_t index)
{
    if (this->g_useIndexBuffer)
    {
        this->g_indices.push_back(index>=static_cast<uint16_t>(this->g_vertices.size()) ? static_cast<uint16_t>(this->g_vertices.size()-1) : index);

        this->g_indexNeedUpdate = true;
    }
}

void VertexBuffer::destroy()
{
    if (this->g_type != Types::UNINITIALIZED)
    {
        this->cleanVertexBuffers();
        this->cleanIndexBuffers();

        this->g_vertices.clear();
        this->g_indices.clear();

        this->g_vertexNeedUpdate = true;
        this->g_indexNeedUpdate = true;

        this->g_type = Types::UNINITIALIZED;
        this->g_useIndexBuffer = true;
        this->g_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        this->g_context = nullptr;
    }
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer) const
{
    this->updateBuffers();

    const VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->g_vertexBuffer, offsets);
    if (this->g_useIndexBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, this->g_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    }
}

std::size_t VertexBuffer::getVertexCount() const
{
    return this->g_vertices.size();
}
std::size_t VertexBuffer::getIndexCount() const
{
    return this->g_indices.size();
}

Vertex* VertexBuffer::getVertices()
{
    this->g_vertexNeedUpdate = true; ///TODO
    return this->g_vertices.empty() ? nullptr : this->g_vertices.data();
}
const Vertex* VertexBuffer::getVertices() const
{
    this->g_vertexNeedUpdate = true; ///TODO
    return this->g_vertices.empty() ? nullptr : this->g_vertices.data();
}
void VertexBuffer::mapVertices() const
{
    if (!this->g_vertexNeedUpdate)
    {
        return;
    }

    this->g_vertexNeedUpdate = false;

    const std::size_t size = sizeof(Vertex) * this->g_vertices.size();

    if (size == 0)
    {
        return;
    }

    void* data = nullptr;

    switch (this->g_type)
    {
    case Types::VERTEX_BUFFER:
        vkMapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_vertexBufferMemory, 0, size, 0, &data);
            memcpy(data, this->g_vertices.data(), size);
        vkUnmapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_vertexBufferMemory);
        break;
    case Types::STAGING_BUFFER:
        vkMapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_vertexStagingBufferMemory, 0, size, 0, &data);
            memcpy(data, this->g_vertices.data(), size);
        vkUnmapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_vertexStagingBufferMemory);

        this->g_context->copyBuffer(this->g_vertexStagingBuffer, this->g_vertexBuffer, size);
        break;
    default:
        throw std::runtime_error("uninitialized vertex buffer !");
    }
}

uint16_t* VertexBuffer::getIndices()
{
    this->g_indexNeedUpdate = true; ///TODO
    return this->g_indices.empty() ? nullptr : this->g_indices.data();
}
const uint16_t* VertexBuffer::getIndices() const
{
    this->g_indexNeedUpdate = true; ///TODO
    return this->g_indices.empty() ? nullptr : this->g_indices.data();
}

void VertexBuffer::setPrimitiveTopology(VkPrimitiveTopology topology)
{
    this->g_primitiveTopology = topology;
}
VkPrimitiveTopology VertexBuffer::getPrimitiveTopology() const
{
    return this->g_primitiveTopology;
}

void VertexBuffer::mapIndices() const
{
    if (!this->g_useIndexBuffer || !this->g_indexNeedUpdate)
    {
        return;
    }

    this->g_indexNeedUpdate = false;

    const std::size_t size = sizeof(uint16_t) * this->g_indices.size();

    if (size == 0)
    {
        return;
    }

    void* data = nullptr;

    switch (this->g_type)
    {
    case Types::VERTEX_BUFFER:
        vkMapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_indexBufferMemory, 0, size, 0, &data);
            memcpy(data, this->g_indices.data(), size);
        vkUnmapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_indexBufferMemory);
        break;
    case Types::STAGING_BUFFER:
        vkMapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_indexStagingBufferMemory, 0, size, 0, &data);
            memcpy(data, this->g_indices.data(), size);
        vkUnmapMemory(this->g_context->getLogicalDevice().getDevice(), this->g_indexStagingBufferMemory);

        this->g_context->copyBuffer(this->g_indexStagingBuffer, this->g_indexBuffer, size);
        break;
    default:
        throw std::runtime_error("uninitialized vertex buffer !");
    }
}

VkBuffer VertexBuffer::getVerticesBuffer() const
{
    return this->g_vertexBuffer;
}
VkBuffer VertexBuffer::getIndicesBuffer() const
{
    return this->g_indexBuffer;
}
VkDeviceMemory VertexBuffer::getVerticesBufferMemory() const
{
    return this->g_vertexBufferMemory;
}
VkDeviceMemory VertexBuffer::getIndicesBufferMemory() const
{
    return this->g_indexBufferMemory;
}
const Context* VertexBuffer::getContext() const
{
    return this->g_context;
}

VertexBuffer::Types VertexBuffer::getType() const
{
    return this->g_type;
}
bool VertexBuffer::isUsingIndexBuffer() const
{
    return this->g_useIndexBuffer;
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

void VertexBuffer::cleanVertexBuffers() const
{
    if (this->g_type != Types::UNINITIALIZED)
    {
        this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_vertexBuffer,
                                                                                             this->g_vertexBufferMemory,
                                                                                             this->g_context->getLogicalDevice().getDevice()));
        this->g_vertexBuffer = VK_NULL_HANDLE;
        this->g_vertexBufferMemory = VK_NULL_HANDLE;

        if (this->g_type == Types::STAGING_BUFFER)
        {
            this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_vertexStagingBuffer,
                                                                                                 this->g_vertexStagingBufferMemory,
                                                                                                 this->g_context->getLogicalDevice().getDevice()));

            this->g_vertexStagingBuffer = VK_NULL_HANDLE;
            this->g_vertexStagingBufferMemory = VK_NULL_HANDLE;
        }

        this->g_vertexBufferCapacity = 0;
    }
}
void VertexBuffer::cleanIndexBuffers() const
{
    if (this->g_type != Types::UNINITIALIZED && this->g_useIndexBuffer)
    {
        this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_indexBuffer,
                                                                                             this->g_indexBufferMemory,
                                                                                             this->g_context->getLogicalDevice().getDevice()));

        this->g_indexBuffer = VK_NULL_HANDLE;
        this->g_indexBufferMemory = VK_NULL_HANDLE;

        if (this->g_type == Types::STAGING_BUFFER)
        {
            this->g_context->_garbageCollector.push(fge::vulkan::GarbageCollector::GarbageBuffer(this->g_indexStagingBuffer,
                                                                                                 this->g_indexStagingBufferMemory,
                                                                                                 this->g_context->getLogicalDevice().getDevice()));

            this->g_indexStagingBuffer = VK_NULL_HANDLE;
            this->g_indexStagingBufferMemory = VK_NULL_HANDLE;
        }

        this->g_indexBufferCapacity = 0;
    }
}
void VertexBuffer::updateBuffers() const
{
    if (this->g_type == Types::UNINITIALIZED)
    {
        return;
    }

    if (this->g_vertices.size() > this->g_vertexBufferCapacity || this->g_vertexBuffer == VK_NULL_HANDLE)
    {
        this->cleanVertexBuffers();

        this->g_vertexBufferCapacity = this->g_vertices.size();

        const std::size_t vertexBufferSize = sizeof(Vertex) * (this->g_vertices.empty() ? 1 : this->g_vertices.size());

        switch (this->g_type)
        {
        case Types::VERTEX_BUFFER:
            CreateBuffer(this->g_context->getLogicalDevice(), this->g_context->getPhysicalDevice(),
                         vertexBufferSize,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         this->g_vertexBuffer, this->g_vertexBufferMemory);
            break;
        case Types::STAGING_BUFFER:
            CreateBuffer(this->g_context->getLogicalDevice(), this->g_context->getPhysicalDevice(),
                         vertexBufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         this->g_vertexStagingBuffer, this->g_vertexStagingBufferMemory);

            CreateBuffer(this->g_context->getLogicalDevice(), this->g_context->getPhysicalDevice(),
                         vertexBufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         this->g_vertexBuffer, this->g_vertexBufferMemory);
            break;
        default:
            throw std::runtime_error("unexpected code path !");
        }

        this->g_vertexNeedUpdate = true;
    }

    if ((this->g_indices.size() > this->g_indexBufferCapacity && this->g_useIndexBuffer) ||
        (this->g_indexBuffer == VK_NULL_HANDLE && this->g_useIndexBuffer))
    {
        this->cleanIndexBuffers();

        this->g_indexBufferCapacity = this->g_indices.size();

        const std::size_t indexBufferSize = sizeof(uint16_t) * (this->g_indices.empty() ? 1 : this->g_indices.size());

        switch (this->g_type)
        {
        case Types::VERTEX_BUFFER:
            CreateBuffer(this->g_context->getLogicalDevice(), this->g_context->getPhysicalDevice(),
                         indexBufferSize,
                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         this->g_indexBuffer, this->g_indexBufferMemory);
            break;
        case Types::STAGING_BUFFER:
            CreateBuffer(this->g_context->getLogicalDevice(), this->g_context->getPhysicalDevice(),
                         indexBufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         this->g_indexStagingBuffer, this->g_indexStagingBufferMemory);

            CreateBuffer(this->g_context->getLogicalDevice(), this->g_context->getPhysicalDevice(),
                         indexBufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         this->g_indexBuffer, this->g_indexBufferMemory);
            break;
        default:
            throw std::runtime_error("unexpected code path !");
        }

        this->g_indexNeedUpdate = true;
    }

    this->mapVertices();
    this->mapIndices();
}

}//end fge::vulkan