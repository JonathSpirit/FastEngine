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

#ifndef _FGE_VULKAN_C_VERTEXBUFFER_HPP_INCLUDED
#define _FGE_VULKAN_C_VERTEXBUFFER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "C_vertex.hpp"
#include "FastEngine/C_rect.hpp"
#include "SDL_vulkan.h"
#include "vk_mem_alloc.h"
#include "volk.h"
#include <limits>
#include <vector>

#define FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST

namespace fge::vulkan
{

class LogicalDevice;
class PhysicalDevice;
class Context;

enum class BufferTypes
{
    UNINITIALIZED,
    LOCAL,
    DEVICE,

    DEFAULT = LOCAL
};

class FGE_API VertexBuffer
{
public:
    VertexBuffer();
    VertexBuffer(const VertexBuffer& r);
    VertexBuffer(VertexBuffer&& r) noexcept;
    ~VertexBuffer();

    VertexBuffer& operator=(const VertexBuffer& r);
    VertexBuffer& operator=(VertexBuffer&& r) noexcept;

    void create(const Context& context,
                std::size_t vertexSize,
                VkPrimitiveTopology topology,
                BufferTypes type = BufferTypes::DEFAULT);

    void clear();
    void resize(std::size_t vertexSize);
    void append(const Vertex& vertex);

    void destroy();

    void bind(VkCommandBuffer commandBuffer) const;

    [[nodiscard]] std::size_t getCount() const;

    [[nodiscard]] Vertex* getVertices();
    [[nodiscard]] const Vertex* getVertices() const;

    [[nodiscard]] Vertex& operator[](std::size_t index);
    [[nodiscard]] const Vertex& operator[](std::size_t index) const;

    void setPrimitiveTopology(VkPrimitiveTopology topology);
    [[nodiscard]] VkPrimitiveTopology getPrimitiveTopology() const;

    [[nodiscard]] VkBuffer getVerticesBuffer() const;
    [[nodiscard]] VmaAllocation getVerticesBufferAllocation() const;
    [[nodiscard]] const Context* getContext() const;

    [[nodiscard]] BufferTypes getType() const;

    [[nodiscard]] fge::RectFloat getBounds() const; ///TODO take a IndexBuffer as optional parameter

private:
    void mapBuffer() const;
    void cleanBuffer() const;
    void updateBuffer() const;

    std::vector<Vertex> g_vertices;

    mutable VkBuffer g_buffer;
    mutable VkBuffer g_stagingBuffer;
    mutable VmaAllocation g_bufferAllocation;
    mutable VmaAllocation g_stagingBufferAllocation;
    mutable std::size_t g_bufferCapacity;

    mutable bool g_needUpdate;

    BufferTypes g_type;

    mutable VkPrimitiveTopology g_primitiveTopology;

    const Context* g_context;
};

class FGE_API IndexBuffer
{
public:
    IndexBuffer();
    IndexBuffer(const IndexBuffer& r);
    IndexBuffer(IndexBuffer&& r) noexcept;
    ~IndexBuffer();

    IndexBuffer& operator=(const IndexBuffer& r);
    IndexBuffer& operator=(IndexBuffer&& r) noexcept;

    void create(const Context& context, std::size_t indexSize, BufferTypes type = BufferTypes::DEFAULT);

    void clear();
    void resize(std::size_t indexSize);
    void append(uint16_t index = std::numeric_limits<uint16_t>::max());

    void destroy();

    void bind(VkCommandBuffer commandBuffer) const;

    [[nodiscard]] std::size_t getCount() const;

    [[nodiscard]] uint16_t* getIndices();
    [[nodiscard]] const uint16_t* getIndices() const;

    [[nodiscard]] uint16_t& operator[](std::size_t index);
    [[nodiscard]] const uint16_t& operator[](std::size_t index) const;

    [[nodiscard]] VkBuffer getIndicesBuffer() const;
    [[nodiscard]] VmaAllocation getIndicesBufferAllocation() const;
    [[nodiscard]] const Context* getContext() const;

    [[nodiscard]] BufferTypes getType() const;

private:
    void mapBuffer() const;
    void cleanBuffer() const;
    void updateBuffer() const;

    std::vector<uint16_t> g_indices;

    mutable VkBuffer g_buffer;
    mutable VkBuffer g_stagingBuffer;
    mutable VmaAllocation g_bufferAllocation;
    mutable VmaAllocation g_stagingBufferAllocation;
    mutable std::size_t g_bufferCapacity;

    mutable bool g_needUpdate;

    BufferTypes g_type;

    const Context* g_context;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_VERTEXBUFFER_HPP_INCLUDED
