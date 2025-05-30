/*
 * Copyright 2025 Guillaume Guillet
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
Rect<T>::Rect(Vector2<T> const& position, Vector2<T> const& size) :
        _x(position.x),
        _y(position.y),
        _width(size.x),
        _height(size.y)
{}

template<class T>
template<class U>
Rect<T>::Rect(Rect<U> const& rectangle) :
        _x(static_cast<T>(rectangle._x)),
        _y(static_cast<T>(rectangle._y)),
        _width(static_cast<T>(rectangle._width)),
        _height(static_cast<T>(rectangle._height))
{}

template<class T>
bool Rect<T>::operator==(Rect<T> const& right) const
{
    return (this->_x == right._x) && (this->_width == right._width) && (this->_y == right._y) &&
           (this->_height == right._height);
}

template<class T>
bool Rect<T>::operator!=(Rect<T> const& right) const
{
    return !this->operator==(right);
}

template<class T>
bool Rect<T>::contains(Vector2<T> const& point) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly
    T const farX = static_cast<T>(this->_x + this->_width);
    T const farY = static_cast<T>(this->_y + this->_height);

    // Compute the real min and max of the rectangle on both axes
    T const minX = (this->_x < farX) ? this->_x : farX;
    T const maxX = (this->_x > farX) ? this->_x : farX;
    T const minY = (this->_y < farY) ? this->_y : farY;
    T const maxY = (this->_y > farY) ? this->_y : farY;

    return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
}
template<class T>
bool Rect<T>::contains(Rect<T> const& rectangle) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly
    T const r1FarX = static_cast<T>(this->_x + this->_width);
    T const r1FarY = static_cast<T>(this->_y + this->_height);

    T const r2FarX = static_cast<T>(rectangle._x + rectangle._width);
    T const r2FarY = static_cast<T>(rectangle._y + rectangle._height);

    // Compute the min and max of the first rectangle on both axes
    T const r1MinX = (this->_x < r1FarX) ? this->_x : r1FarX;
    T const r1MaxX = (this->_x > r1FarX) ? this->_x : r1FarX;
    T const r1MinY = (this->_y < r1FarY) ? this->_y : r1FarY;
    T const r1MaxY = (this->_y > r1FarY) ? this->_y : r1FarY;

    // Compute the min and max of the second rectangle on both axes
    T const r2MinX = (rectangle._x < r2FarX) ? rectangle._x : r2FarX;
    T const r2MaxX = (rectangle._x > r2FarX) ? rectangle._x : r2FarX;
    T const r2MinY = (rectangle._y < r2FarY) ? rectangle._y : r2FarY;
    T const r2MaxY = (rectangle._y > r2FarY) ? rectangle._y : r2FarY;

    return (r1MinX <= r2MinX) && (r1MaxX >= r2MaxX) && (r1MinY <= r2MinY) && (r1MaxY >= r2MaxY);
}

template<class T>
std::optional<Rect<T>> Rect<T>::findIntersection(Rect<T> const& rectangle) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly
    T const r1FarX = static_cast<T>(this->_x + this->_width);
    T const r1FarY = static_cast<T>(this->_y + this->_height);

    T const r2FarX = static_cast<T>(rectangle._x + rectangle._width);
    T const r2FarY = static_cast<T>(rectangle._y + rectangle._height);

    // Compute the min and max of the first rectangle on both axes
    T const r1MinX = (this->_x < r1FarX) ? this->_x : r1FarX;
    T const r1MaxX = (this->_x > r1FarX) ? this->_x : r1FarX;
    T const r1MinY = (this->_y < r1FarY) ? this->_y : r1FarY;
    T const r1MaxY = (this->_y > r1FarY) ? this->_y : r1FarY;

    // Compute the min and max of the second rectangle on both axes
    T const r2MinX = (rectangle._x < r2FarX) ? rectangle._x : r2FarX;
    T const r2MaxX = (rectangle._x > r2FarX) ? rectangle._x : r2FarX;
    T const r2MinY = (rectangle._y < r2FarY) ? rectangle._y : r2FarY;
    T const r2MaxY = (rectangle._y > r2FarY) ? rectangle._y : r2FarY;

    // Compute the intersection boundaries
    T const interLeft = (r1MinX > r2MinX) ? r1MinX : r2MinX;
    T const interTop = (r1MinY > r2MinY) ? r1MinY : r2MinY;
    T const interRight = (r1MaxX < r2MaxX) ? r1MaxX : r2MaxX;
    T const interBottom = (r1MaxY < r2MaxY) ? r1MaxY : r2MaxY;

    // If the intersection is valid (positive non-zero area), then there is an intersection
    if ((interLeft < interRight) && (interTop < interBottom))
    {
        return Rect<T>({interLeft, interTop}, {interRight - interLeft, interBottom - interTop});
    }
    return std::nullopt;
}

template<class T>
Vector2<T> Rect<T>::getPosition() const
{
    return Vector2<T>(this->_x, this->_y);
}

template<class T>
Vector2<T> Rect<T>::getSize() const
{
    return Vector2<T>(this->_width, this->_height);
}

inline fge::RectFloat operator*(glm::mat4 const& left, fge::RectFloat const& right)
{
    // Transform the 4 corners of the rectangle
    fge::Vector2f const points[] = {left * glm::vec4(right._x, right._y, 0.0f, 1.0f),
                                    left * glm::vec4(right._x, right._y + right._height, 0.0f, 1.0f),
                                    left * glm::vec4(right._x + right._width, right._y, 0.0f, 1.0f),
                                    left * glm::vec4(right._x + right._width, right._y + right._height, 0.0f, 1.0f)};

    // Compute the bounding rectangle of the transformed points
    float posLeft = points[0].x;
    float posTop = points[0].y;
    float posRight = points[0].x;
    float posBottom = points[0].y;
    for (int i = 1; i < 4; ++i)
    {
        if (points[i].x < posLeft)
        {
            posLeft = points[i].x;
        }
        else if (points[i].x > posRight)
        {
            posRight = points[i].x;
        }
        if (points[i].y < posTop)
        {
            posTop = points[i].y;
        }
        else if (points[i].y > posBottom)
        {
            posBottom = points[i].y;
        }
    }

    return fge::RectFloat({posLeft, posTop}, {posRight - posLeft, posBottom - posTop});
}

} // namespace fge