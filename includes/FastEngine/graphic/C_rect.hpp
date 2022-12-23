#ifndef _FGE_C_RECT_HPP_INCLUDED
#define _FGE_C_RECT_HPP_INCLUDED

#include <FastEngine/graphic/C_vector.hpp>
#include <optional>
#include <type_traits>

namespace fge
{

template <class T>
class Rect
{
    static_assert(std::is_arithmetic_v<T>, "T must be arithmetic !");
public:
    Rect();
    Rect(const Vector2<T>& position, const Vector2<T>& size);

    template <class U>
    explicit Rect(const Rect<U>& rectangle);

    [[nodiscard]] bool operator==(const Rect<T>& right) const;
    [[nodiscard]] bool operator!=(const Rect<T>& right) const;

    [[nodiscard]] bool contains(const Vector2<T>& point) const;
    [[nodiscard]] std::optional<Rect<T>> findIntersection(const Rect<T>& rectangle) const;

    [[nodiscard]] Vector2<T> getPosition() const;
    [[nodiscard]] Vector2<T> getSize() const;

    T _x;
    T _y;
    T _width;
    T _height;
};

using RectInt = Rect<int32_t>;
using RectUint = Rect<uint32_t>;
using RectFloat = Rect<float>;

fge::RectFloat operator*(const glm::mat4& left, const fge::RectFloat& right);

}//end fge

#include <FastEngine/graphic/C_rect.inl>

#endif //_FGE_C_RECT_HPP_INCLUDED