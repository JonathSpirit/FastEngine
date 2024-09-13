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

#ifndef _FGE_VULKAN_C_SURFACE_HPP_INCLUDED
#define _FGE_VULKAN_C_SURFACE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include "FastEngine/C_vector.hpp"
#include "SDL_vulkan.h"
#include <string_view>

namespace fge::vulkan
{

class Instance;

class FGE_API Surface
{
public:
    explicit Surface(Instance& instance);
    Surface(Surface const& r) = delete;
    Surface(Surface&& r) noexcept;
    virtual ~Surface() = default;

    Surface& operator=(Surface const& r) = delete;
    Surface& operator=(Surface&& r) noexcept = delete;

    virtual void destroy() = 0;

    [[nodiscard]] VkSurfaceKHR get() const;
    [[nodiscard]] bool isCreated() const;

    [[nodiscard]] Instance& getInstance();
    [[nodiscard]] Instance const& getInstance() const;

    [[nodiscard]] virtual VkExtent2D getExtent() const;

protected:
    VkSurfaceKHR _g_surface;

private:
    Instance* g_instance;
};

class SurfaceWindow : public Surface
{
public:
    enum class Types
    {
        UNKNOWN,
        SDL
    };

    inline explicit SurfaceWindow(Instance& instance) :
            Surface(instance)
    {}

    [[nodiscard]] inline VkExtent2D getExtent() const override
    {
        auto const size = this->getSize();
        return {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};
    }

    [[nodiscard]] virtual Types getType() const = 0;

    [[nodiscard]] virtual fge::Vector2i getSize() const = 0;
};

class FGE_API SurfaceSDLWindow final : public SurfaceWindow
{
public:
    inline explicit SurfaceSDLWindow(Instance& instance) :
            SurfaceWindow(instance),
            g_window(nullptr)
    {}
    SurfaceSDLWindow(SurfaceSDLWindow&& r) noexcept;
    ~SurfaceSDLWindow() final;

    bool create(SDL_Window* window);
    bool create(std::string_view title, fge::Vector2i const& position, fge::Vector2i const& size, uint32_t flags);
    void destroy() final;

    [[nodiscard]] Types getType() const override;

    [[nodiscard]] fge::Vector2i getSize() const override;

    [[nodiscard]] SDL_Window* getWindow() const;

private:
    SDL_Window* g_window;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_SURFACE_HPP_INCLUDED
