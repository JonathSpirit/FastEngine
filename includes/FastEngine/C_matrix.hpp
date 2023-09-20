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

#ifndef _FGE_C_MATRIX_HPP_INCLUDED
#define _FGE_C_MATRIX_HPP_INCLUDED

#include "FastEngine/C_vector.hpp"
#include "FastEngine/fge_except.hpp"
#include "json.hpp"
#include <initializer_list>

#define FGE_MATRIX_GET(dataType_, data_, sizeY_, px_, py_)                                                             \
    (reinterpret_cast<dataType_*>(data_) + (py_) + ((px_) * (sizeY_)))

namespace fge
{

/**
 * \class Matrix
 * \ingroup utility
 * \brief A container to store a 2D matrix of any type
 *
 * \tparam T The type of the matrix
 */
template<class T>
class Matrix
{
public:
    using iterator = T*;
    using const_iterator = T const*;

    /**
     * \brief Construct a empty matrix
     */
    Matrix();

    /**
     * \brief Construct a matrix with a initializer list
     *
     * Make sure that the number of elements in a row is the same to all other rows.
     *
     * \param data The initializer list
     */
    Matrix(std::initializer_list<std::initializer_list<T>> data);

    /**
     * \brief Construct a matrix with a certain size
     *
     * \tparam Tvec The type of the vector
     * \param msize The size of the matrix
     */
    template<class Tvec>
    explicit Matrix(fge::Vector2<Tvec> const& msize);
    /**
     * \brief Construct a matrix with a certain size
     *
     * \param sizex The size of the matrix on the x axis
     * \param sizey The size of the matrix on the y axis
     */
    Matrix(std::size_t sizex, std::size_t sizey);

    /**
     * \brief Construct a matrix with a certain size and a default value
     *
     * \tparam Tvec The type of the vector
     * \param msize The size of the matrix
     * \param defaultValue The value to fill the matrix with
     */
    template<class Tvec>
    Matrix(fge::Vector2<Tvec> const& msize, T const& defaultValue);
    /**
     * \brief Construct a matrix with a certain size and a default value
     *
     * \param sizex The size of the matrix on the x axis
     * \param sizey The size of the matrix on the y axis
     * \param defaultValue The value to fill the matrix with
     */
    Matrix(std::size_t sizex, std::size_t sizey, T const& defaultValue);

    Matrix(fge::Matrix<T> const& m);
    Matrix(fge::Matrix<T>&& m) noexcept;

    ~Matrix() = default;

    /**
     * \brief Clear the matrix and set the size to 0,0
     */
    void clear();

    fge::Matrix<T>& operator=(fge::Matrix<T> const& m);
    fge::Matrix<T>& operator=(fge::Matrix<T>&& m) noexcept;

    /**
     * \brief Get the specified row
     *
     * \param x The index of the row
     * \return The row as a vector
     */
    inline T* operator[](std::size_t x);
    inline T const* operator[](std::size_t x) const;

    /**
     * \brief Get the specified value
     *
     * \param x The x index of the value
     * \param y The y index of the value
     * \return A reference to the value
     */
    inline T const& get(std::size_t x, std::size_t y) const;
    /**
     * \brief Get the specified value
     *
     * \tparam Tvec The type of the vector
     * \param coord The coordinates of the value
     * \return A reference to the value
     */
    template<class Tvec>
    inline T const& get(fge::Vector2<Tvec> const& coord) const;
    inline T& get(std::size_t x, std::size_t y);
    template<class Tvec>
    inline T& get(fge::Vector2<Tvec> const& coord);

    /**
     * \brief Get the specified value without throwing an exception
     *
     * \param x The x index of the value
     * \param y The y index of the value
     * \param buff The value to store the result in
     * \return \b true if the value was found, \b false otherwise
     */
    bool get(std::size_t x, std::size_t y, T& buff) const;
    /**
     * \brief Get the specified value without throwing an exception
     *
     * \tparam Tvec The type of the vector
     * \param coord The coordinates of the value
     * \param buff The value to store the result in
     * \return \b true if the value was found, \b false otherwise
     */
    template<class Tvec>
    bool get(fge::Vector2<Tvec> const& coord, T& buff) const;

    /**
     * \brief Get the specified value as a pointer
     *
     * \param x The x index of the value
     * \param y The y index of the value
     * \return A pointer to the value or \b nullptr if the value was not found
     */
    T* getPtr(std::size_t x, std::size_t y);
    /**
     * \brief Get the specified value as a pointer
     *
     * \tparam Tvec The type of the vector
     * \param coord The coordinates of the value
     * \return A pointer to the value or \b nullptr if the value was not found
     */
    template<class Tvec>
    T* getPtr(fge::Vector2<Tvec> const& coord);
    T const* getPtr(std::size_t x, std::size_t y) const;
    template<class Tvec>
    T const* getPtr(fge::Vector2<Tvec> const& coord) const;

    /**
     * \brief Set the specified value by moving it
     *
     * \param x The x index of the value
     * \param y The y index of the value
     * \param value The value to move
     */
    void set(std::size_t x, std::size_t y, T&& value);
    /**
     * \brief Set the specified value by moving it
     *
     * \tparam Tvec The type of the vector
     * \param coord The coordinates of the value
     * \param value The value to move
     */
    template<class Tvec>
    void set(fge::Vector2<Tvec> const& coord, T&& value);
    /**
     * \brief Set the specified value by copying it
     *
     * \param x The x index of the value
     * \param y The y index of the value
     * \param value The value to copy
     */
    void set(std::size_t x, std::size_t y, T const& value);
    /**
     * \brief Set the specified value by copying it
     *
     * \tparam Tvec The type of the vector
     * \param coord The coordinates of the value
     * \param value The value to copy
     */
    template<class Tvec>
    void set(fge::Vector2<Tvec> const& coord, T const& value);

    /**
     * \brief Set values with a initializer list
     *
     * The size of the matrix will be set to the size of the provided list.
     *
     * \param data The initializer list
     */
    void set(std::initializer_list<std::initializer_list<T>> data);

    /**
     * \brief Get the total number of elements in the matrix
     *
     * \return The total number of elements
     */
    [[nodiscard]] inline std::size_t getTotalSize() const;
    /**
     * \brief Get the size of the matrix as a vector2
     *
     * \return The size of the matrix as a vector2
     */
    [[nodiscard]] inline fge::Vector2<std::size_t> const& getSize() const;
    /**
     * \brief Get the x size of the matrix
     *
     * \return The x size of the matrix
     */
    [[nodiscard]] inline std::size_t getSizeX() const;
    /**
     * \brief Get the y size of the matrix
     *
     * \return The y size of the matrix
     */
    [[nodiscard]] inline std::size_t getSizeY() const;

    /**
     * \brief Get the 2D array of the matrix
     *
     * \return The 2D array as a pointer of the first element
     */
    [[nodiscard]] inline T const* get() const;
    [[nodiscard]] inline T* get();

    [[nodiscard]] fge::Matrix<T>::iterator begin();
    [[nodiscard]] fge::Matrix<T>::iterator end();
    [[nodiscard]] fge::Matrix<T>::const_iterator begin() const;
    [[nodiscard]] fge::Matrix<T>::const_iterator end() const;

    /**
     * \brief Set the size of the matrix
     *
     * \tparam Tvec The type of the vector
     * \param msize The size of the matrix
     */
    template<class Tvec>
    void setSize(fge::Vector2<Tvec> const& msize);
    /**
     * \brief Set the size of the matrix
     *
     * \param sizex The x size of the matrix
     * \param sizey The y size of the matrix
     */
    void setSize(std::size_t sizex, std::size_t sizey);

    /**
     * \brief Fill the matrix by copying a value
     *
     * \param value The value to copy
     */
    void fill(T const& value);

    /**
     * \brief Rotate the matrix by 90 degrees clockwise
     */
    void rotateClockwise();
    /**
     * \brief Rotate the matrix by 90 degrees counter-clockwise
     */
    void rotateCounterClockwise();
    /**
     * \brief Rotate the matrix by 90 degrees clockwise and n times
     *
     * \param n The number of times to rotate
     */
    void rotateClockwise(unsigned int n);
    /**
     * \brief Rotate the matrix by 90 degrees counter-clockwise and n times
     *
     * \param n The number of times to rotate
     */
    void rotateCounterClockwise(unsigned int n);
    /**
     * \brief Flip the matrix horizontally
     */
    void flipHorizontally();
    /**
     * \brief Flip the matrix vertically
     */
    void flipVertically();

    /**
     * \brief Insert all elements of the matrix in a 1D vector
     *
     * \param buff The vector to insert the elements in
     */
    void toVector(std::vector<T>& buff) const;

private:
    std::unique_ptr<T[]> g_mdata;
    fge::Vector2<std::size_t> g_msize;
};

/**
 * \brief json function to save a matrix
 *
 * \tparam T The type of the matrix
 * \param j The json object to save the matrix in
 * \param r The matrix to save
 */
template<class T>
void to_json(nlohmann::json& j, fge::Matrix<T> const& r);
/**
 * \brief json function to load a matrix
 *
 * \tparam T The type of the matrix
 * \param j The json object to load the matrix from
 * \param r The matrix to load
 */
template<class T>
void from_json(nlohmann::json const& j, fge::Matrix<T>& r);

} // namespace fge

#include <FastEngine/C_matrix.inl>

#endif // _FGE_C_MATRIX_HPP_INCLUDED
