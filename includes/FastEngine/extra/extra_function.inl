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

namespace fge
{

///Utility
char UnicodeToChar(uint32_t unicode)
{
    return (unicode < 128) ? static_cast<char>(unicode) : 0;
}

///Detection
inline bool
IsVertexInCone(fge::Line const& line1, fge::Line const& line2, fge::Vector2f const& origin, fge::Vector2f const& vertex)
{
    auto const relativePos = vertex - origin;

    auto const line1Product = fge::Cross2d(relativePos, line1.getDirection());
    auto const line2Product = fge::Cross2d(relativePos, line2.getDirection());

    return line1Product < 0.0f && line2Product > 0.0f;
}

///Position/Rectangle
template<typename T>
fge::Rect<T> ToRect(fge::Vector2<T> const& pos1, fge::Vector2<T> const& pos2)
{
    return fge::Rect<T>({(pos1.x < pos2.x) ? pos1.x : pos2.x, (pos1.y < pos2.y) ? pos1.y : pos2.y},
                        {(pos1.x > pos2.x) ? (pos1.x - pos2.x) : (pos2.x - pos1.x),
                         (pos1.y > pos2.y) ? (pos1.y - pos2.y) : (pos2.y - pos1.y)});
}
template<typename T>
fge::Rect<T> ToRect(std::vector<fge::Vector2<T>> const& pos)
{
    float smallestX = std::numeric_limits<float>::max();
    float biggestX = std::numeric_limits<float>::lowest();
    float smallestY = std::numeric_limits<float>::max();
    float biggestY = std::numeric_limits<float>::lowest();

    for (std::size_t i = 0; i < pos.size(); ++i)
    {
        if (pos[i].x < smallestX)
        {
            smallestX = pos[i].x;
        }
        if (pos[i].x > biggestX)
        {
            biggestX = pos[i].x;
        }
        if (pos[i].y < smallestY)
        {
            smallestY = pos[i].y;
        }
        if (pos[i].y > biggestY)
        {
            biggestY = pos[i].y;
        }
    }

    return fge::Rect<T>({smallestX, smallestY}, {biggestX - smallestX, biggestY - smallestY});
}
template<typename T>
fge::Rect<T> ToRect(fge::Vector2<T> const* pos, std::size_t size)
{
    T smallestX = std::numeric_limits<T>::max();
    T biggestX = std::numeric_limits<T>::lowest();
    T smallestY = std::numeric_limits<T>::max();
    T biggestY = std::numeric_limits<T>::lowest();

    for (std::size_t i = 0; i < size; ++i)
    {
        if (pos[i].x < smallestX)
        {
            smallestX = pos[i].x;
        }
        if (pos[i].x > biggestX)
        {
            biggestX = pos[i].x;
        }
        if (pos[i].y < smallestY)
        {
            smallestY = pos[i].y;
        }
        if (pos[i].y > biggestY)
        {
            biggestY = pos[i].y;
        }
    }

    return fge::Rect<T>({smallestX, smallestY}, {biggestX - smallestX, biggestY - smallestY});
}

///Reach
template<typename T>
T ReachValue(T value, T target, T speed, float deltaTime)
{
    float travelDistance = static_cast<float>(speed) * deltaTime;
    float direction = (static_cast<float>(target) - static_cast<float>(value)) /
                      std::abs(static_cast<float>(target) - static_cast<float>(value));
    T actualDistance = std::abs(target - value);

    if (travelDistance >= actualDistance)
    { //We already reached the target
        return target;
    }
    return static_cast<T>(static_cast<float>(value) + direction * travelDistance);
}

///2D Math
inline constexpr float Cross2d(fge::Vector2f const& vec1, fge::Vector2f const& vec2)
{
    return glm::cross(fge::Vector3f{vec1, 0.0f}, fge::Vector3f{vec2, 0.0f}).z;
}
inline fge::Vector2f GetSegmentNormal(fge::Vector2f const& vec1, fge::Vector2f const& vec2)
{
    return glm::normalize(fge::Vector2f{vec1.y - vec2.y, vec2.x - vec1.x});
}
inline constexpr float GetAngle(fge::Vector2f const& vec)
{
    return glm::degrees(std::atan2(vec.y, vec.x));
}
inline constexpr float GetAngleBetween(fge::Vector2f const& vec1, fge::Vector2f const& vec2)
{
    return glm::degrees(std::atan2(fge::Cross2d(vec1, vec2), glm::dot(vec1, vec2)));
}
inline float GetDistanceBetween(fge::Vector2f const& vec1, fge::Vector2f const& vec2)
{
    return glm::length(vec2 - vec1);
}
inline float
GetShortestDistanceBetween(fge::Vector2f const& point, fge::Vector2f const& lineStart, fge::Vector2f const& lineEnd)
{
    auto normalDir = glm::normalize(lineEnd - lineStart);
    return std::abs((normalDir.y * (point.x - lineStart.x) - normalDir.x * (point.y - lineStart.y)) /
                    (normalDir.x * normalDir.x + normalDir.y * normalDir.y));
}

template<typename TIterator>
TIterator GetNearestPoint(fge::Vector2f const& point, TIterator const& pointsBegin, TIterator const& pointsEnd)
{
    TIterator bestNearestPoint = pointsEnd;
    float bestNearestDistance;

    for (TIterator it = pointsBegin; it != pointsEnd; ++it)
    {
        float distance = fge::GetDistanceBetween(point, *it);
        if (bestNearestPoint == pointsEnd)
        {
            bestNearestPoint = it;
            bestNearestDistance = distance;
        }
        else
        {
            if (distance < bestNearestDistance)
            {
                bestNearestPoint = it;
                bestNearestDistance = distance;
            }
        }
    }

    return bestNearestPoint;
}

inline constexpr fge::Vector2f GetForwardVector(float angle)
{
    angle = glm::radians(angle);
    return {std::cos(angle), std::sin(angle)};
}
inline constexpr fge::Vector2f GetBackwardVector(float angle)
{
    angle = glm::radians(angle);
    return -fge::Vector2f{std::cos(angle), std::sin(angle)};
}
inline constexpr fge::Vector2f GetLeftVector(float angle)
{
    angle = glm::radians(angle - 90.0f);
    return {std::cos(angle), std::sin(angle)};
}
inline constexpr fge::Vector2f GetRightVector(float angle)
{
    angle = glm::radians(angle + 90.0f);
    return {std::cos(angle), std::sin(angle)};
}

inline constexpr float DotSquare(fge::Vector2f const& vec)
{
    return glm::dot(vec, vec);
}

inline constexpr float GetHandedness(fge::Vector2f const& vec1, fge::Vector2f const& vec2, fge::Vector2f const& vec3)
{
    auto const edge1 = vec2 - vec1;
    auto const edge2 = vec3 - vec2;

    return fge::Cross2d(edge1, edge2);
}

///Color
inline fge::Color SetAlpha(fge::Color color, uint8_t alpha)
{
    color._a = alpha;
    return color;
}
inline fge::Color SetRed(fge::Color color, uint8_t red)
{
    color._r = red;
    return color;
}
inline fge::Color SetGreen(fge::Color color, uint8_t green)
{
    color._g = green;
    return color;
}
inline fge::Color SetBlue(fge::Color color, uint8_t blue)
{
    color._b = blue;
    return color;
}

///Time
template<class T>
float DurationToSecondFloat(T duration)
{
    return std::chrono::duration<float, std::ratio<1, 1>>(duration).count();
}

} // namespace fge
