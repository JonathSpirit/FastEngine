/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_GRAPHIC_C_SURFACE_HPP_INCLUDED
#define _FGE_GRAPHIC_C_SURFACE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "SDL_render.h"
#include <cstdint>
#include <filesystem>
#include <optional>

namespace fge
{

#ifdef FGE_DEF_SERVER
namespace vulkan
{

class Context;

} // namespace vulkan
#endif

/**
 * \class Surface
 * \ingroup graphics
 * \brief Abstraction of SDL_Surface
 *
 * This class is an abstraction of SDL_Surface.
 * It can be used to load image from file or memory with the help of SDL_image.
 * The texture manager must be initialized in order to load image from file (or just init SDL_image manually).
 *
 * The surface is automatically destroyed when the object is destroyed.
 *
 * \see fge::texture::Init
 */
class FGE_API Surface
{
public:
    Surface();
#ifdef FGE_DEF_SERVER
    //TODO: this is here in order to be interchangeable easily with TextureImage
    //when building for the server or client target.
    //For future, I want to remove that and have a TextureImage server version
    //instead of switching between Surface <-> TextureImage with the fge::TextureType alias
    explicit Surface([[maybe_unused]] fge::vulkan::Context const& r) :
            Surface()
    {}
#endif
    Surface(int width, int height, fge::Color const& color = {0, 0, 0, 255});
    Surface(Surface const& r);
    Surface(Surface&& r) noexcept;
    explicit Surface(SDL_Surface* newSurface);
    ~Surface();

    Surface& operator=(Surface const& r);
    Surface& operator=(Surface&& r) noexcept;

    /**
     * \brief Destroy the surface
     */
    void clear();

    bool create(int width, int height, fge::Color const& color = {0, 0, 0, 255});
    bool loadFromFile(std::filesystem::path const& filePath);
    bool loadFromMemory(void const* data, std::size_t size);

    /**
     * \brief Save the surface to a PNG format file
     *
     * \param filePath The path to the file
     * \return true if the file was successfully saved
     */
    bool saveToFile(std::filesystem::path const& filePath) const;

    [[nodiscard]] fge::Vector2i getSize() const;

    /**
     * \brief Create a transparent mask from a color
     *
     * \param color The color to make transparent
     * \param alpha The alpha value of the transparent color
     */
    void createMaskFromColor(fge::Color const& color, uint8_t alpha = 0);

    bool setPixel(int x, int y, fge::Color const& color);
    [[nodiscard]] std::optional<fge::Color> getPixel(int x, int y) const;

    void setCircle(int x, int y, unsigned int radius, fge::Color const& color);

    void flipHorizontally();
    void flipVertically();

    void stretch(int width, int height);
    enum class ShearBaseSides
    {
        Top,
        Bottom
    };
    void shear(float angle, ShearBaseSides side);

    /**
     * \brief Blit a surface on this surface
     *
     * \see https://wiki.libsdl.org/SDL2/SDL_BlitSurface
     *
     * \param src The source surface
     * \param srcRect The source rectangle
     * \param dstRect The destination rectangle
     * \return true if the blit was successful
     */
    bool blitSurface(Surface const& src, std::optional<SDL_Rect> const& srcRect, std::optional<SDL_Rect>& dstRect);

    /**
     * \brief Fill a rectangle section of the surface with a color
     *
     * \see https://wiki.libsdl.org/SDL2/SDL_FillRect
     *
     * \param rect The rectangle to fill
     * \param color The color to fill with
     * \return true if the rectangle was successfully filled
     */
    bool fillRect(std::optional<SDL_Rect> const& rect, fge::Color const& color);

    /**
     * \brief Add a border to the surface with a specific color
     *
     * An example of this function is to add a transparent border to a surface and then
     * convert it to a texture.
     *
     * \param borderSize The size of the border
     * \param color The color of the border
     * \return true if the border was successfully added
     */
    bool addBorder(int borderSize, fge::Color const& color);

    /**
     * \brief Set a new surface
     *
     * The newly set surface will be destroyed when the object is destroyed.
     *
     * \param surface The new surface
     */
    void set(SDL_Surface* surface);
    /**
     * \brief Get the SDL_Surface pointer
     *
     * \return The SDL_Surface pointer
     */
    [[nodiscard]] SDL_Surface* get() const;

    /**
     * \brief Convert some pixel coordinates to texture coordinates (0.0f to 1.0f)
     *
     * \param coords The pixel coordinates
     * \return The texture coordinates
     */
    [[nodiscard]] fge::Vector2f normalizeTextureCoords(fge::Vector2i const& coords) const;
    /**
     * \brief Convert a pixel rectangle to a texture rectangle (0.0f to 1.0f)
     *
     * \param rect The pixel rectangle
     * \return The texture rectangle
     */
    [[nodiscard]] fge::RectFloat normalizeTextureRect(fge::RectInt const& rect) const;

private:
    SDL_Surface* g_surface;
};

} // namespace fge

#endif //_FGE_GRAPHIC_C_SURFACE_HPP_INCLUDED
