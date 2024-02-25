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

namespace fge
{

constexpr Quad::Quad(Vector2f const& fillValue) :
        _points{fillValue, fillValue, fillValue, fillValue}
{}
constexpr Quad::Quad(RectFloat const& rect) :
        _points{Vector2f{rect._x, rect._y}, Vector2f{rect._x + rect._width, rect._y},
                Vector2f{rect._x + rect._width, rect._y + rect._height}, Vector2f{rect._x, rect._y + rect._height}}
{}
constexpr Quad::Quad(Vector2f const& vec1, Vector2f const& vec2, Vector2f const& vec3, Vector2f const& vec4) :
        _points{vec1, vec2, vec3, vec4}
{}

constexpr bool Quad::operator==(Quad const& right) const
{
    return _points[0] == right._points[0] && _points[1] == right._points[1] && _points[2] == right._points[2] &&
           _points[3] == right._points[3];
}
constexpr bool Quad::operator!=(Quad const& right) const
{
    return !(*this == right);
}

constexpr fge::Vector2f const& Quad::operator[](std::size_t index) const
{
    return this->_points[index];
}
constexpr fge::Vector2f& Quad::operator[](std::size_t index)
{
    return this->_points[index];
}

constexpr fge::Quad operator*(glm::mat4 const& left, fge::Quad const& right)
{
    return {left * glm::vec4{right[0], 0.0f, 1.0f}, left * glm::vec4{right[1], 0.0f, 1.0f},
            left * glm::vec4{right[2], 0.0f, 1.0f}, left * glm::vec4{right[3], 0.0f, 1.0f}};
}

} // namespace fge