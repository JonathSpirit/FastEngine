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

#ifndef _FGE_C_QUAD_HPP_INCLUDED
#define _FGE_C_QUAD_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_vector.hpp"
#include <array>

namespace fge
{

class FGE_API Quad
{
public:
    constexpr Quad() = default;
    constexpr explicit Quad(Vector2f const& fillValue);
    constexpr explicit Quad(RectFloat const& rect);
    constexpr Quad(Vector2f const& vec1, Vector2f const& vec2, Vector2f const& vec3, Vector2f const& vec4);

    [[nodiscard]] constexpr bool operator==(Quad const& right) const;
    [[nodiscard]] constexpr bool operator!=(Quad const& right) const;

    [[nodiscard]] bool contains(Vector2f const& point) const;
    [[nodiscard]] bool intersects(Quad const& quad) const;

    [[nodiscard]] constexpr fge::Vector2f const& operator[](std::size_t index) const;
    [[nodiscard]] constexpr fge::Vector2f& operator[](std::size_t index);

    std::array<fge::Vector2f, 4> _points{fge::Vector2f{0.0f}, fge::Vector2f{0.0f}, fge::Vector2f{0.0f},
                                         fge::Vector2f{0.0f}};
};

constexpr fge::Quad operator*(glm::mat4 const& left, fge::Quad const& right);

} // namespace fge

#include "C_quad.inl"

#endif //_FGE_C_QUAD_HPP_INCLUDED