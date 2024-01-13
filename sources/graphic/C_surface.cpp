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

#include "FastEngine/graphic/C_surface.hpp"
#include "SDL_image.h"

namespace fge
{

Surface::Surface() :
        g_surface(nullptr)
{}

Surface::Surface(int width, int height, fge::Color const& color) :
        g_surface(nullptr)
{
    this->create(width, height, color);
}

Surface::Surface(Surface const& r) :
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

Surface& Surface::operator=(Surface const& r)
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

bool Surface::create(int width, int height, fge::Color const& color)
{
    this->clear();
    this->g_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    this->fillRect(std::nullopt, color);
    return this->g_surface != nullptr;
}

bool Surface::loadFromFile(std::filesystem::path const& filePath)
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
bool Surface::loadFromMemory(void const* data, std::size_t size)
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

bool Surface::saveToFile(std::filesystem::path const& filePath) const
{
    return IMG_SavePNG(this->g_surface, filePath.string().c_str()) == 0;
}

fge::Vector2i Surface::getSize() const
{
    if (this->g_surface == nullptr)
    {
        return {0, 0};
    }
    return {this->g_surface->w, this->g_surface->h};
}

void Surface::createMaskFromColor(fge::Color const& color, uint8_t alpha)
{
    if (this->g_surface == nullptr)
    {
        return;
    }

    for (int h = 0; h < this->g_surface->h; ++h)
    {
        for (int w = 0; w < this->g_surface->w; ++w)
        {
            auto pixel = this->getPixel(w, h).value();
            if (pixel == color)
            {
                pixel._a = alpha;
                this->setPixel(w, h, pixel);
            }
        }
    }
}

bool Surface::setPixel(int x, int y, fge::Color const& color)
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

std::optional<fge::Color> Surface::getPixel(int x, int y) const
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

    return fge::Color{result};
}

void Surface::flipHorizontally()
{
    if (this->g_surface == nullptr)
    {
        return;
    }

    for (int h = 0; h < this->g_surface->h; ++h)
    {
        for (int w = 0; w < this->g_surface->w / 2; ++w)
        {
            auto pixelLeft = this->getPixel(w, h).value();
            auto pixelRight = this->getPixel(this->g_surface->w - w - 1, h).value();

            this->setPixel(w, h, pixelRight);
            this->setPixel(this->g_surface->w - w - 1, h, pixelLeft);
        }
    }
}

void Surface::flipVertically()
{
    if (this->g_surface == nullptr)
    {
        return;
    }

    for (int w = 0; w < this->g_surface->w; ++w)
    {
        for (int h = 0; h < this->g_surface->h / 2; ++h)
        {
            auto pixelTop = this->getPixel(w, h).value();
            auto pixelBottom = this->getPixel(w, this->g_surface->h - h - 1).value();

            this->setPixel(w, h, pixelBottom);
            this->setPixel(w, this->g_surface->h - h - 1, pixelTop);
        }
    }
}

bool Surface::blitSurface(Surface const& src, std::optional<SDL_Rect> const& srcRect, std::optional<SDL_Rect>& dstRect)
{
    if (this->g_surface == nullptr)
    {
        return false;
    }

    return SDL_BlitSurface(src.get(), srcRect.has_value() ? &srcRect.value() : nullptr, this->g_surface,
                           dstRect.has_value() ? &dstRect.value() : nullptr) == 0;
}

bool Surface::fillRect(std::optional<SDL_Rect> const& rect, fge::Color const& color)
{
    if (this->g_surface == nullptr)
    {
        return false;
    }
    return SDL_FillRect(this->g_surface, rect.has_value() ? &rect.value() : nullptr,
                        SDL_MapRGBA(this->g_surface->format, color._r, color._g, color._b, color._a)) == 0;
}

bool Surface::addBorder(int borderSize, fge::Color const& color)
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

fge::Vector2f Surface::normalizeTextureCoords(fge::Vector2i const& coords) const
{
    if (this->g_surface == nullptr)
    {
        return {0.0f, 0.0f};
    }
    return {static_cast<float>(coords.x) / static_cast<float>(this->g_surface->w),
            static_cast<float>(coords.y) / static_cast<float>(this->g_surface->h)};
}
fge::RectFloat Surface::normalizeTextureRect(fge::RectInt const& rect) const
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