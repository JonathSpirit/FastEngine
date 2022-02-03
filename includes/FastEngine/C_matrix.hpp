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

    Matrix(const sf::Vector2<std::size_t>& msize);
    Matrix(std::size_t sizex, std::size_t sizey);

    Matrix(const sf::Vector2<std::size_t>& msize, const T& defaultValue);
    Matrix(std::size_t sizex, std::size_t sizey, const T& defaultValue);

    Matrix(const fge::Matrix<T>& m) = default;
    Matrix(fge::Matrix<T>&& m);

    ~Matrix() = default;

    void clear();

    inline operator std::vector<std::vector<T> >&();
    inline operator const std::vector<std::vector<T> >&() const;

    fge::Matrix<T>& operator =(const fge::Matrix<T>& m) = default;
    fge::Matrix<T>& operator =(fge::Matrix<T>&& m);

    inline std::vector<T>& operator[](std::size_t x);
    inline const std::vector<T>& operator[](std::size_t x) const;

    inline typename std::vector<T>::const_reference get(std::size_t x, std::size_t y) const;
    inline typename std::vector<T>::const_reference get(const sf::Vector2<std::size_t>& coord) const;
    inline typename std::vector<T>::reference get(std::size_t x, std::size_t y);
    inline typename std::vector<T>::reference get(const sf::Vector2<std::size_t>& coord);

    bool get(std::size_t x, std::size_t y, T& buff) const;
    bool get(const sf::Vector2<std::size_t>& coord, T& buff) const;

    T* getPtr(std::size_t x, std::size_t y);
    T* getPtr(const sf::Vector2<std::size_t>& coord);
    const T* getPtr(std::size_t x, std::size_t y) const;
    const T* getPtr(const sf::Vector2<std::size_t>& coord) const;

    inline std::vector<std::vector<T> >& get();
    inline const std::vector<std::vector<T> >& get() const;

    void set(std::size_t x, std::size_t y, const T&& value);
    void set(const sf::Vector2<std::size_t>& coord, const T&& value);
    void set(std::size_t x, std::size_t y, T&& value);
    void set(const sf::Vector2<std::size_t>& coord, T&& value);
    void set(std::size_t x, std::size_t y, const T& value);
    void set(const sf::Vector2<std::size_t>& coord, const T& value);
    void set(std::size_t x, std::size_t y, T& value);
    void set(const sf::Vector2<std::size_t>& coord, T& value);

    void set(std::initializer_list<std::initializer_list<T>> data);

    inline std::size_t getTotalSize() const;
    inline const sf::Vector2<std::size_t>& getSize() const;
    inline std::size_t getSizeX() const;
    inline std::size_t getSizeY() const;

    void setSize(const sf::Vector2<std::size_t>& msize);
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
