/*
 * Copyright 2023 Guillaume Guillet
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

#ifndef _FGE_VULKAN_C_SURFACE_HPP_INCLUDED
#define _FGE_VULKAN_C_SURFACE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include "SDL_vulkan.h"

namespace fge::vulkan
{

class Instance;

class FGE_API Surface
{
public:
    Surface();
    Surface(Surface const& r) = delete;
    Surface(Surface&& r) noexcept;
    ~Surface();

    Surface& operator=(Surface const& r) = delete;
    Surface& operator=(Surface&& r) noexcept = delete;

    void create(Instance& instance);
    void destroy();

    [[nodiscard]] VkSurfaceKHR getSurface() const;

    [[nodiscard]] Instance& getInstance();
    [[nodiscard]] Instance const& getInstance() const;

private:
    VkSurfaceKHR g_surface;
    Instance* g_instance;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_SURFACE_HPP_INCLUDED
