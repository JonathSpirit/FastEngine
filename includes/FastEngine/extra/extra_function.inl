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

namespace fge
{

///Utility
char UnicodeToChar(uint32_t unicode)
{
    return (unicode < 128) ? static_cast<char>(unicode) : 0;
}

///Position/Rectangle
template<typename T>
fge::Rect<T> ToRect(const fge::Vector2<T>& pos1, const fge::Vector2<T>& pos2)
{
    return fge::Rect<T>({(pos1.x < pos2.x) ? pos1.x : pos2.x, (pos1.y < pos2.y) ? pos1.y : pos2.y},
                        {(pos1.x > pos2.x) ? (pos1.x - pos2.x) : (pos2.x - pos1.x),
                         (pos1.y > pos2.y) ? (pos1.y - pos2.y) : (pos2.y - pos1.y)});
}
template<typename T>
fge::Rect<T> ToRect(const std::vector<fge::Vector2<T>>& pos)
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
fge::Rect<T> ToRect(const fge::Vector2<T>* pos, std::size_t size)
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
template<typename T>
fge::Vector2f NormalizeVector2(const fge::Vector2<T>& vec)
{
    return vec / std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

template<typename TIterator>
TIterator GetNearestVector(const fge::Vector2f& vec, const TIterator& pointsBegin, const TIterator& pointsEnd)
{
    TIterator bestNearestPoint = pointsEnd;
    float bestNearestDistance;

    for (TIterator it = pointsBegin; it != pointsEnd; ++it)
    {
        float distance = fge::GetDistanceBetween(vec, *it);
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

///Color
fge::Color SetAlpha(const fge::Color& color, uint8_t alpha)
{
    fge::Color buff(color);
    buff._a = alpha;
    return buff;
}
fge::Color SetRed(const fge::Color& color, uint8_t red)
{
    fge::Color buff(color);
    buff._r = red;
    return buff;
}
fge::Color SetGreen(const fge::Color& color, uint8_t green)
{
    fge::Color buff(color);
    buff._g = green;
    return buff;
}
fge::Color SetBlue(const fge::Color& color, uint8_t blue)
{
    fge::Color buff(color);
    buff._b = blue;
    return buff;
}

fge::Color&& SetAlpha(fge::Color&& color, uint8_t alpha)
{
    color._a = alpha;
    return std::move(color);
}
fge::Color&& SetRed(fge::Color&& color, uint8_t red)
{
    color._r = red;
    return std::move(color);
}
fge::Color&& SetGreen(fge::Color&& color, uint8_t green)
{
    color._g = green;
    return std::move(color);
}
fge::Color&& SetBlue(fge::Color&& color, uint8_t blue)
{
    color._b = blue;
    return std::move(color);
}

///Time
template<class T>
float DurationToSecondFloat(T duration)
{
    return std::chrono::duration<float, std::ratio<1, 1>>(duration).count();
}

} // namespace fge