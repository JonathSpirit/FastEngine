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

#ifndef _FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED
#define _FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "SDL_vulkan.h"
#include "volk.h"
#include <cstdint>

namespace fge::vulkan
{

class Context;

class FGE_API UniformBuffer
{
public:
    UniformBuffer();
    UniformBuffer(const UniformBuffer& r);
    UniformBuffer(UniformBuffer&& r) noexcept;
    ~UniformBuffer();

    UniformBuffer& operator=(const UniformBuffer& r) = delete;
    UniformBuffer& operator=(UniformBuffer&& r) noexcept = delete;

    void create(const Context& context, VkDeviceSize bufferSize);
    void destroy();

    [[nodiscard]] VkBuffer getBuffer() const;
    [[nodiscard]] VkDeviceMemory getBufferMemory() const;
    [[nodiscard]] void* getBufferMapped() const;
    [[nodiscard]] VkDeviceSize getBufferSize() const;

    [[nodiscard]] const Context* getContext() const;

    void copyData(const void* data, std::size_t size) const;

private:
    VkBuffer g_uniformBuffer;
    VkDeviceMemory g_uniformBufferMemory;
    void* g_uniformBufferMapped;
    VkDeviceSize g_bufferSize;

    const Context* g_context;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_UNIFORMBUFFER_HPP_INCLUDED
