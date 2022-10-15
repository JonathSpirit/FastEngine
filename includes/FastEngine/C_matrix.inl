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
    this->setSize(static_cast<std::size_t>(msize.x), static_cast<std::size_t>(msize.y));
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
    this->setSize(static_cast<std::size_t>(msize.x), static_cast<std::size_t>(msize.y));
    this->fill(defaultValue);
}
template<class T>
Matrix<T>::Matrix(std::size_t sizex, std::size_t sizey, const T& defaultValue)
{
    this->setSize(sizex, sizey);
    this->fill(defaultValue);
}

template<class T>
Matrix<T>::Matrix(const fge::Matrix<T>& m)
{
    this->setSize(m.g_msize);
    for (std::size_t i=0; i<m.getTotalSize(); ++i)
    {
        this->g_mdata[i] = m.g_mdata[i];
    }
}
template<class T>
Matrix<T>::Matrix(fge::Matrix<T>&& m) noexcept :
    g_msize(m.g_msize),
    g_mdata( std::move(m.g_mdata) )
{
    m.g_msize.y = 0;
    m.g_msize.x = 0;
}

template<class T>
void Matrix<T>::clear()
{
    this->g_mdata.reset();
    this->g_msize.x = 0;
    this->g_msize.y = 0;
}

template<class T>
fge::Matrix<T>& Matrix<T>::operator =(const fge::Matrix<T>& m)
{
    this->setSize(m.g_msize);
    for (std::size_t i=0; i<m.getTotalSize(); ++i)
    {
        this->g_mdata[i] = m.g_mdata[i];
    }
    return *this;
}
template<class T>
fge::Matrix<T>& Matrix<T>::operator =(fge::Matrix<T>&& m) noexcept
{
    this->g_msize = m.g_msize;
    m.g_msize.y = 0;
    m.g_msize.x = 0;
    this->g_mdata = std::move(m.g_mdata);
    return *this;
}

template<class T>
T* Matrix<T>::operator[](std::size_t x)
{
    return reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x];
}
template<class T>
const T* Matrix<T>::operator[](std::size_t x) const
{
    return reinterpret_cast<const T(*)[this->g_msize.y]>(this->g_mdata.get())[x];
}

template<class T>
const T& Matrix<T>::get(std::size_t x, std::size_t y) const
{
    return reinterpret_cast<const T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y];
}
template<class T>
template<class Tvec>
const T& Matrix<T>::get(const sf::Vector2<Tvec>& coord) const
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    return reinterpret_cast<const T(*)[this->g_msize.y]>(this->g_mdata.get())[static_cast<std::size_t>(coord.x)][static_cast<std::size_t>(coord.y)];
}
template<class T>
T& Matrix<T>::get(std::size_t x, std::size_t y)
{
    return reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y];
}
template<class T>
template<class Tvec>
T& Matrix<T>::get(const sf::Vector2<Tvec>& coord)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    return reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[static_cast<std::size_t>(coord.x)][static_cast<std::size_t>(coord.y)];
}

template<class T>
bool Matrix<T>::get(std::size_t x, std::size_t y, T& buff) const
{
    if ( (x < this->g_msize.x) && (y < this->g_msize.y) )
    {
        buff = reinterpret_cast<const T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y];
        return true;
    }
    return false;
}
template<class T>
template<class Tvec>
bool Matrix<T>::get(const sf::Vector2<Tvec>& coord, T& buff) const
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    return this->get(static_cast<std::size_t>(coord.x), static_cast<std::size_t>(coord.y), buff);
}

template<class T>
T* Matrix<T>::getPtr(std::size_t x, std::size_t y)
{
    if ( (x < this->g_msize.x) && (y < this->g_msize.y) )
    {
        return &reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y];
    }
    return nullptr;
}
template<class T>
template<class Tvec>
T* Matrix<T>::getPtr(const sf::Vector2<Tvec>& coord)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    return this->getPtr(static_cast<std::size_t>(coord.x), static_cast<std::size_t>(coord.y));
}
template<class T>
const T* Matrix<T>::getPtr(std::size_t x, std::size_t y) const
{
    if ( (x < this->g_msize.x) && (y < this->g_msize.y) )
    {
        return &reinterpret_cast<const T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y];
    }
    return nullptr;
}
template<class T>
template<class Tvec>
const T* Matrix<T>::getPtr(const sf::Vector2<Tvec>& coord) const
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    return this->getPtr(static_cast<std::size_t>(coord.x), static_cast<std::size_t>(coord.y));
}

template<class T>
void Matrix<T>::set(std::size_t x, std::size_t y, T&& value)
{
    reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y] = std::move(value);
}
template<class T>
template<class Tvec>
void Matrix<T>::set(const sf::Vector2<Tvec>& coord, T&& value)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[static_cast<std::size_t>(coord.x)][static_cast<std::size_t>(coord.y)] = std::move(value);
}
template<class T>
void Matrix<T>::set(std::size_t x, std::size_t y, const T& value)
{
    reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y] = value;
}
template<class T>
template<class Tvec>
void Matrix<T>::set(const sf::Vector2<Tvec>& coord, const T& value)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[static_cast<std::size_t>(coord.x)][static_cast<std::size_t>(coord.y)] = value;
}

template<class T>
void Matrix<T>::set(std::initializer_list<std::initializer_list<T>> data)
{
    const std::size_t sizey = data.size();
    std::size_t sizex = 0;
    if (sizey > 0)
    {
        auto it = data.begin();
        sizex = it->size();
        for (++it; it!=data.end(); ++it)
        {
            if (it->size() != sizex)
            {
                throw std::runtime_error("Matrix : size must be constant between rows");
            }
        }
    }

    this->setSize(sizex, sizey);

    std::size_t ycount = 0;
    for (auto datay : data)
    {
        std::size_t xcount = 0;
        for (auto datax : datay)
        {
            reinterpret_cast<T(*)[sizey]>(this->g_mdata.get())[xcount][ycount] = std::move(datax);
            ++xcount;
        }
        ++ycount;
    }
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
inline const T* Matrix<T>::get() const
{
    return this->g_mdata;
}
template<class T>
inline T* Matrix<T>::get()
{
    return this->g_mdata;
}

template<class T>
typename Matrix<T>::iterator Matrix<T>::begin()
{
    return this->g_mdata.get();
}
template<class T>
typename Matrix<T>::iterator Matrix<T>::end()
{
    return this->g_mdata.get()+this->getTotalSize();
}
template<class T>
typename Matrix<T>::const_iterator Matrix<T>::begin() const
{
    return this->g_mdata.get();
}
template<class T>
typename Matrix<T>::const_iterator Matrix<T>::end() const
{
    return this->g_mdata.get()+this->getTotalSize();
}

template<class T>
template<class Tvec>
void Matrix<T>::setSize(const sf::Vector2<Tvec>& msize)
{
    static_assert(std::is_integral<Tvec>::value, "Tvec must be an integral type");
    this->setSize(static_cast<std::size_t>(msize.x), static_cast<std::size_t>(msize.y));
}
template<class T>
void Matrix<T>::setSize(std::size_t sizex, std::size_t sizey)
{
    if (sizex==this->g_msize.x && sizey==this->g_msize.y)
    {//Same size, ignoring
        return;
    }
    if (sizex==0 || sizey==0)
    {//Null size, aborting
        throw std::runtime_error("Matrix : size cannot be 0");
    }

    this->g_mdata.reset( new T[sizex*sizey] );
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
            reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y] = value;
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
            newMatrix[newMatrix.g_msize.x - y-1][x] = std::move(reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y]);
        }
    }
    this->g_mdata = std::move(newMatrix.g_mdata);
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
            newMatrix[y][newMatrix.g_msize.y - x-1] = std::move(reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x][y]);
        }
    }
    this->g_mdata = std::move(newMatrix.g_mdata);
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
            newMatrix[x][y] = std::move(reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[this->g_msize.x-1-x][y]);
        }
    }
    this->g_mdata = std::move(newMatrix.g_mdata);
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
            newMatrix[x][y] = std::move(reinterpret_cast<T(*)[this->g_msize.y]>(this->g_mdata.get())[x][this->g_msize.y-1-y]);
        }
    }
    this->g_mdata = std::move(newMatrix.g_mdata);
    this->g_msize = newMatrix.g_msize;
}

template<class T>
void Matrix<T>::toVector(std::vector<T>& buff) const
{
    buff.assign(this->g_mdata.get(), this->g_mdata.get()+this->g_msize.x*this->g_msize.y);
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
    std::size_t sizex;
    std::size_t sizey;

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

