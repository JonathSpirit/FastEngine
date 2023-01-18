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
#include "volk.h"
#include <limits>
#include <vector>

namespace fge::vulkan
{

class LogicalDevice;
class PhysicalDevice;
class Context;

class FGE_API VertexBuffer
{
public:
    enum class Types
    {
        UNINITIALIZED,
        VERTEX_BUFFER,
        STAGING_BUFFER,

        DEFAULT=STAGING_BUFFER
    };

    VertexBuffer();
    VertexBuffer(const VertexBuffer& r);
    VertexBuffer(VertexBuffer&& r) noexcept;
    ~VertexBuffer();

    VertexBuffer& operator=(const VertexBuffer& r);
    VertexBuffer& operator=(VertexBuffer&& r) noexcept;

    void create(const Context& context, std::size_t vertexSize, std::size_t indexSize, bool useIndexBuffer, VkPrimitiveTopology topology, Types type=Types::DEFAULT);
    void create(const Context& context, std::size_t vertexSize, VkPrimitiveTopology topology, Types type=Types::DEFAULT);

    void clear();
    void resize(std::size_t vertexSize, std::size_t indexSize=0);
    void append(const Vertex& vertex);
    void appendIndex(uint16_t index=std::numeric_limits<uint16_t>::max());

    void destroy();

    void bind(VkCommandBuffer commandBuffer) const;

    [[nodiscard]] std::size_t getVertexCount() const;
    [[nodiscard]] std::size_t getIndexCount() const;

    [[nodiscard]] Vertex* getVertices();
    [[nodiscard]] const Vertex* getVertices() const;

    [[nodiscard]] uint16_t* getIndices();
    [[nodiscard]] const uint16_t* getIndices() const;

    void setPrimitiveTopology(VkPrimitiveTopology topology);
    [[nodiscard]] VkPrimitiveTopology getPrimitiveTopology() const;

    [[nodiscard]] VkBuffer getVerticesBuffer() const;
    [[nodiscard]] VkBuffer getIndicesBuffer() const;
    [[nodiscard]] VkDeviceMemory getVerticesBufferMemory() const;
    [[nodiscard]] VkDeviceMemory getIndicesBufferMemory() const;
    [[nodiscard]] const Context* getContext() const;

    [[nodiscard]] Types getType() const;
    [[nodiscard]] bool isUsingIndexBuffer() const;

    [[nodiscard]] fge::RectFloat getBounds() const;

private:
    void mapVertices() const;
    void mapIndices() const;

    void cleanVertexBuffers() const;
    void cleanIndexBuffers() const;
    void updateBuffers() const;

    std::vector<Vertex> g_vertices;
    mutable VkBuffer g_vertexBuffer;
    mutable VkBuffer g_vertexStagingBuffer;
    mutable VkDeviceMemory g_vertexBufferMemory;
    mutable VkDeviceMemory g_vertexStagingBufferMemory;
    mutable std::size_t g_vertexBufferCapacity;

    std::vector<uint16_t> g_indices;
    mutable VkBuffer g_indexBuffer;
    mutable VkBuffer g_indexStagingBuffer;
    mutable VkDeviceMemory g_indexBufferMemory;
    mutable VkDeviceMemory g_indexStagingBufferMemory;
    mutable std::size_t g_indexBufferCapacity;

    mutable bool g_vertexNeedUpdate;
    mutable bool g_indexNeedUpdate;

    Types g_type;
    bool g_useIndexBuffer;

    mutable VkPrimitiveTopology g_primitiveTopology;

    const Context* g_context;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_VERTEXBUFFER_HPP_INCLUDED
