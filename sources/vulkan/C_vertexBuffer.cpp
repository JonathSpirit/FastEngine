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
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <stdexcept>

namespace fge::vulkan
{

VertexBuffer::VertexBuffer() :
        g_vertexBuffer(VK_NULL_HANDLE),
        g_stagingBuffer(VK_NULL_HANDLE),
        g_vertexBufferMemory(VK_NULL_HANDLE),
        g_stagingBufferMemory(VK_NULL_HANDLE),

        g_indexBuffer(VK_NULL_HANDLE),
        g_indexBufferMemory(VK_NULL_HANDLE),

        g_type(Types::UNINITIALIZED),

        g_logicalDevice(nullptr)
{}
VertexBuffer::VertexBuffer(VertexBuffer&& r) noexcept :
        g_vertices(std::move(r.g_vertices)),

        g_vertexBuffer(r.g_vertexBuffer),
        g_stagingBuffer(r.g_stagingBuffer),
        g_vertexBufferMemory(r.g_vertexBufferMemory),
        g_stagingBufferMemory(r.g_stagingBufferMemory),

        g_indexBuffer(r.g_indexBuffer),
        g_indexBufferMemory(r.g_indexBufferMemory),

        g_type(r.g_type),

        g_logicalDevice(r.g_logicalDevice)
{
    r.g_vertexBuffer = VK_NULL_HANDLE;
    r.g_stagingBuffer = VK_NULL_HANDLE;
    r.g_vertexBufferMemory = VK_NULL_HANDLE;
    r.g_stagingBufferMemory = VK_NULL_HANDLE;

    r.g_indexBuffer = VK_NULL_HANDLE;
    r.g_indexBufferMemory = VK_NULL_HANDLE;

    r.g_type = Types::UNINITIALIZED;

    r.g_logicalDevice = nullptr;
}
VertexBuffer::~VertexBuffer()
{
    this->destroy();
}

void VertexBuffer::create(const Context& context, std::size_t vertexSize, std::size_t indexSize, Types type)
{
    this->destroy();
    if (type == Types::UNINITIALIZED)
    {
        return;
    }

    this->g_type = type;

    this->g_logicalDevice = &context.getLogicalDevice();

    this->g_vertices.resize(vertexSize);

    const std::size_t size = sizeof(Vertex) * vertexSize;

    switch (this->g_type)
    {
    case Types::VERTEX_BUFFER:
        CreateBuffer(context.getLogicalDevice(), context.getPhysicalDevice(),
                     size,
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     this->g_vertexBuffer, this->g_vertexBufferMemory);

        this->mapVertices(context);
        break;
    case Types::STAGING_BUFFER:
        CreateBuffer(context.getLogicalDevice(), context.getPhysicalDevice(),
                     size,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     this->g_stagingBuffer, this->g_stagingBufferMemory);

        CreateBuffer(context.getLogicalDevice(), context.getPhysicalDevice(),
                     size,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     this->g_vertexBuffer, this->g_vertexBufferMemory);

        this->mapVertices(context);
        break;
    default:
        throw std::runtime_error("unexpected code path !");
    }

    this->createIndexBuffer(context, indexSize);
}
void VertexBuffer::destroy()
{
    if (this->g_type != Types::UNINITIALIZED)
    {
        vkDestroyBuffer(this->g_logicalDevice->getDevice(), this->g_indexBuffer, nullptr);
        vkFreeMemory(this->g_logicalDevice->getDevice(), this->g_indexBufferMemory, nullptr);

        this->g_indexBuffer = VK_NULL_HANDLE;
        this->g_indexBufferMemory = VK_NULL_HANDLE;

        if (this->g_type == Types::STAGING_BUFFER)
        {
            vkDestroyBuffer(this->g_logicalDevice->getDevice(), this->g_stagingBuffer, nullptr);
            vkFreeMemory(this->g_logicalDevice->getDevice(), this->g_stagingBufferMemory, nullptr);

            this->g_stagingBuffer = VK_NULL_HANDLE;
            this->g_stagingBufferMemory = VK_NULL_HANDLE;
        }

        vkDestroyBuffer(this->g_logicalDevice->getDevice(), this->g_vertexBuffer, nullptr);
        vkFreeMemory(this->g_logicalDevice->getDevice(), this->g_vertexBufferMemory, nullptr);

        this->g_vertexBuffer = VK_NULL_HANDLE;
        this->g_vertexBufferMemory = VK_NULL_HANDLE;

        this->g_vertices.clear();
        this->g_indices.clear();

        this->g_type = Types::UNINITIALIZED;

        this->g_logicalDevice = nullptr;
    }
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer) const
{
    const VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->g_vertexBuffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, this->g_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
}

std::vector<Vertex>& VertexBuffer::getVertices()
{
    return this->g_vertices;
}
const std::vector<Vertex>& VertexBuffer::getVertices() const
{
    return this->g_vertices;
}
void VertexBuffer::mapVertices(const Context& context)
{
    const std::size_t size = sizeof(Vertex) * this->g_vertices.size();

    void* data = nullptr;

    switch (this->g_type)
    {
    case Types::VERTEX_BUFFER:
        vkMapMemory(this->g_logicalDevice->getDevice(), this->g_vertexBufferMemory, 0, size, 0, &data);
            memcpy(data, this->g_vertices.data(), size);
        vkUnmapMemory(this->g_logicalDevice->getDevice(), this->g_vertexBufferMemory);
        break;
    case Types::STAGING_BUFFER:
        vkMapMemory(this->g_logicalDevice->getDevice(), this->g_stagingBufferMemory, 0, size, 0, &data);
            memcpy(data, this->g_vertices.data(), size);
        vkUnmapMemory(this->g_logicalDevice->getDevice(), this->g_stagingBufferMemory);

        context.copyBuffer(this->g_stagingBuffer, this->g_vertexBuffer, size);
        break;
    default:
        throw std::runtime_error("uninitialized vertex buffer !");
    }
}

std::vector<uint16_t>& VertexBuffer::getIndices()
{
    return this->g_indices;
}
const std::vector<uint16_t>& VertexBuffer::getIndices() const
{
    return this->g_indices;
}
void VertexBuffer::mapIndices(const Context& context)
{
    const std::size_t size = sizeof(uint16_t) * this->g_indices.size();

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

    CreateBuffer(context.getLogicalDevice(), context.getPhysicalDevice(),
                 size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory, 0, size, 0, &data);
        memcpy(data, this->g_indices.data(), size);
    vkUnmapMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory);

    context.copyBuffer(stagingBuffer, this->g_indexBuffer, size);

    vkDestroyBuffer(context.getLogicalDevice().getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory, nullptr);
}

VkBuffer VertexBuffer::getBuffer() const
{
    return this->g_vertexBuffer;
}
VkDeviceMemory VertexBuffer::getBufferMemory() const
{
    return this->g_vertexBufferMemory;
}
const LogicalDevice* VertexBuffer::getLogicalDevice() const
{
    return this->g_logicalDevice;
}

VertexBuffer::Types VertexBuffer::getType() const
{
    return this->g_type;
}

void VertexBuffer::createIndexBuffer(const Context& context, std::size_t indicesSize)
{
    this->g_indices.resize(indicesSize);

    const std::size_t size = sizeof(uint16_t) * indicesSize;

    CreateBuffer(context.getLogicalDevice(), context.getPhysicalDevice(),
                 size,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 this->g_indexBuffer, this->g_indexBufferMemory);

    this->mapIndices(context);
}

}//end fge::vulkan