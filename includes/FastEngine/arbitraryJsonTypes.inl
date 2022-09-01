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

namespace sf
{

template<class T>
void to_json(nlohmann::json& j, const sf::Vector2<T>& p)
{
    j = nlohmann::json{{"x", p.x},
                       {"y", p.y}};
}
template<class T>
void from_json(const nlohmann::json& j, sf::Vector2<T>& p)
{
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}

template<class T>
void to_json(nlohmann::json& j, const sf::Vector3<T>& p)
{
    j = nlohmann::json{{"x", p.x},
                       {"y", p.y},
                       {"z", p.z}};
}
template<class T>
void from_json(const nlohmann::json& j, sf::Vector3<T>& p)
{
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
    j.at("z").get_to(p.z);
}

inline void to_json(nlohmann::json& j, const sf::Color& p)
{
    j = p.toInteger();
}
inline void from_json(const nlohmann::json& j, sf::Color& p)
{
    uint32_t color{0};
    j.get_to(color);
    p = sf::Color{color};
}

}//end sf
