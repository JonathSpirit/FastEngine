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

#ifndef _FGE_VULKAN_C_VERTEXBUFFER_HPP_INCLUDED
#define _FGE_VULKAN_C_VERTEXBUFFER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_vertex.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <limits>
#include <vector>

#define FGE_VULKAN_VERTEX_DEFAULT_TOPOLOGY VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST

namespace fge::vulkan
{

class CommandBuffer;

enum class BufferTypes
{
    UNINITIALIZED,
    LOCAL,
    DEVICE,

    DEFAULT = LOCAL
};

class FGE_API VertexBuffer : public ContextAware
{
public:
    explicit VertexBuffer(Context const& context);
    VertexBuffer(VertexBuffer const& r);
    VertexBuffer(VertexBuffer&& r) noexcept;
    ~VertexBuffer() override;

    VertexBuffer& operator=(VertexBuffer const& r);
    VertexBuffer& operator=(VertexBuffer&& r) noexcept;

    void create(std::size_t vertexSize, VkPrimitiveTopology topology, BufferTypes type = BufferTypes::DEFAULT);

    void clear();
    void resize(std::size_t vertexSize);
    void append(Vertex const& vertex);

    void destroy() final;

    void bind(CommandBuffer& commandBuffer) const;

    [[nodiscard]] std::size_t getCount() const;

    [[nodiscard]] Vertex* getVertices();
    [[nodiscard]] Vertex const* getVertices() const;

    [[nodiscard]] Vertex& operator[](std::size_t index);
    [[nodiscard]] Vertex const& operator[](std::size_t index) const;

    void setPrimitiveTopology(VkPrimitiveTopology topology);
    [[nodiscard]] VkPrimitiveTopology getPrimitiveTopology() const;

    [[nodiscard]] VkBuffer getVerticesBuffer() const;
    [[nodiscard]] VmaAllocation getVerticesBufferAllocation() const;

    [[nodiscard]] BufferTypes getType() const;

    [[nodiscard]] fge::RectFloat getBounds() const; ///TODO take a IndexBuffer as optional parameter

private:
    void mapBuffer() const;
    void cleanBuffer() const;
    void updateBuffer() const;

    std::vector<Vertex> g_vertices;

    mutable BufferInfo g_bufferInfo;
    mutable BufferInfo g_stagingBufferInfo;
    mutable std::size_t g_bufferCapacity;

    mutable bool g_needUpdate;

    BufferTypes g_type;

    mutable VkPrimitiveTopology g_primitiveTopology;
};

class FGE_API IndexBuffer : public ContextAware
{
public:
    explicit IndexBuffer(Context const& context);
    IndexBuffer(IndexBuffer const& r);
    IndexBuffer(IndexBuffer&& r) noexcept;
    ~IndexBuffer() override;

    IndexBuffer& operator=(IndexBuffer const& r);
    IndexBuffer& operator=(IndexBuffer&& r) noexcept;

    void create(std::size_t indexSize, BufferTypes type = BufferTypes::DEFAULT);

    void clear();
    void resize(std::size_t indexSize);
    void append(uint16_t index = std::numeric_limits<uint16_t>::max());

    void destroy() final;

    void bind(CommandBuffer& commandBuffer) const;

    [[nodiscard]] std::size_t getCount() const;

    [[nodiscard]] uint16_t* getIndices();
    [[nodiscard]] uint16_t const* getIndices() const;

    [[nodiscard]] uint16_t& operator[](std::size_t index);
    [[nodiscard]] uint16_t const& operator[](std::size_t index) const;

    [[nodiscard]] VkBuffer getIndicesBuffer() const;
    [[nodiscard]] VmaAllocation getIndicesBufferAllocation() const;

    [[nodiscard]] BufferTypes getType() const;

private:
    void mapBuffer() const;
    void cleanBuffer() const;
    void updateBuffer() const;

    std::vector<uint16_t> g_indices;

    mutable BufferInfo g_bufferInfo;
    mutable BufferInfo g_stagingBufferInfo;
    mutable std::size_t g_bufferCapacity;

    mutable bool g_needUpdate;

    BufferTypes g_type;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_VERTEXBUFFER_HPP_INCLUDED
