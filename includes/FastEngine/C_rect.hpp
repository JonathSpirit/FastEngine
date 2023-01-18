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

#ifndef _FGE_GRAPHIC_C_RECT_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RECT_HPP_INCLUDED

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

template <class T>
class Rect
{
    static_assert(std::is_arithmetic_v<T>, "T must be arithmetic !");
public:
    Rect();
    Rect(const Vector2<T>& position, const Vector2<T>& size);

    template <class U>
    explicit Rect(const Rect<U>& rectangle);

    [[nodiscard]] bool operator==(const Rect<T>& right) const;
    [[nodiscard]] bool operator!=(const Rect<T>& right) const;

    [[nodiscard]] bool contains(const Vector2<T>& point) const;
    [[nodiscard]] std::optional<Rect<T>> findIntersection(const Rect<T>& rectangle) const;

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

fge::RectFloat operator*(const glm::mat4& left, const fge::RectFloat& right);

}//end fge

#include "C_rect.inl"

#endif //_FGE_GRAPHIC_C_RECT_HPP_INCLUDED