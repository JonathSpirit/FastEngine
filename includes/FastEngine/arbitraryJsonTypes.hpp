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

#ifndef _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED
#define _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED

#include "C_rect.hpp"
#include "C_vector.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "json.hpp"
#include "tinyutf8.h"

namespace fge
{

template<class T>
void to_json(nlohmann::json& j, const fge::Rect<T>& p);
template<class T>
void from_json(const nlohmann::json& j, fge::Rect<T>& p);

template<class T>
void to_json(nlohmann::json& j, const fge::Vector2<T>& p);
template<class T>
void from_json(const nlohmann::json& j, fge::Vector2<T>& p);

template<class T>
void to_json(nlohmann::json& j, const fge::Vector3<T>& p);
template<class T>
void from_json(const nlohmann::json& j, fge::Vector3<T>& p);

inline void to_json(nlohmann::json& j, const fge::Color& p);
inline void from_json(const nlohmann::json& j, fge::Color& p);

} // namespace fge

namespace glm
{

template<class T>
void to_json(nlohmann::json& j, const glm::vec<2, T>& p);
template<class T>
void from_json(const nlohmann::json& j, glm::vec<2, T>& p);

template<class T>
void to_json(nlohmann::json& j, const glm::vec<3, T>& p);
template<class T>
void from_json(const nlohmann::json& j, glm::vec<3, T>& p);

} // namespace glm

namespace tiny_utf8
{

inline void to_json(nlohmann::json& j, const tiny_utf8::string& p);
inline void from_json(const nlohmann::json& j, tiny_utf8::string& p);

} // namespace tiny_utf8

#include <FastEngine/arbitraryJsonTypes.inl>

#endif // _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED
