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

#ifndef _FGE_VULKAN_C_SURFACE_HPP_INCLUDED
#define _FGE_VULKAN_C_SURFACE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include "FastEngine/C_vector.hpp"
#include "SDL_video.h"
#include <string_view>

#define FGE_WINDOWPOS_UNDEFINED {SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED}
#define FGE_WINDOWPOS_CENTERED {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED}

namespace fge::vulkan
{

class Instance;

/**
 * \class Surface
 * \ingroup vulkan
 * \brief Vulkan surface abstraction
 *
 * This base class is used to create a Vulkan surface.
 */
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

class FGE_API SurfaceHeadless final : public Surface
{
public:
    explicit SurfaceHeadless(Instance& instance, VkExtent2D extent = {0, 0});
    SurfaceHeadless(SurfaceHeadless&& r) noexcept;
    ~SurfaceHeadless() override;

    bool create(VkExtent2D extent);

    void setExtent(VkExtent2D extent);

    void destroy() override;

    [[nodiscard]] VkExtent2D getExtent() const override;

private:
    VkExtent2D g_extent;
};

/**
 * \class SurfaceWindow
 * \ingroup vulkan
 * \brief Vulkan OS window surface
 *
 * This base class is used to create a Vulkan surface on a window.
 * It's also an abstraction on multiple window creation library classes.
 */
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
    [[nodiscard]] virtual fge::Vector2i getPosition() const = 0;
};

/**
 * \class SurfaceSDLWindow
 * \ingroup vulkan
 * \brief Vulkan OS window surface made with SDL
 *
 * This class is used to create a Vulkan surface on a SDL window.
 */
class FGE_API SurfaceSDLWindow final : public SurfaceWindow
{
public:
    inline explicit SurfaceSDLWindow(Instance& instance) :
            SurfaceWindow(instance),
            g_window(nullptr)
    {}
    SurfaceSDLWindow(Instance& instance,
                     std::string_view title,
                     fge::Vector2i const& position,
                     fge::Vector2i const& size,
                     uint32_t flags);
    SurfaceSDLWindow(Instance& instance, fge::Vector2i const& position, fge::Vector2i const& size, uint32_t flags);
    SurfaceSDLWindow(SurfaceSDLWindow&& r) noexcept;
    ~SurfaceSDLWindow() override;

    /**
     * \brief Create a surface by taking an already created SDL_Window.
     *
     * The window must be created with no error and with the SDL_WINDOW_VULKAN flag.
     * This method take the ownership of the SDL window, that means it will automatically destroy it.
     *
     * \param window The SDL window handle
     * \return \b true if created successfully, \b false otherwise
     */
    bool create(SDL_Window* window);
    /**
     * \brief Create a surface and the SDL_Window.
     *
     * You can use FGE_WINDOWPOS_UNDEFINED or FGE_WINDOWPOS_CENTERED helpers for the position.
     *
     * \param title A valid UTF8 string view
     * \param position The position of the window when created
     * \param size The size of the window
     * \param flags The flag passed to SDL_CreateWindow
     */
    bool create(std::string_view title, fge::Vector2i const& position, fge::Vector2i const& size, uint32_t flags);
    void destroy() override;

    [[nodiscard]] Types getType() const override;

    [[nodiscard]] fge::Vector2i getSize() const override;
    [[nodiscard]] fge::Vector2i getPosition() const override;

    [[nodiscard]] SDL_Window* getWindow() const;

private:
    SDL_Window* g_window;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_SURFACE_HPP_INCLUDED
