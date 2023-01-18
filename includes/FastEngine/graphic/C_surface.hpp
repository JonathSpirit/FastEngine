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

#ifndef _FGE_GRAPHIC_C_SURFACE_HPP_INCLUDED
#define _FGE_GRAPHIC_C_SURFACE_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_vector.hpp"
#include <FastEngine/graphic/C_color.hpp>
#include <SDL_render.h>
#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>
#include <optional>

namespace fge
{

class FGE_API Surface
{
public:
    Surface();
    Surface(const Surface& r);
    Surface(Surface&& r) noexcept;
    explicit Surface(SDL_Surface* newSurface);
    ~Surface();

    Surface& operator=(const Surface& r);
    Surface& operator=(Surface&& r) noexcept;

    void clear();

    bool create(int width, int height, const fge::Color& color={0, 0, 0, 255});
    bool loadFromFile(const std::filesystem::path& filePath);
    bool loadFromMemory(const void *data, std::size_t size);

    bool saveToFile(const std::filesystem::path& filePath) const;

    [[nodiscard]] glm::vec<2, int> getSize() const;

    void createMaskFromColor(const fge::Color& color, uint8_t alpha=0);

    bool setPixel (int x, int y, const fge::Color& color);
    [[nodiscard]] std::optional<SDL_Color> getPixel (int x, int y) const;

    void flipHorizontally();
    void flipVertically();

    bool blitSurface(const Surface& src, const std::optional<SDL_Rect>& srcRect, std::optional<SDL_Rect>& dstRect);

    bool fillRect(const std::optional<SDL_Rect>& rect, const fge::Color& color);

    bool addBorder(int borderSize, const fge::Color& color);

    void set(SDL_Surface* surface);
    [[nodiscard]] SDL_Surface* get() const;

    [[nodiscard]] fge::Vector2f normalizeTextureCoords(const fge::Vector2i& coords) const;
    [[nodiscard]] fge::RectFloat normalizeTextureRect(const fge::RectInt& rect) const;

private:
    SDL_Surface* g_surface;
};

}//end fge

#endif //_FGE_GRAPHIC_C_SURFACE_HPP_INCLUDED
