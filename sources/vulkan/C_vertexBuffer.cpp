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

        g_indexBuffer(VK_NULL_HANDLE),
        g_indexStagingBuffer(VK_NULL_HANDLE),
        g_indexBufferMemory(VK_NULL_HANDLE),
        g_indexStagingBufferMemory(VK_NULL_HANDLE),

        g_type(Types::UNINITIALIZED),
        g_useIndexBuffer(true),

        g_context(nullptr)
{}
VertexBuffer::VertexBuffer(const VertexBuffer& r) :
        g_vertices(r.g_vertices),
        g_vertexBuffer(VK_NULL_HANDLE),
        g_vertexStagingBuffer(VK_NULL_HANDLE),
        g_vertexBufferMemory(VK_NULL_HANDLE),
        g_vertexStagingBufferMemory(VK_NULL_HANDLE),

        g_indices(r.g_indices),
        g_indexBuffer(VK_NULL_HANDLE),
        g_indexStagingBuffer(VK_NULL_HANDLE),
        g_indexBufferMemory(VK_NULL_HANDLE),
        g_indexStagingBufferMemory(VK_NULL_HANDLE),

        g_type(r.g_type),
        g_useIndexBuffer(r.g_useIndexBuffer),

        g_context(r.g_context)
{
    this->updateBuffers();
}
VertexBuffer::VertexBuffer(VertexBuffer&& r) noexcept :
        g_vertices(std::move(r.g_vertices)),
        g_vertexBuffer(r.g_vertexBuffer),
        g_vertexStagingBuffer(r.g_vertexStagingBuffer),
        g_vertexBufferMemory(r.g_vertexBufferMemory),
        g_vertexStagingBufferMemory(r.g_vertexStagingBufferMemory),

        g_indices(std::move(r.g_indices)),
        g_indexBuffer(r.g_indexBuffer),
        g_indexStagingBuffer(r.g_indexStagingBuffer),
        g_indexBufferMemory(r.g_indexBufferMemory),
        g_indexStagingBufferMemory(r.g_indexStagingBufferMemory),

        g_type(r.g_type),
        g_useIndexBuffer(r.g_useIndexBuffer),

        g_context(r.g_context)
{
    r.g_vertexBuffer = VK_NULL_HANDLE;
    r.g_vertexStagingBuffer = VK_NULL_HANDLE;
    r.g_vertexBufferMemory = VK_NULL_HANDLE;
    r.g_vertexStagingBufferMemory = VK_NULL_HANDLE;

    r.g_indexBuffer = VK_NULL_HANDLE;
    r.g_indexStagingBuffer = VK_NULL_HANDLE;
    r.g_indexBufferMemory = VK_NULL_HANDLE;
    r.g_indexStagingBufferMemory = VK_NULL_HANDLE;

    r.g_type = Types::UNINITIALIZED;
    r.g_useIndexBuffer = true;

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

    this->g_indices = r.g_indices;
    this->g_indexBuffer = VK_NULL_HANDLE;
    this->g_indexStagingBuffer = VK_NULL_HANDLE;
    this->g_indexBufferMemory = VK_NULL_HANDLE;
    this->g_indexStagingBufferMemory = VK_NULL_HANDLE;

    this->g_type = r.g_type;
    this->g_useIndexBuffer = r.g_useIndexBuffer;

    this->g_context = r.g_context;

    return *this;
}
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& r) noexcept
{
    this->destroy();

    this->g_vertices = std::move(r.g_vertices);
    this->g_vertexBuffer = VK_NULL_HANDLE;
    this->g_vertexStagingBuffer = VK_NULL_HANDLE;
    this->g_vertexBufferMemory = VK_NULL_HANDLE;
    this->g_vertexStagingBufferMemory = VK_NULL_HANDLE;

    this->g_indices = std::move(r.g_indices);
    this->g_indexBuffer = VK_NULL_HANDLE;
    this->g_indexStagingBuffer = VK_NULL_HANDLE;
    this->g_indexBufferMemory = VK_NULL_HANDLE;
    this->g_indexStagingBufferMemory = VK_NULL_HANDLE;

    this->g_type = r.g_type;
    this->g_useIndexBuffer = r.g_useIndexBuffer;

    this->g_context = r.g_context;

    r.g_vertexBuffer = VK_NULL_HANDLE;
    r.g_vertexStagingBuffer = VK_NULL_HANDLE;
    r.g_vertexBufferMemory = VK_NULL_HANDLE;
    r.g_vertexStagingBufferMemory = VK_NULL_HANDLE;

    r.g_indexBuffer = VK_NULL_HANDLE;
    r.g_indexStagingBuffer = VK_NULL_HANDLE;
    r.g_indexBufferMemory = VK_NULL_HANDLE;
    r.g_indexStagingBufferMemory = VK_NULL_HANDLE;

    r.g_type = Types::UNINITIALIZED;
    r.g_useIndexBuffer = true;

    r.g_context = nullptr;

    return *this;
}

void VertexBuffer::create(const Context& context, std::size_t vertexSize, std::size_t indexSize, bool useIndexBuffer, Types type)
{
    this->destroy();
    if (type == Types::UNINITIALIZED)
    {
        return;
    }

    this->g_useIndexBuffer = useIndexBuffer;
    this->g_type = type;

    this->g_context = &context;

    this->resize(vertexSize, indexSize);
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

    this->updateBuffers();
}
void VertexBuffer::append(const Vertex& vertex)
{
    this->g_vertices.push_back(vertex);
    this->updateBuffers();
}
void VertexBuffer::appendIndex(uint16_t index)
{
    if (this->g_useIndexBuffer)
    {
        this->g_indices.push_back(index>=static_cast<uint16_t>(this->g_vertices.size()) ? static_cast<uint16_t>(this->g_vertices.size()-1) : index);

        this->updateBuffers();
    }
}

void VertexBuffer::destroy()
{
    if (this->g_type != Types::UNINITIALIZED)
    {
        this->cleanBuffers();

        this->g_vertices.clear();
        this->g_indices.clear();

        this->g_type = Types::UNINITIALIZED;
        this->g_useIndexBuffer = true;

        this->g_context = nullptr;
    }
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer) const
{
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
    return this->g_vertices.empty() ? nullptr : this->g_vertices.data();
}
const Vertex* VertexBuffer::getVertices() const
{
    return this->g_vertices.empty() ? nullptr : this->g_vertices.data();
}
void VertexBuffer::mapVertices()
{
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
    return this->g_indices.empty() ? nullptr : this->g_indices.data();
}
const uint16_t* VertexBuffer::getIndices() const
{
    return this->g_indices.empty() ? nullptr : this->g_indices.data();
}
void VertexBuffer::mapIndices()
{
    if (!this->g_useIndexBuffer)
    {
        return;
    }

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

void VertexBuffer::cleanBuffers()
{
    if (this->g_type != Types::UNINITIALIZED)
    {
        if (this->g_useIndexBuffer)
        {
            vkDestroyBuffer(this->g_context->getLogicalDevice().getDevice(), this->g_indexBuffer, nullptr);
            vkFreeMemory(this->g_context->getLogicalDevice().getDevice(), this->g_indexBufferMemory, nullptr);

            this->g_indexBuffer = VK_NULL_HANDLE;
            this->g_indexBufferMemory = VK_NULL_HANDLE;

            if (this->g_type == Types::STAGING_BUFFER)
            {
                vkDestroyBuffer(this->g_context->getLogicalDevice().getDevice(), this->g_indexStagingBuffer, nullptr);
                vkFreeMemory(this->g_context->getLogicalDevice().getDevice(), this->g_indexStagingBufferMemory, nullptr);

                this->g_indexStagingBuffer = VK_NULL_HANDLE;
                this->g_indexStagingBufferMemory = VK_NULL_HANDLE;
            }
        }

        vkDestroyBuffer(this->g_context->getLogicalDevice().getDevice(), this->g_vertexBuffer, nullptr);
        vkFreeMemory(this->g_context->getLogicalDevice().getDevice(), this->g_vertexBufferMemory, nullptr);

        this->g_vertexBuffer = VK_NULL_HANDLE;
        this->g_vertexBufferMemory = VK_NULL_HANDLE;

        if (this->g_type == Types::STAGING_BUFFER)
        {
            vkDestroyBuffer(this->g_context->getLogicalDevice().getDevice(), this->g_vertexStagingBuffer, nullptr);
            vkFreeMemory(this->g_context->getLogicalDevice().getDevice(), this->g_vertexStagingBufferMemory, nullptr);

            this->g_vertexStagingBuffer = VK_NULL_HANDLE;
            this->g_vertexStagingBufferMemory = VK_NULL_HANDLE;
        }
    }
}
void VertexBuffer::updateBuffers()
{
    if (this->g_type == Types::UNINITIALIZED)
    {
        return;
    }

    this->cleanBuffers();

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

    //Creating indices buffer
    if (!this->g_useIndexBuffer)
    {
        return;
    }

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

    this->mapVertices();
    this->mapIndices();
}

}//end fge::vulkan