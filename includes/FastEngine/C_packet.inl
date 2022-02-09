namespace fge
{
namespace net
{

fge::net::Packet& Packet::operator <<(bool data)
{
    uint8_t a = data ? 1 : 0;
    return this->append(&a, sizeof(uint8_t));
}

fge::net::Packet& Packet::operator <<(int8_t data)
{
    return this->append(&data, sizeof(int8_t));
}
fge::net::Packet& Packet::operator <<(int16_t data)
{
    return this->pack(&data, sizeof(int16_t));
}
fge::net::Packet& Packet::operator <<(int32_t data)
{
    return this->pack(&data, sizeof(int32_t));
}
fge::net::Packet& Packet::operator <<(int64_t data)
{
    return this->pack(&data, sizeof(int64_t));
}

fge::net::Packet& Packet::operator <<(uint8_t data)
{
    return this->append(&data, sizeof(uint8_t));
}
fge::net::Packet& Packet::operator <<(uint16_t data)
{
    return this->pack(&data, sizeof(uint16_t));
}
fge::net::Packet& Packet::operator <<(uint32_t data)
{
    return this->pack(&data, sizeof(uint32_t));
}
fge::net::Packet& Packet::operator <<(uint64_t data)
{
    return this->pack(&data, sizeof(uint64_t));
}

fge::net::Packet& Packet::operator <<(float data)
{
    return this->pack(&data, sizeof(float));
}
fge::net::Packet& Packet::operator <<(double data)
{
    return this->pack(&data, sizeof(double));
}
fge::net::Packet& Packet::operator <<(long double data)
{
    return this->pack(&data, sizeof(long double));
}

template <typename T>
fge::net::Packet& Packet::operator <<(const std::forward_list<T>& data)
{
    *this << static_cast<uint32_t>(data.size());
    for (auto it=data.cbegin(); it!=data.cend(); ++it)
    {
        *this << (*it);
    }
    return *this;
}
template <typename T>
fge::net::Packet& Packet::operator <<(const std::list<T>& data)
{
    *this << static_cast<uint32_t>(data.size());
    for (auto it=data.cbegin(); it!=data.cend(); ++it)
    {
        *this << (*it);
    }
    return *this;
}
template <typename T>
fge::net::Packet& Packet::operator <<(const std::vector<T>& data)
{
    *this << static_cast<uint32_t>(data.size());
    for (std::size_t i=0; i<data.size(); ++i)
    {
        *this << data[i];
    }
    return *this;
}

template<typename T>
fge::net::Packet& Packet::operator <<(const sf::Vector2<T>& data)
{
    return *this << data.x << data.y;
}
template<typename T>
fge::net::Packet& Packet::operator <<(const sf::Vector3<T>& data)
{
    return *this << data.x << data.y << data.z;
}

template<typename T>
fge::net::Packet& Packet::operator <<(const fge::Matrix<T>& data)
{
    *this << static_cast<uint32_t>(data.getSizeX()) << static_cast<uint32_t>(data.getSizeY());
    for (std::size_t x=0; x<data.getSizeX(); ++x)
    {
        for (std::size_t y=0; y<data.getSizeY(); ++y)
        {
            *this << data[x][y];
        }
    }
    return *this;
}

fge::net::Packet& Packet::operator <<(const sf::Color& data)
{
    return *this << static_cast<uint32_t>(data.toInteger());
}

fge::net::Packet& Packet::operator <<(const fge::net::IpAddress& data)
{
    uint32_t ip = data.getNetworkByteOrder();
    return this->append(&ip, sizeof(uint32_t) );
}

///

fge::net::Packet& Packet::operator >>(bool& data)
{
    uint8_t a;
    this->read(&a, sizeof(uint8_t));
    data = (a > 0);
    return *this;
}

fge::net::Packet& Packet::operator >>(int8_t& data)
{
    return this->read(&data, sizeof(int8_t));
}
fge::net::Packet& Packet::operator >>(int16_t& data)
{
    return this->unpack(&data, sizeof(int16_t));
}
fge::net::Packet& Packet::operator >>(int32_t& data)
{
    return this->unpack(&data, sizeof(int32_t));
}
fge::net::Packet& Packet::operator >>(int64_t& data)
{
    return this->unpack(&data, sizeof(int64_t));
}

fge::net::Packet& Packet::operator >>(uint8_t& data)
{
    return this->read(&data, sizeof(uint8_t));
}
fge::net::Packet& Packet::operator >>(uint16_t& data)
{
    return this->unpack(&data, sizeof(uint16_t));
}
fge::net::Packet& Packet::operator >>(uint32_t& data)
{
    return this->unpack(&data, sizeof(uint32_t));
}
fge::net::Packet& Packet::operator >>(uint64_t& data)
{
    return this->unpack(&data, sizeof(uint64_t));
}

fge::net::Packet& Packet::operator >>(float& data)
{
    return this->unpack(&data, sizeof(float));
}
fge::net::Packet& Packet::operator >>(double& data)
{
    return this->unpack(&data, sizeof(double));
}
fge::net::Packet& Packet::operator >>(long double& data)
{
    return this->unpack(&data, sizeof(long double));
}

template <typename T>
fge::net::Packet& Packet::operator >>(std::forward_list<T>& data)
{
    uint32_t length = 0;
    *this >> length;

    data.resize(length);
    auto it = data.begin();

    for (uint32_t i=0; i<length; ++i)
    {
        *this >> (*it);
        ++it;
    }
    return *this;
}
template <typename T>
fge::net::Packet& Packet::operator >>(std::list<T>& data)
{
    uint32_t length = 0;
    *this >> length;

    data.resize(length);
    auto it = data.begin();

    for (uint32_t i=0; i<length; ++i)
    {
        *this >> (*it);
        ++it;
    }
    return *this;
}
template <typename T>
fge::net::Packet& Packet::operator >>(std::vector<T>& data)
{
    uint32_t length = 0;
    *this >> length;

    data.resize(length);

    for (uint32_t i=0; i<length; ++i)
    {
        *this >> data[length];
    }
    return *this;
}

template<typename T>
fge::net::Packet& Packet::operator >>(sf::Vector2<T>& data)
{
    return *this >> data.x >> data.y;
}
template<typename T>
fge::net::Packet& Packet::operator >>(sf::Vector3<T>& data)
{
    return *this >> data.x >> data.y >> data.z;
}

template<typename T>
fge::net::Packet& Packet::operator >>(fge::Matrix<T>& data)
{
    uint32_t sizeX=0, sizeY=0;
    *this >> sizeX >> sizeY;

    data.setSize(sizeX, sizeY);

    for (uint32_t x=0; x<sizeX; ++x)
    {
        for (uint32_t y=0; y<sizeY; ++y)
        {
            *this >> data[x][y];
        }
    }
    return *this;
}

fge::net::Packet& Packet::operator >>(sf::Color& data)
{
    uint32_t buff;
    *this >> buff;
    data = sf::Color(buff);
    return *this;
}

fge::net::Packet& Packet::operator >>(fge::net::IpAddress& data)
{
    uint32_t ip=0;
    this->read(&ip, sizeof(uint32_t) );
    data.setNetworkByteOrdered(ip);
    return *this;
}

}//end net
}//end fge
