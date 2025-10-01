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

#ifndef _FGE_GRAPHIC_C_COLOR_HPP_INCLUDED
#define _FGE_GRAPHIC_C_COLOR_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "volk.h"
#include "FastEngine/C_vector.hpp"
#include "SDL_pixels.h"

namespace fge
{

class Color
{
public:
    constexpr Color() noexcept :
            _r(0),
            _g(0),
            _b(0),
            _a(255)
    {}
    constexpr explicit Color(SDL_Color const& sdlColor) noexcept :
            _r(sdlColor.r),
            _g(sdlColor.g),
            _b(sdlColor.b),
            _a(sdlColor.a)
    {}
    constexpr explicit Color(VkClearColorValue const& clearColorValue) noexcept :
            _r(static_cast<uint8_t>(clearColorValue.float32[0] * 255.0f)),
            _g(static_cast<uint8_t>(clearColorValue.float32[1] * 255.0f)),
            _b(static_cast<uint8_t>(clearColorValue.float32[2] * 255.0f)),
            _a(static_cast<uint8_t>(clearColorValue.float32[3] * 255.0f))
    {}
    constexpr explicit Color(glm::vec4 const& vec4) noexcept :
            _r(static_cast<uint8_t>(vec4.r * 255.0f)),
            _g(static_cast<uint8_t>(vec4.g * 255.0f)),
            _b(static_cast<uint8_t>(vec4.b * 255.0f)),
            _a(static_cast<uint8_t>(vec4.a * 255.0f))
    {}
    constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept :
            _r(red),
            _g(green),
            _b(blue),
            _a(alpha)
    {}
    constexpr explicit Color(uint32_t color) noexcept :
            _r(static_cast<uint8_t>((color & 0xFF000000) >> 24)),
            _g(static_cast<uint8_t>((color & 0x00FF0000) >> 16)),
            _b(static_cast<uint8_t>((color & 0x0000FF00) >> 8)),
            _a(static_cast<uint8_t>(color & 0x000000FF))
    {}

    constexpr operator SDL_Color() const
    {
        return SDL_Color{.r = this->_r, .g = this->_g, .b = this->_b, .a = this->_a};
    }
    constexpr operator VkClearColorValue() const
    {
        return {{static_cast<float>(this->_r) / 255.0f, static_cast<float>(this->_g) / 255.0f,
                 static_cast<float>(this->_b) / 255.0f, static_cast<float>(this->_a) / 255.0f}};
    }
    constexpr operator glm::vec4() const
    {
        return {static_cast<float>(this->_r) / 255.0f, static_cast<float>(this->_g) / 255.0f,
                static_cast<float>(this->_b) / 255.0f, static_cast<float>(this->_a) / 255.0f};
    }
    constexpr operator glm::uvec4() const { return {this->_r, this->_g, this->_b, this->_a}; }
    constexpr operator uint32_t() const { return this->toInteger(); }

    [[nodiscard]] constexpr uint32_t toInteger() const
    {
        return (static_cast<uint32_t>(this->_r) << 24) | (static_cast<uint32_t>(this->_g) << 16) |
               (static_cast<uint32_t>(this->_b) << 8) | static_cast<uint32_t>(this->_a);
    }

    constexpr bool operator==(Color const& right) const
    {
        return this->_r == right._r && this->_g == right._g && this->_b == right._b && this->_a == right._a;
    }
    constexpr bool operator!=(Color const& right) const { return !this->operator==(right); }

    constexpr Color operator+(Color const& right) const
    {
        uint16_t const red = static_cast<uint16_t>(this->_r) + right._r;
        uint16_t const green = static_cast<uint16_t>(this->_g) + right._g;
        uint16_t const blue = static_cast<uint16_t>(this->_b) + right._b;
        uint16_t const alpha = static_cast<uint16_t>(this->_a) + right._a;
        return {red > 255 ? uint8_t(255) : static_cast<uint8_t>(red),
                green > 255 ? uint8_t(255) : static_cast<uint8_t>(green),
                blue > 255 ? uint8_t(255) : static_cast<uint8_t>(blue),
                alpha > 255 ? uint8_t(255) : static_cast<uint8_t>(alpha)};
    }
    constexpr Color operator-(Color const& right) const
    {
        auto const red = static_cast<int16_t>(this->_r - right._r);
        auto const green = static_cast<int16_t>(this->_g - right._g);
        auto const blue = static_cast<int16_t>(this->_b - right._b);
        auto const alpha = static_cast<int16_t>(this->_a - right._a);
        return {red < 0 ? uint8_t(0) : static_cast<uint8_t>(red), green < 0 ? uint8_t(0) : static_cast<uint8_t>(green),
                blue < 0 ? uint8_t(0) : static_cast<uint8_t>(blue),
                alpha < 0 ? uint8_t(0) : static_cast<uint8_t>(alpha)};
    }
    constexpr Color operator*(Color const& right) const
    {
        uint16_t const red = static_cast<uint16_t>(this->_r) * right._r;
        uint16_t const green = static_cast<uint16_t>(this->_g) * right._g;
        uint16_t const blue = static_cast<uint16_t>(this->_b) * right._b;
        uint16_t const alpha = static_cast<uint16_t>(this->_a) * right._a;
        return {static_cast<uint8_t>(red / 255), static_cast<uint8_t>(green / 255), static_cast<uint8_t>(blue / 255),
                static_cast<uint8_t>(alpha / 255)};
    }

    constexpr Color& operator+=(Color const& right) { return this->operator=(this->operator+(right)); }
    constexpr Color& operator-=(Color const& right) { return this->operator=(this->operator-(right)); }
    constexpr Color& operator*=(Color const& right) { return this->operator=(this->operator*(right)); }

    uint8_t _r;
    uint8_t _g;
    uint8_t _b;
    uint8_t _a;

    static Color const Black;       //!< Black predefined color
    static Color const White;       //!< White predefined color
    static Color const Red;         //!< Red predefined color
    static Color const Green;       //!< Green predefined color
    static Color const Blue;        //!< Blue predefined color
    static Color const Yellow;      //!< Yellow predefined color
    static Color const Magenta;     //!< Magenta predefined color
    static Color const Cyan;        //!< Cyan predefined color
    static Color const Transparent; //!< Transparent (black) predefined color
};

inline Color const Color::Black(0, 0, 0);
inline Color const Color::White(255, 255, 255);
inline Color const Color::Red(255, 0, 0);
inline Color const Color::Green(0, 255, 0);
inline Color const Color::Blue(0, 0, 255);
inline Color const Color::Yellow(255, 255, 0);
inline Color const Color::Magenta(255, 0, 255);
inline Color const Color::Cyan(0, 255, 255);
inline Color const Color::Transparent(0, 0, 0, 0);

} // namespace fge

#endif // _FGE_GRAPHIC_C_COLOR_HPP_INCLUDED
