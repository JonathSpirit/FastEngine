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
Matrix<T>::Matrix() :
    g_msize(0,0)
{
}

template<class T>
Matrix<T>::Matrix(std::initializer_list<std::initializer_list<T>> data)
{
    this->set(data);
}

template<class T>
template<class Tvec>
Matrix<T>::Matrix(const sf::Vector2<Tvec>& msize)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    this->setSize( static_cast<sf::Vector2<std::size_t>>(msize) );
}
template<class T>
Matrix<T>::Matrix(std::size_t sizex, std::size_t sizey)
{
    this->setSize(sizex, sizey);
}

template<class T>
template<class Tvec>
Matrix<T>::Matrix(const sf::Vector2<Tvec>& msize, const T& defaultValue)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    this->setSize( static_cast<sf::Vector2<std::size_t>>(msize) );
    this->fill(defaultValue);
}
template<class T>
Matrix<T>::Matrix(std::size_t sizex, std::size_t sizey, const T& defaultValue)
{
    this->setSize(sizex, sizey);
    this->fill(defaultValue);
}

template<class T>
Matrix<T>::Matrix(fge::Matrix<T>&& m) noexcept :
    g_msize(m.g_msize),
    g_matrix( std::move(m.g_matrix) )
{
    m.g_msize.y = 0;
    m.g_msize.x = 0;
}

template<class T>
void Matrix<T>::clear()
{
    this->g_matrix.clear();
    this->g_msize.x = 0;
    this->g_msize.y = 0;
}

template<class T>
Matrix<T>::operator std::vector<std::vector<T> >&()
{
    return this->g_matrix;
}
template<class T>
Matrix<T>::operator const std::vector<std::vector<T> >&() const
{
    return this->g_matrix;
}

template<class T>
fge::Matrix<T>& Matrix<T>::operator =(fge::Matrix<T>&& m) noexcept
{
    this->g_msize = m.g_msize;
    m.g_msize.y = 0;
    m.g_msize.x = 0;
    this->g_matrix = std::move(m.g_matrix);
    return *this;
}

template<class T>
std::vector<T>& Matrix<T>::operator[](std::size_t x)
{
    return this->g_matrix[x];
}
template<class T>
const std::vector<T>& Matrix<T>::operator[](std::size_t x) const
{
    return this->g_matrix[x];
}

template<class T>
typename std::vector<T>::const_reference Matrix<T>::get(std::size_t x, std::size_t y) const
{
    return this->g_matrix.at(x).at(y);
}
template<class T>
template<class Tvec>
typename std::vector<T>::const_reference Matrix<T>::get(const sf::Vector2<Tvec>& coord) const
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    return this->g_matrix.at(static_cast<std::size_t>(coord.x)).at(static_cast<std::size_t>(coord.y));
}
template<class T>
typename std::vector<T>::reference Matrix<T>::get(std::size_t x, std::size_t y)
{
    return this->g_matrix.at(x).at(y);
}
template<class T>
template<class Tvec>
typename std::vector<T>::reference Matrix<T>::get(const sf::Vector2<Tvec>& coord)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    return this->g_matrix.at(static_cast<std::size_t>(coord.x)).at(static_cast<std::size_t>(coord.y));
}

template<class T>
bool Matrix<T>::get(std::size_t x, std::size_t y, T& buff) const
{
    if ( (x < this->g_msize.x) && (y < this->g_msize.y) )
    {
        buff = this->g_matrix[x][y];
        return true;
    }
    return false;
}
template<class T>
template<class Tvec>
bool Matrix<T>::get(const sf::Vector2<Tvec>& coord, T& buff) const
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    if ( (static_cast<std::size_t>(coord.x) < this->g_msize.x) && (static_cast<std::size_t>(coord.y) < this->g_msize.y) )
    {
        buff = this->g_matrix[static_cast<std::size_t>(coord.x)][static_cast<std::size_t>(coord.y)];
        return true;
    }
    return false;
}

template<class T>
T* Matrix<T>::getPtr(std::size_t x, std::size_t y)
{
    if ( (x < this->g_msize.x) && (y < this->g_msize.y) )
    {
        return &this->g_matrix[x][y];
    }
    return nullptr;
}
template<class T>
template<class Tvec>
T* Matrix<T>::getPtr(const sf::Vector2<Tvec>& coord)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    if ( (static_cast<std::size_t>(coord.x) < this->g_msize.x) && (static_cast<std::size_t>(coord.y) < this->g_msize.y) )
    {
        return &this->g_matrix[static_cast<std::size_t>(coord.x)][static_cast<std::size_t>(coord.y)];
    }
    return nullptr;
}
template<class T>
const T* Matrix<T>::getPtr(std::size_t x, std::size_t y) const
{
    if ( (x < this->g_msize.x) && (y < this->g_msize.y) )
    {
        return &this->g_matrix[x][y];
    }
    return nullptr;
}
template<class T>
template<class Tvec>
const T* Matrix<T>::getPtr(const sf::Vector2<Tvec>& coord) const
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    if ( (static_cast<std::size_t>(coord.x) < this->g_msize.x) && (static_cast<std::size_t>(coord.y) < this->g_msize.y) )
    {
        return &this->g_matrix[static_cast<std::size_t>(coord.x)][static_cast<std::size_t>(coord.y)];
    }
    return nullptr;
}

template<class T>
std::vector<std::vector<T> >& Matrix<T>::get()
{
    return this->g_matrix;
}
template<class T>
const std::vector<std::vector<T> >& Matrix<T>::get() const
{
    return this->g_matrix;
}

template<class T>
void Matrix<T>::set(std::size_t x, std::size_t y, T&& value)
{
    this->g_matrix.at(x).at(y) = std::move(value);
}
template<class T>
template<class Tvec>
void Matrix<T>::set(const sf::Vector2<Tvec>& coord, T&& value)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    this->g_matrix.at(static_cast<std::size_t>(coord.x)).at(static_cast<std::size_t>(coord.y)) = std::move(value);
}
template<class T>
void Matrix<T>::set(std::size_t x, std::size_t y, const T& value)
{
    this->g_matrix.at(x).at(y) = value;
}
template<class T>
template<class Tvec>
void Matrix<T>::set(const sf::Vector2<Tvec>& coord, const T& value)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    this->g_matrix.at(static_cast<std::size_t>(coord.x)).at(static_cast<std::size_t>(coord.y)) = value;
}

template<class T>
void Matrix<T>::set(std::initializer_list<std::initializer_list<T>> data)
{
    std::size_t sizey = data.size();
    std::size_t sizex = 0;
    if (sizey)
    {
        sizex = data.begin()->size();
        if (!sizex)
        {
            throw std::runtime_error("Matrix : size cannot be 0 (sizex)");
        }
    }
    else
    {
        throw std::runtime_error("Matrix : size cannot be 0 (sizey)");
    }

    unsigned int ycount = 0;

    this->g_matrix.resize(sizex);
    for (auto datay : data)
    {
        std::size_t xcount = 0;
        for (auto datax : datay)
        {
            this->g_matrix[xcount].resize(sizey);
            this->g_matrix[xcount][ycount] = datax;
            ++xcount;
        }
        ++ycount;
    }

    this->g_msize.x = sizex;
    this->g_msize.y = sizey;
}

template<class T>
inline std::size_t Matrix<T>::getTotalSize() const
{
    return this->g_msize.x * this->g_msize.y;
}
template<class T>
const sf::Vector2<std::size_t>& Matrix<T>::getSize() const
{
    return this->g_msize;
}
template<class T>
std::size_t Matrix<T>::getSizeX() const
{
    return this->g_msize.x;
}
template<class T>
std::size_t Matrix<T>::getSizeY() const
{
    return this->g_msize.y;
}

template<class T>
template<class Tvec>
void Matrix<T>::setSize(const sf::Vector2<Tvec>& msize)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    this->g_matrix.resize(static_cast<std::size_t>(msize.x));
    for (std::size_t x=0; x<static_cast<std::size_t>(msize.x); ++x)
    {
        this->g_matrix[x].resize(static_cast<std::size_t>(msize.y));
    }
    this->g_msize.x = static_cast<std::size_t>(msize.x);
    this->g_msize.y = static_cast<std::size_t>(msize.y);
}
template<class T>
void Matrix<T>::setSize(std::size_t sizex, std::size_t sizey)
{
    this->g_matrix.resize(sizex);
    for (std::size_t x=0; x<sizex; ++x)
    {
        this->g_matrix[x].resize(sizey);
    }
    this->g_msize.x = sizex;
    this->g_msize.y = sizey;
}

template<class T>
void Matrix<T>::fill(const T& value)
{
    for (std::size_t x=0; x<this->g_msize.x; ++x)
    {
        for (std::size_t y=0; y<this->g_msize.y; ++y)
        {
            this->g_matrix[x][y] = value;
        }
    }
}

template<class T>
void Matrix<T>::rotateClockwise()
{
    Matrix<T> newMatrix(this->g_msize.y, this->g_msize.x);

    for (std::size_t y=0; y<this->g_msize.y; ++y)
    {
        for (std::size_t x=0; x<this->g_msize.x; ++x)
        {
            newMatrix.g_matrix[newMatrix.g_msize.x - y-1][x] = std::move(this->g_matrix[x][y]);
        }
    }
    this->g_matrix = std::move(newMatrix.g_matrix);
    this->g_msize = newMatrix.g_msize;
}
template<class T>
void Matrix<T>::rotateCounterClockwise()
{
    Matrix<T> newMatrix(this->g_msize.y, this->g_msize.x);

    for (std::size_t y=0; y<this->g_msize.y; ++y)
    {
        for (std::size_t x=0; x<this->g_msize.x; ++x)
        {
            newMatrix.g_matrix[y][newMatrix.g_msize.y - x-1] = std::move(this->g_matrix[x][y]);
        }
    }
    this->g_matrix = std::move(newMatrix.g_matrix);
    this->g_msize = newMatrix.g_msize;
}

template<class T>
void Matrix<T>::rotateClockwise(unsigned int n)
{
    n %= 4;
    while (n>0)
    {
        this->rotateClockwise();
        --n;
    }
}
template<class T>
void Matrix<T>::rotateCounterClockwise(unsigned int n)
{
    n %= 4;
    while (n>0)
    {
        this->rotateCounterClockwise();
        --n;
    }
}

template<class T>
void Matrix<T>::flipHorizontally()
{
    Matrix<T> newMatrix(this->g_msize.x, this->g_msize.y);

    for (std::size_t x=0; x<this->g_msize.x; ++x)
    {
        for (std::size_t y=0; y<this->g_msize.y; ++y)
        {
            newMatrix.g_matrix[x][y] = std::move(this->g_matrix[this->g_msize.x-1-x][y]);
        }
    }
    this->g_matrix = std::move(newMatrix.g_matrix);
    this->g_msize = newMatrix.g_msize;
}

template<class T>
void Matrix<T>::flipVertically()
{
    Matrix<T> newMatrix(this->g_msize.x, this->g_msize.y);

    for (std::size_t x=0; x<this->g_msize.x; ++x)
    {
        for (std::size_t y=0; y<this->g_msize.y; ++y)
        {
            newMatrix.g_matrix[x][y] = std::move(this->g_matrix[x][this->g_msize.y-1-y]);
        }
    }
    this->g_matrix = std::move(newMatrix.g_matrix);
    this->g_msize = newMatrix.g_msize;
}

template<class T>
void Matrix<T>::toVector(std::vector<T>& buff) const
{
    buff.resize(this->g_msize.x*this->g_msize.y);

    for (std::size_t x=0; x<this->g_msize.x; ++x)
    {
        for (std::size_t y=0; y<this->g_msize.y; ++y)
        {
            buff[x + this->g_msize.y*y] = this->g_matrix[x][y];
        }
    }
}

template<class T>
void to_json(nlohmann::json& j, const fge::Matrix<T>& r)
{
    nlohmann::json data = {{"sizeX",r.getSizeX()}, {"sizeY",r.getSizeY()}};

    nlohmann::json& datay = data["data"];
    datay = nlohmann::json::array();

    for (std::size_t y=0; y<r.getSizeY(); ++y)
    {
        nlohmann::json datax = nlohmann::json::array();
        for (std::size_t x=0; x<r.getSizeX(); ++x)
        {
            datax += r[x][y];
        }
        datay += datax;
    }
    j = data;
}
template<class T>
void from_json(const nlohmann::json& j, fge::Matrix<T>& r)
{
    std::size_t sizex, sizey;

    j.at("sizeX").get_to(sizex);
    j.at("sizeY").get_to(sizey);

    const nlohmann::json& datay = j.at("data");

    if (!datay.is_array())
    {
        throw std::runtime_error("Matrix json : must be an array");
    }
    if (datay.size() != sizey)
    {
        throw std::runtime_error("Matrix json : size y is not the same");
    }

    r.setSize(sizex, sizey);

    std::size_t y=0;
    for (nlohmann::json::const_iterator ity = datay.begin(); ity != datay.end(); ++ity)
    {
        if (!(*ity).is_array())
        {
            throw std::runtime_error("Matrix json : must be an array");
        }
        if ((*ity).size() != sizex)
        {
            throw std::runtime_error("Matrix json : size x is not the same");
        }

        std::size_t x=0;
        for (nlohmann::json::const_iterator itx = (*ity).begin(); itx != (*ity).end(); ++itx)
        {
            (*itx).get_to( r[x][y] );
            ++x;
        }
        ++y;
    }
}

}//end fge

