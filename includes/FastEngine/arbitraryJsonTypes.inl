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

/**template<class T>
void to_json(nlohmann::json& j, const sf::Rect<T>& p)
{
    j = nlohmann::json{{"x", p.left}, {"y", p.top}, {"w", p.width}, {"h", p.height}};
}
template<class T>
void from_json(const nlohmann::json& j, sf::Rect<T>& p)
{
    j.at("x").get_to(p.left);
    j.at("y").get_to(p.top);
    j.at("w").get_to(p.width);
    j.at("h").get_to(p.height);
}

template<class T>
void to_json(nlohmann::json& j, const sf::Vector2<T>& p)
{
    j = nlohmann::json{{"x", p.x}, {"y", p.y}};
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
    j = nlohmann::json{{"x", p.x}, {"y", p.y}, {"z", p.z}};
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
}**/

} // namespace sf

namespace fge
{

template<class T>
void to_json(nlohmann::json& j, const fge::Rect<T>& p)
{
    j = nlohmann::json{{"x", p._x}, {"y", p._y}, {"w", p._width}, {"h", p._height}};
}
template<class T>
void from_json(const nlohmann::json& j, fge::Rect<T>& p)
{
    j.at("x").get_to(p._x);
    j.at("y").get_to(p._y);
    j.at("w").get_to(p._width);
    j.at("h").get_to(p._height);
}

template<class T>
void to_json(nlohmann::json& j, const fge::Vector2<T>& p)
{
    j = nlohmann::json{{"x", p.x}, {"y", p.y}};
}
template<class T>
void from_json(const nlohmann::json& j, fge::Vector2<T>& p)
{
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}

template<class T>
void to_json(nlohmann::json& j, const fge::Vector3<T>& p)
{
    j = nlohmann::json{{"x", p.x}, {"y", p.y}, {"z", p.z}};
}
template<class T>
void from_json(const nlohmann::json& j, fge::Vector3<T>& p)
{
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
    j.at("z").get_to(p.z);
}

inline void to_json(nlohmann::json& j, const fge::Color& p)
{
    j = p.toInteger();
}
inline void from_json(const nlohmann::json& j, fge::Color& p)
{
    uint32_t color{0};
    j.get_to(color);
    p = fge::Color{color};
}

} // namespace fge

namespace glm
{

template<class T>
void to_json(nlohmann::json& j, const glm::vec<2, T>& p)
{
    j = nlohmann::json{{"x", p.x}, {"y", p.y}};
}
template<class T>
void from_json(const nlohmann::json& j, glm::vec<2, T>& p)
{
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}

template<class T>
void to_json(nlohmann::json& j, const glm::vec<3, T>& p)
{
    j = nlohmann::json{{"x", p.x}, {"y", p.y}, {"z", p.z}};
}
template<class T>
void from_json(const nlohmann::json& j, glm::vec<3, T>& p)
{
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
    j.at("z").get_to(p.z);
}

} // namespace glm

namespace tiny_utf8
{

inline void to_json(nlohmann::json& j, const tiny_utf8::string& p)
{
    j = p.c_str();
}
inline void from_json(const nlohmann::json& j, tiny_utf8::string& p)
{
    std::string str;
    j.get_to(str);
    p = tiny_utf8::string{std::move(str)};
}

} // namespace tiny_utf8
