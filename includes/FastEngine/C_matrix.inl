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
Matrix<T>::Matrix(const sf::Vector2<std::size_t>& msize)
{
    this->setSize(msize);
}
template<class T>
Matrix<T>::Matrix(std::size_t sizex, std::size_t sizey)
{
    this->setSize(sizex, sizey);
}

template<class T>
Matrix<T>::Matrix(const sf::Vector2<std::size_t>& msize, const T& defaultValue)
{
    this->setSize(msize);
    this->fill(defaultValue);
}
template<class T>
Matrix<T>::Matrix(std::size_t sizex, std::size_t sizey, const T& defaultValue)
{
    this->setSize(sizex, sizey);
    this->fill(defaultValue);
}

template<class T>
Matrix<T>::Matrix(fge::Matrix<T>&& m)
{
    this->g_msize = m.g_msize;
    m.g_msize.y = 0;
    m.g_msize.x = 0;
    this->g_matrix = std::move(m.g_matrix);
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
fge::Matrix<T>& Matrix<T>::operator =(fge::Matrix<T>&& m)
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
typename std::vector<T>::const_reference Matrix<T>::get(const sf::Vector2<std::size_t>& coord) const
{
    return this->g_matrix.at(coord.x).at(coord.y);
}
template<class T>
typename std::vector<T>::reference Matrix<T>::get(std::size_t x, std::size_t y)
{
    return this->g_matrix.at(x).at(y);
}
template<class T>
typename std::vector<T>::reference Matrix<T>::get(const sf::Vector2<std::size_t>& coord)
{
    return this->g_matrix.at(coord.x).at(coord.y);
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
bool Matrix<T>::get(const sf::Vector2<std::size_t>& coord, T& buff) const
{
    if ( (coord.x < this->g_msize.x) && (coord.y < this->g_msize.y) )
    {
        buff = this->g_matrix[coord.x][coord.y];
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
T* Matrix<T>::getPtr(const sf::Vector2<std::size_t>& coord)
{
    if ( (coord.x < this->g_msize.x) && (coord.y < this->g_msize.y) )
    {
        return &this->g_matrix[coord.x][coord.y];
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
const T* Matrix<T>::getPtr(const sf::Vector2<std::size_t>& coord) const
{
    if ( (coord.x < this->g_msize.x) && (coord.y < this->g_msize.y) )
    {
        return &this->g_matrix[coord.x][coord.y];
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
void Matrix<T>::set(std::size_t x, std::size_t y, const T&& value)
{
    this->g_matrix.at(x).at(y) = std::move(value);
}
template<class T>
void Matrix<T>::set(const sf::Vector2<std::size_t>& coord, const T&& value)
{
    this->g_matrix.at(coord.x).at(coord.y) = std::move(value);
}
template<class T>
void Matrix<T>::set(std::size_t x, std::size_t y, T&& value)
{
    this->g_matrix.at(x).at(y) = std::move(value);
}
template<class T>
void Matrix<T>::set(const sf::Vector2<std::size_t>& coord, T&& value)
{
    this->g_matrix.at(coord.x).at(coord.y) = std::move(value);
}
template<class T>
void Matrix<T>::set(std::size_t x, std::size_t y, const T& value)
{
    this->g_matrix.at(x).at(y) = value;
}
template<class T>
void Matrix<T>::set(const sf::Vector2<std::size_t>& coord, const T& value)
{
    this->g_matrix.at(coord.x).at(coord.y) = value;
}
template<class T>
void Matrix<T>::set(std::size_t x, std::size_t y, T& value)
{
    this->g_matrix.at(x).at(y) = value;
}
template<class T>
void Matrix<T>::set(const sf::Vector2<std::size_t>& coord, T& value)
{
    this->g_matrix.at(coord.x).at(coord.y) = value;
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
void Matrix<T>::setSize(const sf::Vector2<std::size_t>& msize)
{
    this->g_matrix.resize(msize.x);
    for (std::size_t x=0; x<msize.x; ++x)
    {
        this->g_matrix[x].resize(msize.y);
    }
    this->g_msize.x = msize.x;
    this->g_msize.y = msize.y;
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
            newMatrix.g_matrix[newMatrix.g_msize.x - y-1][x] = this->g_matrix[x][y];
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
            newMatrix.g_matrix[y][newMatrix.g_msize.y - x-1] = this->g_matrix[x][y];
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

