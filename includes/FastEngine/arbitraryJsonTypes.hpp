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

#ifndef _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED
#define _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED

#include <SFML/System.hpp>
#include <json.hpp>

namespace sf
{

template<class T>
void to_json(nlohmann::json& j, const sf::Rect<T>& p);
template<class T>
void from_json(const nlohmann::json& j, sf::Rect<T>& p);

template<class T>
void to_json(nlohmann::json& j, const sf::Vector2<T>& p);
template<class T>
void from_json(const nlohmann::json& j, sf::Vector2<T>& p);

template<class T>
void to_json(nlohmann::json& j, const sf::Vector3<T>& p);
template<class T>
void from_json(const nlohmann::json& j, sf::Vector3<T>& p);

inline void to_json(nlohmann::json& j, const sf::Color& p);
inline void from_json(const nlohmann::json& j, sf::Color& p);

}//end sc

#include <FastEngine/arbitraryJsonTypes.inl>

#endif // _FGE_ARBITRARYJSONTYPES_HPP_INCLUDED
