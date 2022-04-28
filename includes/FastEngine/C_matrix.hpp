#ifndef _FGE_C_MATRIX_HPP_INCLUDED
#define _FGE_C_MATRIX_HPP_INCLUDED

#include <SFML/System.hpp>
#include <vector>
#include <initializer_list>
#include <json.hpp>

namespace fge
{

template<class T>
class Matrix
{
public:
    Matrix();

    Matrix(std::initializer_list<std::initializer_list<T>> data);

    template<class Tvec>
    explicit Matrix(const sf::Vector2<Tvec>& msize);
    Matrix(std::size_t sizex, std::size_t sizey);

    template<class Tvec>
    Matrix(const sf::Vector2<Tvec>& msize, const T& defaultValue);
    Matrix(std::size_t sizex, std::size_t sizey, const T& defaultValue);

    Matrix(const fge::Matrix<T>& m) = default;
    Matrix(fge::Matrix<T>&& m) noexcept;

    ~Matrix() = default;

    void clear();

    inline explicit operator std::vector<std::vector<T> >&();
    inline explicit operator const std::vector<std::vector<T> >&() const;

    fge::Matrix<T>& operator =(const fge::Matrix<T>& m) = default;
    fge::Matrix<T>& operator =(fge::Matrix<T>&& m) noexcept;

    inline std::vector<T>& operator[](std::size_t x);
    inline const std::vector<T>& operator[](std::size_t x) const;

    inline typename std::vector<T>::const_reference get(std::size_t x, std::size_t y) const;
    template<class Tvec>
    inline typename std::vector<T>::const_reference get(const sf::Vector2<Tvec>& coord) const;
    inline typename std::vector<T>::reference get(std::size_t x, std::size_t y);
    template<class Tvec>
    inline typename std::vector<T>::reference get(const sf::Vector2<Tvec>& coord);

    bool get(std::size_t x, std::size_t y, T& buff) const;
    template<class Tvec>
    bool get(const sf::Vector2<Tvec>& coord, T& buff) const;

    T* getPtr(std::size_t x, std::size_t y);
    template<class Tvec>
    T* getPtr(const sf::Vector2<Tvec>& coord);
    const T* getPtr(std::size_t x, std::size_t y) const;
    template<class Tvec>
    const T* getPtr(const sf::Vector2<Tvec>& coord) const;

    inline std::vector<std::vector<T> >& get();
    inline const std::vector<std::vector<T> >& get() const;

    void set(std::size_t x, std::size_t y, T&& value);
    template<class Tvec>
    void set(const sf::Vector2<Tvec>& coord, T&& value);
    void set(std::size_t x, std::size_t y, const T& value);
    template<class Tvec>
    void set(const sf::Vector2<Tvec>& coord, const T& value);

    void set(std::initializer_list<std::initializer_list<T>> data);

    [[nodiscard]] inline std::size_t getTotalSize() const;
    [[nodiscard]] inline const sf::Vector2<std::size_t>& getSize() const;
    [[nodiscard]] inline std::size_t getSizeX() const;
    [[nodiscard]] inline std::size_t getSizeY() const;

    template<class Tvec>
    void setSize(const sf::Vector2<Tvec>& msize);
    void setSize(std::size_t sizex, std::size_t sizey);

    void fill(const T& value);

    void rotateClockwise();
    void rotateCounterClockwise();

    void toVector(std::vector<T>& buff) const;

private:
    std::vector<std::vector<T> > g_matrix;
    sf::Vector2<std::size_t> g_msize;
};

template<class T>
void to_json(nlohmann::json& j, const fge::Matrix<T>& r);
template<class T>
void from_json(const nlohmann::json& j, fge::Matrix<T>& r);

}//end fge

#include <FastEngine/C_matrix.inl>

#endif // _FGE_C_MATRIX_HPP_INCLUDED
