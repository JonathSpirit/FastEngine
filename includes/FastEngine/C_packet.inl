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
namespace net
{

fge::net::Packet& Packet::operator<<(bool data)
{
    uint8_t a = data ? 1 : 0;
    return this->append(&a, sizeof(uint8_t));
}

fge::net::Packet& Packet::operator<<(int8_t data)
{
    return this->append(&data, sizeof(int8_t));
}
fge::net::Packet& Packet::operator<<(int16_t data)
{
    return this->pack(&data, sizeof(int16_t));
}
fge::net::Packet& Packet::operator<<(int32_t data)
{
    return this->pack(&data, sizeof(int32_t));
}
fge::net::Packet& Packet::operator<<(int64_t data)
{
    return this->pack(&data, sizeof(int64_t));
}

fge::net::Packet& Packet::operator<<(uint8_t data)
{
    return this->append(&data, sizeof(uint8_t));
}
fge::net::Packet& Packet::operator<<(uint16_t data)
{
    return this->pack(&data, sizeof(uint16_t));
}
fge::net::Packet& Packet::operator<<(uint32_t data)
{
    return this->pack(&data, sizeof(uint32_t));
}
fge::net::Packet& Packet::operator<<(uint64_t data)
{
    return this->pack(&data, sizeof(uint64_t));
}

fge::net::Packet& Packet::operator<<(float data)
{
    return this->pack(&data, sizeof(float));
}
fge::net::Packet& Packet::operator<<(double data)
{
    return this->pack(&data, sizeof(double));
}
fge::net::Packet& Packet::operator<<(long double data)
{
    return this->pack(&data, sizeof(long double));
}

template<typename T>
fge::net::Packet& Packet::operator<<(const std::forward_list<T>& data)
{
    *this << static_cast<fge::net::SizeType>(data.size());
    for (auto it = data.cbegin(); it != data.cend(); ++it)
    {
        *this << (*it);
    }
    return *this;
}
template<typename T>
fge::net::Packet& Packet::operator<<(const std::list<T>& data)
{
    *this << static_cast<fge::net::SizeType>(data.size());
    for (auto it = data.cbegin(); it != data.cend(); ++it)
    {
        *this << (*it);
    }
    return *this;
}
template<typename T>
fge::net::Packet& Packet::operator<<(const std::vector<T>& data)
{
    *this << static_cast<fge::net::SizeType>(data.size());
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        *this << data[i];
    }
    return *this;
}

template<typename T>
fge::net::Packet& Packet::operator<<(const sf::Vector2<T>& data)
{
    return *this << data.x << data.y;
}
template<typename T>
fge::net::Packet& Packet::operator<<(const sf::Vector3<T>& data)
{
    return *this << data.x << data.y << data.z;
}

template<typename T>
fge::net::Packet& Packet::operator<<(const fge::Matrix<T>& data)
{
    *this << static_cast<fge::net::SizeType>(data.getSizeX()) << static_cast<fge::net::SizeType>(data.getSizeY());
    for (std::size_t x = 0; x < data.getSizeX(); ++x)
    {
        for (std::size_t y = 0; y < data.getSizeY(); ++y)
        {
            *this << data[x][y];
        }
    }
    return *this;
}

fge::net::Packet& Packet::operator<<(const sf::Color& data)
{
    return *this << static_cast<uint32_t>(data.toInteger());
}

fge::net::Packet& Packet::operator<<(const fge::net::IpAddress& data)
{
    uint32_t ip = data.getNetworkByteOrder();
    return this->append(&ip, sizeof(ip));
}

///

const fge::net::Packet& Packet::operator>>(bool& data) const
{
    uint8_t a;
    this->read(&a, sizeof(uint8_t));
    data = (a > 0);
    return *this;
}

const fge::net::Packet& Packet::operator>>(int8_t& data) const
{
    return this->read(&data, sizeof(int8_t));
}
const fge::net::Packet& Packet::operator>>(int16_t& data) const
{
    return this->unpack(&data, sizeof(int16_t));
}
const fge::net::Packet& Packet::operator>>(int32_t& data) const
{
    return this->unpack(&data, sizeof(int32_t));
}
const fge::net::Packet& Packet::operator>>(int64_t& data) const
{
    return this->unpack(&data, sizeof(int64_t));
}

const fge::net::Packet& Packet::operator>>(uint8_t& data) const
{
    return this->read(&data, sizeof(uint8_t));
}
const fge::net::Packet& Packet::operator>>(uint16_t& data) const
{
    return this->unpack(&data, sizeof(uint16_t));
}
const fge::net::Packet& Packet::operator>>(uint32_t& data) const
{
    return this->unpack(&data, sizeof(uint32_t));
}
const fge::net::Packet& Packet::operator>>(uint64_t& data) const
{
    return this->unpack(&data, sizeof(uint64_t));
}

const fge::net::Packet& Packet::operator>>(float& data) const
{
    return this->unpack(&data, sizeof(float));
}
const fge::net::Packet& Packet::operator>>(double& data) const
{
    return this->unpack(&data, sizeof(double));
}
const fge::net::Packet& Packet::operator>>(long double& data) const
{
    return this->unpack(&data, sizeof(long double));
}

template<typename T>
const fge::net::Packet& Packet::operator>>(std::forward_list<T>& data) const
{
    fge::net::SizeType length = 0;
    *this >> length;

    data.resize(length);
    auto it = data.begin();

    for (fge::net::SizeType i = 0; i < length; ++i)
    {
        *this >> (*it);
        ++it;
    }
    return *this;
}
template<typename T>
const fge::net::Packet& Packet::operator>>(std::list<T>& data) const
{
    fge::net::SizeType length = 0;
    *this >> length;

    data.resize(length);
    auto it = data.begin();

    for (fge::net::SizeType i = 0; i < length; ++i)
    {
        *this >> (*it);
        ++it;
    }
    return *this;
}
template<typename T>
const fge::net::Packet& Packet::operator>>(std::vector<T>& data) const
{
    fge::net::SizeType length = 0;
    *this >> length;

    data.resize(length);

    for (fge::net::SizeType i = 0; i < length; ++i)
    {
        *this >> data[length];
    }
    return *this;
}

template<typename T>
const fge::net::Packet& Packet::operator>>(sf::Vector2<T>& data) const
{
    return *this >> data.x >> data.y;
}
template<typename T>
const fge::net::Packet& Packet::operator>>(sf::Vector3<T>& data) const
{
    return *this >> data.x >> data.y >> data.z;
}

template<typename T>
const fge::net::Packet& Packet::operator>>(fge::Matrix<T>& data) const
{
    fge::net::SizeType sizeX = 0, sizeY = 0;
    *this >> sizeX >> sizeY;

    data.setSize(sizeX, sizeY);

    for (fge::net::SizeType x = 0; x < sizeX; ++x)
    {
        for (fge::net::SizeType y = 0; y < sizeY; ++y)
        {
            *this >> data[x][y];
        }
    }
    return *this;
}

const fge::net::Packet& Packet::operator>>(sf::Color& data) const
{
    uint32_t buff;
    *this >> buff;
    data = sf::Color(buff);
    return *this;
}

const fge::net::Packet& Packet::operator>>(fge::net::IpAddress& data) const
{
    uint32_t ip = 0;
    this->read(&ip, sizeof(uint32_t));
    data.setNetworkByteOrdered(ip);
    return *this;
}

} // namespace net
} // namespace fge
