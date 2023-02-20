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

#include "FastEngine/graphic/C_surface.hpp"
#include "SDL_image.h"

namespace fge
{

Surface::Surface() :
        g_surface(nullptr)
{}

Surface::Surface(const Surface& r) :
        g_surface(nullptr)
{
    if (r.g_surface != nullptr)
    {
        this->g_surface = SDL_CreateRGBSurfaceWithFormatFrom(r.g_surface->pixels, r.g_surface->w, r.g_surface->h, 32,
                                                             r.g_surface->pitch, r.g_surface->format->format);
    }
}

Surface::Surface(Surface&& r) noexcept :
        g_surface(r.g_surface)
{
    r.g_surface = nullptr;
}

Surface::Surface(SDL_Surface* newSurface) :
        g_surface(newSurface)
{}

Surface::~Surface()
{
    this->clear();
}

Surface& Surface::operator=(const Surface& r)
{
    this->clear();
    if (r.g_surface != nullptr)
    {
        this->g_surface = SDL_CreateRGBSurfaceWithFormatFrom(r.g_surface->pixels, r.g_surface->w, r.g_surface->h, 32,
                                                             r.g_surface->pitch, r.g_surface->format->format);
    }
    return *this;
}

Surface& Surface::operator=(Surface&& r) noexcept
{
    this->clear();
    this->g_surface = r.g_surface;
    r.g_surface = nullptr;
    return *this;
}

void Surface::clear()
{
    if (this->g_surface != nullptr)
    {
        SDL_FreeSurface(this->g_surface);
        this->g_surface = nullptr;
    }
}

bool Surface::create(int width, int height, const fge::Color& color)
{
    this->clear();
    this->g_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    this->fillRect(std::nullopt, color);
    return this->g_surface != nullptr;
}

bool Surface::loadFromFile(const std::filesystem::path& filePath)
{
    this->clear();
    SDL_Surface* surface = IMG_Load(filePath.string().c_str());

    if (surface != nullptr)
    {
        this->g_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surface);
        return this->g_surface != nullptr;
    }
    return false;
}
bool Surface::loadFromMemory(const void* data, std::size_t size)
{
    this->clear();
    SDL_RWops* RWops = SDL_RWFromConstMem(data, static_cast<int>(size));
    SDL_Surface* surface = IMG_Load_RW(RWops, 1);

    if (surface != nullptr)
    {
        this->g_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surface);
        return this->g_surface != nullptr;
    }
    return false;
}

bool Surface::saveToFile(const std::filesystem::path& filePath) const
{
    return IMG_SavePNG(this->g_surface, filePath.string().c_str()) == 0;
}

glm::vec<2, int> Surface::getSize() const
{
    if (this->g_surface == nullptr)
    {
        return {0, 0};
    }
    return {this->g_surface->w, this->g_surface->h};
}

void Surface::createMaskFromColor([[maybe_unused]] const fge::Color& color, [[maybe_unused]] uint8_t alpha)
{
    ///TODO
}

bool Surface::setPixel(int x, int y, const fge::Color& color)
{
    if (this->g_surface == nullptr)
    {
        return false;
    }
    if (x < 0 || y < 0 || x >= this->g_surface->w || y >= this->g_surface->h)
    {
        return false;
    }

    auto* const targetPixel = reinterpret_cast<uint32_t*>(
            static_cast<uint8_t*>(this->g_surface->pixels) + static_cast<std::ptrdiff_t>(y * this->g_surface->pitch) +
            static_cast<std::ptrdiff_t>(x * this->g_surface->format->BytesPerPixel));

    *targetPixel = SDL_MapRGBA(this->g_surface->format, color._r, color._g, color._b, color._a);

    return true;
}

std::optional<SDL_Color> Surface::getPixel(int x, int y) const
{
    if (this->g_surface == nullptr)
    {
        return std::nullopt;
    }
    if (x < 0 || y < 0 || x >= this->g_surface->w || y >= this->g_surface->h)
    {
        return std::nullopt;
    }

    auto* const targetPixel = reinterpret_cast<uint32_t*>(
            static_cast<uint8_t*>(this->g_surface->pixels) + static_cast<std::ptrdiff_t>(y * this->g_surface->pitch) +
            static_cast<std::ptrdiff_t>(x * this->g_surface->format->BytesPerPixel));

    SDL_Color result;
    SDL_GetRGBA(*targetPixel, this->g_surface->format, &result.r, &result.g, &result.b, &result.a);

    return result;
}

void Surface::flipHorizontally() {}

void Surface::flipVertically() {}

bool Surface::blitSurface(const Surface& src, const std::optional<SDL_Rect>& srcRect, std::optional<SDL_Rect>& dstRect)
{
    if (this->g_surface == nullptr)
    {
        return false;
    }

    return SDL_BlitSurface(src.get(), srcRect.has_value() ? &srcRect.value() : nullptr, this->g_surface,
                           dstRect.has_value() ? &dstRect.value() : nullptr) == 0;
}

bool Surface::fillRect(const std::optional<SDL_Rect>& rect, const fge::Color& color)
{
    if (this->g_surface == nullptr)
    {
        return false;
    }
    return SDL_FillRect(this->g_surface, rect.has_value() ? &rect.value() : nullptr,
                        SDL_MapRGBA(this->g_surface->format, color._r, color._g, color._b, color._a)) == 0;
}

bool Surface::addBorder(int borderSize, const fge::Color& color)
{
    if (this->g_surface == nullptr)
    {
        return false;
    }
    SDL_Surface* transparentBorderSurface =
            SDL_CreateRGBSurfaceWithFormat(0, this->g_surface->w + borderSize * 2, this->g_surface->h + borderSize * 2,
                                           32, this->g_surface->format->format);
    if (transparentBorderSurface == nullptr)
    {
        return false;
    }

    SDL_FillRect(transparentBorderSurface, nullptr,
                 SDL_MapRGBA(transparentBorderSurface->format, color._r, color._g, color._b, color._a));

    SDL_Rect dstrect{borderSize, borderSize, this->g_surface->w, this->g_surface->h};
    SDL_BlitSurface(this->g_surface, nullptr, transparentBorderSurface, &dstrect);

    SDL_FreeSurface(this->g_surface);
    this->g_surface = transparentBorderSurface;
    return true;
}

void Surface::set(SDL_Surface* surface)
{
    this->clear();
    this->g_surface = surface;
}

SDL_Surface* Surface::get() const
{
    return this->g_surface;
}

fge::Vector2f Surface::normalizeTextureCoords(const fge::Vector2i& coords) const
{
    if (this->g_surface == nullptr)
    {
        return {0.0f, 0.0f};
    }
    return {static_cast<float>(coords.x) / static_cast<float>(this->g_surface->w),
            static_cast<float>(coords.y) / static_cast<float>(this->g_surface->h)};
}
fge::RectFloat Surface::normalizeTextureRect(const fge::RectInt& rect) const
{
    if (this->g_surface == nullptr)
    {
        return {{0.0f, 0.0f}, {0.0f, 0.0f}};
    }

    return {{static_cast<float>(rect._x) / static_cast<float>(this->g_surface->w),
             static_cast<float>(rect._y) / static_cast<float>(this->g_surface->h)},
            {static_cast<float>(rect._width) / static_cast<float>(this->g_surface->w),
             static_cast<float>(rect._height) / static_cast<float>(this->g_surface->h)}};
}

} // namespace fge