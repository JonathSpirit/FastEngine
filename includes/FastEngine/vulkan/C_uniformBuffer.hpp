/*
 * Copyright 2024 Guillaume Guillet
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

#ifndef _FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED
#define _FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <cstdint>
#ifdef FGE_DEF_SERVER
    #include <vector>
#endif

namespace fge::vulkan
{

class FGE_API UniformBuffer : public ContextAware
{
public:
    enum class Types
    {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,
        INDIRECT_BUFFER
    };

    explicit UniformBuffer(Context const& context, Types type = Types::UNIFORM_BUFFER);
    UniformBuffer(UniformBuffer const& r);
    UniformBuffer(UniformBuffer&& r) noexcept;
    ~UniformBuffer() override;

    UniformBuffer& operator=(UniformBuffer const& r);
    UniformBuffer& operator=(UniformBuffer&& r) noexcept;

    void create(VkDeviceSize bufferSize, Types type = Types::UNIFORM_BUFFER);
    void resize(VkDeviceSize bufferSize, bool shrink = false);
    void shrinkToFit();
    void destroy() final;

    [[nodiscard]] VkBuffer getBuffer() const;
    [[nodiscard]] VmaAllocation getBufferAllocation() const;
    [[nodiscard]] void* getBufferMapped() const;
    [[nodiscard]] VkDeviceSize getBufferSize() const;
    [[nodiscard]] VkDeviceSize getBufferCapacity() const;
    [[nodiscard]] Types getType() const;

    void copyData(void const* data, std::size_t size) const;

private:
#ifndef FGE_DEF_SERVER
    void createBuffer(VkDeviceSize bufferSize, VkBuffer& buffer, VmaAllocation& bufferAllocation);

    VkBuffer g_uniformBuffer;
    VmaAllocation g_uniformBufferAllocation;
    void* g_uniformBufferMapped;
    VkDeviceSize g_bufferSize;
    VkDeviceSize g_bufferCapacity;
#else
    mutable std::vector<uint8_t> g_uniformBuffer;
#endif
    Types g_type;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED
