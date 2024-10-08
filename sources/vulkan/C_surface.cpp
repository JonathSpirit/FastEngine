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
#include "FastEngine/C_alloca.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_instance.hpp"
#include <cstring>

namespace fge::vulkan
{

//Surface

Surface::Surface(Instance& instance) :
        _g_surface(VK_NULL_HANDLE),
        g_instance(&instance)
{}
Surface::Surface(Surface&& r) noexcept :
        _g_surface(r._g_surface),
        g_instance(r.g_instance)
{
    r._g_surface = VK_NULL_HANDLE;
}

VkSurfaceKHR Surface::get() const
{
    return this->_g_surface;
}
bool Surface::isCreated() const
{
    return this->_g_surface != VK_NULL_HANDLE;
}

Instance& Surface::getInstance()
{
    return *this->g_instance;
}
Instance const& Surface::getInstance() const
{
    return *this->g_instance;
}

VkExtent2D Surface::getExtent() const
{
    return {0, 0};
}

//SurfaceHeadless

SurfaceHeadless::SurfaceHeadless(Instance& instance, VkExtent2D extent) :
        Surface(instance)
{
    this->create(extent);
}

SurfaceHeadless::SurfaceHeadless(SurfaceHeadless&& r) noexcept :
        Surface(std::move(r))
{
    this->g_extent = r.g_extent;
}

SurfaceHeadless::~SurfaceHeadless()
{
    this->destroy();
}

bool SurfaceHeadless::create(VkExtent2D extent)
{
    this->destroy();

    VkHeadlessSurfaceCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    if (vkCreateHeadlessSurfaceEXT == nullptr)
    {
        throw fge::Exception("VK_EXT_HEADLESS_SURFACE: Vulkan headless surface extension not loaded!");
    }

    if (vkCreateHeadlessSurfaceEXT(this->getInstance().get(), &createInfo, nullptr, &this->_g_surface) != VK_SUCCESS)
    {
        return false;
    }

    this->g_extent = extent;
    return true;
}

void SurfaceHeadless::setExtent(VkExtent2D extent)
{
    this->g_extent = extent;
}

void SurfaceHeadless::destroy()
{
    if (this->isCreated())
    {
        if (this->getInstance().get() == VK_NULL_HANDLE)
        {
            throw fge::Exception("surface must be destroyed before the instance !");
        }

        vkDestroySurfaceKHR(this->getInstance().get(), this->_g_surface, nullptr);
        this->_g_surface = VK_NULL_HANDLE;
    }
}

VkExtent2D SurfaceHeadless::getExtent() const
{
    return this->g_extent;
}

//SurfaceSDLWindow

SurfaceSDLWindow::SurfaceSDLWindow(Instance& instance,
                                   std::string_view title,
                                   fge::Vector2i const& position,
                                   fge::Vector2i const& size,
                                   uint32_t flags) :
        SurfaceSDLWindow(instance)
{
    this->create(title, position, size, flags);
}
SurfaceSDLWindow::SurfaceSDLWindow(Instance& instance,
                                   fge::Vector2i const& position,
                                   fge::Vector2i const& size,
                                   uint32_t flags) :
        SurfaceSDLWindow(instance)
{
    this->create(instance.getApplicationName().cpp_str_view(), position, size, flags);
}
SurfaceSDLWindow::SurfaceSDLWindow(SurfaceSDLWindow&& r) noexcept :
        SurfaceWindow(std::move(r)),
        g_window(r.g_window)
{
    r.g_window = nullptr;
}
SurfaceSDLWindow::~SurfaceSDLWindow()
{
    this->destroy();
}

bool SurfaceSDLWindow::create(SDL_Window* window)
{
    this->destroy();

    if (SDL_Vulkan_CreateSurface(window, this->getInstance().get(), &this->_g_surface) == SDL_FALSE)
    {
        return false;
    }

    this->g_window = window;
    return true;
}
bool SurfaceSDLWindow::create(std::string_view title,
                              fge::Vector2i const& position,
                              fge::Vector2i const& size,
                              uint32_t flags)
{
    this->destroy();

    char* ctitle;
    FGE_ALLOCA_STRINGVIEW_TO_CSTRING(ctitle, title);

    flags |= SDL_WINDOW_VULKAN;

    this->g_window = SDL_CreateWindow(ctitle, position.x, position.y, size.x, size.y, flags);

    if (this->g_window == nullptr)
    {
        return false;
    }

    if (SDL_Vulkan_CreateSurface(this->g_window, this->getInstance().get(), &this->_g_surface) == SDL_FALSE)
    {
        SDL_DestroyWindow(this->g_window);
        this->g_window = nullptr;
        return false;
    }

    return true;
}
void SurfaceSDLWindow::destroy()
{
    if (this->isCreated())
    {
        if (this->getInstance().get() == VK_NULL_HANDLE)
        {
            throw fge::Exception("surface must be destroyed before the instance !");
        }

        vkDestroySurfaceKHR(this->getInstance().get(), this->_g_surface, nullptr);
        this->_g_surface = VK_NULL_HANDLE;

        SDL_DestroyWindow(this->g_window);
        this->g_window = nullptr;
    }
}

SurfaceWindow::Types SurfaceSDLWindow::getType() const
{
    return Types::SDL;
}

fge::Vector2i SurfaceSDLWindow::getSize() const
{
    fge::Vector2i size{0};
    SDL_GetWindowSize(this->g_window, &size.x, &size.y);
    return size;
}
fge::Vector2i SurfaceSDLWindow::getPosition() const
{
    fge::Vector2i position{0};
    SDL_GetWindowPosition(this->g_window, &position.x, &position.y);
    return position;
}

SDL_Window* SurfaceSDLWindow::getWindow() const
{
    return this->g_window;
}

} // namespace fge::vulkan