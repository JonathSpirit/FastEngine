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

#include "FastEngine/vulkan/C_surface.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_instance.hpp"
#include "SDL_vulkan.h"

namespace fge::vulkan
{

Surface::Surface() :
        g_surface(VK_NULL_HANDLE),
        g_instance(nullptr),
        g_window(nullptr)
{}
Surface::Surface(Surface&& r) noexcept :
        g_surface(r.g_surface),
        g_instance(r.g_instance),
        g_window(r.g_window)
{
    r.g_surface = VK_NULL_HANDLE;
    r.g_instance = nullptr;
}
Surface::~Surface()
{
    this->destroy();
}

void Surface::create(SDL_Window* window, Instance& instance)
{
    if (SDL_Vulkan_CreateSurface(window, instance.getInstance(), &this->g_surface) == SDL_FALSE)
    {
        throw fge::Exception("failed to create surface !");
    }

    this->g_instance = &instance;
    this->g_window = window;
}
void Surface::destroy()
{
    if (this->g_surface != VK_NULL_HANDLE)
    {
        if (this->g_instance->getInstance() == VK_NULL_HANDLE)
        {
            throw fge::Exception("surface must be destroyed before the instance !");
        }
        vkDestroySurfaceKHR(this->g_instance->getInstance(), this->g_surface, nullptr);
        this->g_surface = VK_NULL_HANDLE;
        this->g_instance = nullptr;
        this->g_window = nullptr;
    }
}

VkSurfaceKHR Surface::getSurface() const
{
    return this->g_surface;
}

Instance& Surface::getInstance()
{
    return *this->g_instance;
}
Instance const& Surface::getInstance() const
{
    return *this->g_instance;
}

SDL_Window* Surface::getWindow() const
{
    return this->g_window;
}
fge::Vector2i Surface::getWindowSize() const
{
    fge::Vector2i size{0};
    SDL_GetWindowSize(this->g_window, &size.x, &size.y);
    return size;
}

} // namespace fge::vulkan