/*
 * Copyright 2026 Guillaume Guillet
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
#include "FastEngine/graphic/C_color.hpp"
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

/**
 * \class VertexBuffer
 * \ingroup vulkan
 * \brief A vertex buffer abstraction
 *
 * This class is used to create a vertex buffer and manage its data local only or on the device.
 */
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

    /**
     * \warning Only works with triangle topology, otherwise the result is undefined
     */
    Vertex* appendTriangle(fge::Vector2f const& positionA,
                           fge::Vector2f const& texCoordA,
                           fge::Vector2f const& positionB,
                           fge::Vector2f const& texCoordB,
                           fge::Vector2f const& positionC,
                           fge::Vector2f const& texCoordC,
                           fge::Color const& color);
    /**
     * \warning Only works with triangle topology, otherwise the result is undefined
     */
    Vertex* appendTriangle(fge::Vector2f const& positionA,
                           fge::Vector2f const& positionB,
                           fge::Vector2f const& positionC,
                           fge::Color const& color);

    /**
     * \warning Only works with triangle topology, otherwise the result is undefined
     */
    Vertex* appendRect(fge::Vector2f const& position,
                       fge::Vector2f const& size,
                       fge::Vector2f const& texCoordUpLeft,
                       fge::Vector2f const& texCoordUpRight,
                       fge::Vector2f const& texCoordDownRight,
                       fge::Vector2f const& texCoordDownLeft,
                       fge::Color color);
    /**
     * \warning Only works with triangle topology, otherwise the result is undefined
     */
    Vertex* appendRect(fge::Vector2f const& position, fge::Vector2f const& size, fge::Color color);

    /**
     * \warning Only works with triangle topology, otherwise the result is undefined
     */
    Vertex* appendCircle(fge::Vector2f const& center, float radius, fge::Color color, unsigned int segmentCount = 16);

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

/**
 * \class IndexBuffer
 * \ingroup vulkan
 * \brief An index buffer abstraction
 *
 * This class is used to create an index buffer and manage its data local only or on the device.
 */
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
