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

template<class T>
Rect<T>::Rect() :
        _x(0),
        _y(0),
        _width(0),
        _height(0)
{}

template<class T>
Rect<T>::Rect(const Vector2<T>& position, const Vector2<T>& size) :
        _x(position.x),
        _y(position.y),
        _width(size.x),
        _height(size.y)
{}

template<class T>
template<class U>
Rect<T>::Rect(const Rect<U>& rectangle) :
        _x(static_cast<T>(rectangle._x)),
        _y(static_cast<T>(rectangle._y)),
        _width(static_cast<T>(rectangle._width)),
        _height(static_cast<T>(rectangle._height))
{}

template<class T>
bool Rect<T>::operator==(const Rect<T>& right) const
{
    return (this->_x == right._x) && (this->_width == right._width) &&
            (this->_y == right._y) && (this->_height == right._height);
}

template<class T>
bool Rect<T>::operator!=(const Rect<T>& right) const
{
    return !this->operator==(right);
}

template<class T>
bool Rect<T>::contains(const Vector2<T>& point) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly
    const T farX = static_cast<T>(this->_x + this->_width);
    const T farY = static_cast<T>(this->_y + this->_height);

    // Compute the real min and max of the rectangle on both axes
    const T minX = (this->_x < farX) ? this->_x : farX;
    const T maxX = (this->_x > farX) ? this->_x : farX;
    const T minY = (this->_y < farY) ? this->_y : farY;
    const T maxY = (this->_y > farY) ? this->_y : farY;

    return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
}

template<class T>
std::optional<Rect<T>> Rect<T>::findIntersection(const Rect<T>& rectangle) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly
    const T r1FarX = static_cast<T>(this->_x + this->_width);
    const T r1FarY = static_cast<T>(this->_y + this->_height);

    const T r2FarX = static_cast<T>(rectangle._x + rectangle._width);
    const T r2FarY = static_cast<T>(rectangle._y + rectangle._height);

    // Compute the min and max of the first rectangle on both axes
    const T r1MinX = (this->_x < r1FarX) ? this->_x : r1FarX;
    const T r1MaxX = (this->_x > r1FarX) ? this->_x : r1FarX;
    const T r1MinY = (this->_y < r1FarY) ? this->_y : r1FarY;
    const T r1MaxY = (this->_y > r1FarY) ? this->_y : r1FarY;

    // Compute the min and max of the second rectangle on both axes
    const T r2MinX = (rectangle._x < r2FarX) ? rectangle._x : r2FarX;
    const T r2MaxX = (rectangle._x > r2FarX) ? rectangle._x : r2FarX;
    const T r2MinY = (rectangle._y < r2FarY) ? rectangle._y : r2FarY;
    const T r2MaxY = (rectangle._y > r2FarY) ? rectangle._y : r2FarY;

    // Compute the intersection boundaries
    const T interLeft = (r1MinX > r2MinX) ? r1MinX : r2MinX;
    const T interTop = (r1MinY > r2MinY) ? r1MinY : r2MinY;
    const T interRight = (r1MaxX < r2MaxX) ? r1MaxX : r2MaxX;
    const T interBottom = (r1MaxY < r2MaxY) ? r1MaxY : r2MaxY;

    // If the intersection is valid (positive non-zero area), then there is an intersection
    if ((interLeft < interRight) && (interTop < interBottom))
    {
        return Rect<T>({interLeft, interTop}, {interRight - interLeft, interBottom - interTop});
    }
    return std::nullopt;
}

template <class T>
Vector2<T> Rect<T>::getPosition() const
{
    return Vector2<T>(this->_x, this->_y);
}

template <class T>
Vector2<T> Rect<T>::getSize() const
{
    return Vector2<T>(this->_width, this->_height);
}

inline fge::RectFloat operator*(const glm::mat4& left, const fge::RectFloat& right)
{
    // Transform the 4 corners of the rectangle
    const fge::Vector2f points[] =
    {
        left * glm::vec4(right._x, right._y, 0.0f, 0.0f),
        left * glm::vec4(right._x, right._y + right._height, 0.0f, 0.0f),
        left * glm::vec4(right._x + right._width, right._y, 0.0f, 0.0f),
        left * glm::vec4(right._x + right._width, right._y + right._height, 0.0f, 0.0f)
    };

    // Compute the bounding rectangle of the transformed points
    float posLeft = points[0].x;
    float posTop = points[0].y;
    float posRight = points[0].x;
    float posBottom = points[0].y;
    for (int i = 1; i < 4; ++i)
    {
        if      (points[i].x < posLeft)   {posLeft = points[i].x;}
        else if (points[i].x > posRight)  {posRight = points[i].x;}
        if      (points[i].y < posTop)    {posTop = points[i].y;}
        else if (points[i].y > posBottom) {posBottom = points[i].y;}
    }

    return fge::RectFloat({posLeft, posTop}, {posRight - posLeft, posBottom - posTop});
}

}//end fge