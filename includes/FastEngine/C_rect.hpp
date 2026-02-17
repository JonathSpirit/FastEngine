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

#ifndef _FGE_C_RECT_HPP_INCLUDED
#define _FGE_C_RECT_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "C_vector.hpp"
#include <optional>
#include <type_traits>

namespace fge
{

template<class T>
class Rect
{
    static_assert(std::is_arithmetic_v<T>, "T must be arithmetic !");

public:
    Rect();
    Rect(Vector2<T> const& position, Vector2<T> const& size);

    template<class U>
    explicit Rect(Rect<U> const& rectangle);

    [[nodiscard]] bool operator==(Rect<T> const& right) const;
    [[nodiscard]] bool operator!=(Rect<T> const& right) const;

    [[nodiscard]] bool contains(Vector2<T> const& point) const;
    [[nodiscard]] bool contains(Rect<T> const& rectangle) const;
    [[nodiscard]] std::optional<Rect<T>> findIntersection(Rect<T> const& rectangle) const;

    [[nodiscard]] Vector2<T> getPosition() const;
    [[nodiscard]] Vector2<T> getSize() const;

    T _x;
    T _y;
    T _width;
    T _height;
};

using RectInt = Rect<int32_t>;
using RectUint = Rect<uint32_t>;
using RectFloat = Rect<float>;

fge::RectFloat operator*(glm::mat4 const& left, fge::RectFloat const& right);

} // namespace fge

#include "C_rect.inl"

#endif //_FGE_C_RECT_HPP_INCLUDED