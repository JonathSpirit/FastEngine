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

#ifndef _FGE_GRAPHIC_C_VECTOR_HPP_INCLUDED
#define _FGE_GRAPHIC_C_VECTOR_HPP_INCLUDED

#include <cstdint>
#define GLM_FORCE_CTOR_INIT
#include "glm/glm.hpp"

namespace fge
{

template<class T>
using Vector2 = glm::vec<2, T>;
template<class T>
using Vector3 = glm::vec<3, T>;

using Vector2i = Vector2<int32_t>;
using Vector2u = Vector2<uint32_t>;
using Vector2f = Vector2<float>;

using Vector3i = Vector3<int32_t>;
using Vector3u = Vector3<uint32_t>;
using Vector3f = Vector3<float>;

} // namespace fge

namespace glm
{

inline glm::vec2 operator*(glm::mat4 const& left, glm::vec2 const& right)
{
    return left * glm::vec4(right, 0.0f, 1.0f);
}

} // namespace glm

#endif //_FGE_GRAPHIC_C_VECTOR_HPP_INCLUDED
