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

#include "SDL_vulkan.h"
#include <vector>
#include "volk.h"
#include "C_vertex.hpp"

namespace fge::vulkan
{

class LogicalDevice;
class PhysicalDevice;
class Context;

class VertexBuffer
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
    VertexBuffer(const VertexBuffer& r) = delete;
    VertexBuffer(VertexBuffer&& r) noexcept;
    ~VertexBuffer();

    VertexBuffer& operator=(const VertexBuffer& r) = delete;
    VertexBuffer& operator=(VertexBuffer&& r) noexcept = delete;

    void create(const Context& context, std::size_t vertexSize, std::size_t indexSize, Types type=Types::DEFAULT);
    void destroy();

    void bind(VkCommandBuffer commandBuffer) const;

    [[nodiscard]] std::vector<Vertex>& getVertices();
    [[nodiscard]] const std::vector<Vertex>& getVertices() const;
    void mapVertices(const Context& context);

    [[nodiscard]] std::vector<uint16_t>& getIndices();
    [[nodiscard]] const std::vector<uint16_t>& getIndices() const;
    void mapIndices(const Context& context);

    [[nodiscard]] VkBuffer getBuffer() const;
    [[nodiscard]] VkDeviceMemory getBufferMemory() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

    [[nodiscard]] Types getType() const;

private:
    void createIndexBuffer(const Context& context, std::size_t indicesSize);

    std::vector<Vertex> g_vertices;
    VkBuffer g_vertexBuffer;
    VkBuffer g_stagingBuffer;
    VkDeviceMemory g_vertexBufferMemory;
    VkDeviceMemory g_stagingBufferMemory;

    std::vector<uint16_t> g_indices;
    VkBuffer g_indexBuffer;
    VkDeviceMemory g_indexBufferMemory;

    Types g_type;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_VERTEXBUFFER_HPP_INCLUDED
