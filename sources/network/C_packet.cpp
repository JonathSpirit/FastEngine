/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/network/C_packet.hpp"
#include <cstring>

#ifdef __GNUC__
    #if __GNUC__ > 8
        #include <bit>
    #else
        #include <bit>
        #include <type_traits>
    #endif
#else
    #include <bit>
    #include <type_traits>
#endif

namespace fge::net
{

//Packet

Packet::Packet() :
        _g_transmitPos(0),
        _g_transmitCacheValid(false),
        g_readPos(0),
        g_valid(true)
{
    this->g_data.reserve(_defaultReserveSize);
}

Packet::Packet(Packet&& pck) noexcept :
        _g_transmitCache(std::move(pck._g_transmitCache)),
        _g_transmitPos(pck._g_transmitPos),
        _g_transmitCacheValid(pck._g_transmitCacheValid),
        g_data(std::move(pck.g_data)),
        g_readPos(pck.g_readPos),
        g_valid(pck.g_valid)
{
    pck._g_transmitCacheValid = false;
    pck.g_valid = true;
    pck.g_readPos = 0;
    pck._g_transmitPos = 0;
}

Packet::Packet(std::size_t reserveSize) :
        _g_transmitPos(0),
        _g_transmitCacheValid(false),
        g_readPos(0),
        g_valid(true)
{
    this->g_data.reserve(reserveSize);
}

void Packet::clear()
{
    this->_g_transmitPos = 0;
    this->_g_transmitCache.clear();
    this->_g_transmitCacheValid = false;

    this->g_data.clear();
    this->g_readPos = 0;
    this->g_valid = true;
}
void Packet::flush()
{
    this->_g_transmitPos = 0;
    this->_g_transmitCache.clear();
    this->_g_transmitCacheValid = false;
}
void Packet::reserve(std::size_t reserveSize)
{
    this->g_data.reserve(reserveSize);
}

Packet& Packet::append(std::size_t size)
{
    if (size > 0)
    {
        std::size_t startPos = this->g_data.size();
        this->g_data.resize(startPos + size);

        this->_g_transmitCacheValid = false;
    }
    return *this;
}
Packet& Packet::append(void const* data, std::size_t size)
{
    if (data && (size > 0))
    {
        std::size_t startPos = this->g_data.size();
        this->g_data.resize(startPos + size);

        //Copy memory
        for (std::size_t i = 0; i < size; ++i)
        {
            this->g_data[startPos + i] = static_cast<uint8_t const*>(data)[i];
        }
        this->_g_transmitCacheValid = false;
    }
    return *this;
}
Packet& Packet::pack(void const* data, std::size_t size)
{
    if (data && (size > 0))
    {
        std::size_t startPos = this->g_data.size();
        this->g_data.resize(startPos + size);

        if constexpr (std::endian::native == std::endian::big)
        {
            //Copy memory
            for (std::size_t i = 0; i < size; ++i)
            {
                this->g_data[startPos + i] = static_cast<uint8_t const*>(data)[i];
            }
        }
        else
        {
            //Copy memory
            for (std::size_t i = 0; i < size; ++i)
            {
                this->g_data[startPos + i] = static_cast<uint8_t const*>(data)[size - 1 - i];
            }
        }
        this->_g_transmitCacheValid = false;
    }
    return *this;
}

bool Packet::write(std::size_t pos, void const* data, std::size_t size)
{
    if (data && (size > 0) && (pos < this->g_data.size()))
    {
        //Copy memory
        for (std::size_t i = 0; i < size; ++i)
        {
            this->g_data[pos + i] = static_cast<uint8_t const*>(data)[i];
        }
        this->_g_transmitCacheValid = false;
        return true;
    }
    return false;
}
bool Packet::pack(std::size_t pos, void const* data, std::size_t size)
{
    if (data && (size > 0) && (pos < this->g_data.size()))
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            //Copy memory
            for (std::size_t i = 0; i < size; ++i)
            {
                this->g_data[pos + i] = static_cast<uint8_t const*>(data)[i];
            }
        }
        else
        {
            //Copy memory
            for (std::size_t i = 0; i < size; ++i)
            {
                this->g_data[pos + i] = static_cast<uint8_t const*>(data)[size - 1 - i];
            }
        }
        this->_g_transmitCacheValid = false;
        return true;
    }
    return false;
}

Packet const& Packet::read(void* buff, std::size_t size) const
{
    if (buff && (size > 0) && (this->g_readPos + size <= this->g_data.size()))
    {
        //Copy to buff
        for (std::size_t i = 0; i < size; ++i)
        {
            static_cast<uint8_t*>(buff)[i] = this->g_data[this->g_readPos + i];
        }
        this->g_readPos += size;
        this->g_valid = true;
        return *this;
    }
    this->g_valid = false;
    return *this;
}
Packet const& Packet::unpack(void* buff, std::size_t size) const
{
    if (buff && (size > 0) && (this->g_readPos + size <= this->g_data.size()))
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            //Copy to buff
            for (std::size_t i = 0; i < size; ++i)
            {
                static_cast<uint8_t*>(buff)[i] = this->g_data[this->g_readPos + i];
            }
        }
        else
        {
            //Copy to buff
            for (std::size_t i = 0; i < size; ++i)
            {
                static_cast<uint8_t*>(buff)[size - 1 - i] = this->g_data[this->g_readPos + i];
            }
        }
        this->g_readPos += size;
        this->g_valid = true;
        return *this;
    }
    this->g_valid = false;
    return *this;
}

bool Packet::read(std::size_t pos, void* buff, std::size_t size) const
{
    if (buff && (size > 0) && (pos + size <= this->g_data.size()))
    {
        //Copy to buff
        for (std::size_t i = 0; i < size; ++i)
        {
            static_cast<uint8_t*>(buff)[i] = this->g_data[pos + i];
        }
        return true;
    }
    return false;
}
bool Packet::unpack(std::size_t pos, void* buff, std::size_t size) const
{
    if (buff && (size > 0) && (pos + size <= this->g_data.size()))
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            //Copy to buff
            for (std::size_t i = 0; i < size; ++i)
            {
                static_cast<uint8_t*>(buff)[i] = this->g_data[pos + i];
            }
        }
        else
        {
            //Copy to buff
            for (std::size_t i = 0; i < size; ++i)
            {
                static_cast<uint8_t*>(buff)[size - 1 - i] = this->g_data[pos + i];
            }
        }
        return true;
    }
    return false;
}

Packet& Packet::shrink(std::size_t size)
{
    if (size > 0)
    {
        if (size >= this->g_data.size())
        {
            this->g_data.resize(0);
        }
        else
        {
            std::size_t startPos = this->g_data.size();
            this->g_data.resize(startPos - size);
        }

        this->_g_transmitCacheValid = false;
    }
    return *this;
}
bool Packet::erase(std::size_t pos, std::size_t size)
{
    if ((size > 0) && (pos + size <= this->g_data.size()))
    {
        this->g_data.erase(this->g_data.begin() + pos, this->g_data.begin() + pos + size);
        this->_g_transmitCacheValid = false;
    }
    return false;
}
Packet const& Packet::skip(std::size_t size) const
{
    if ((size > 0) && (this->g_readPos + size <= this->g_data.size()))
    {
        this->g_readPos += size;
        this->g_valid = true;
        return *this;
    }
    this->g_valid = false;
    return *this;
}

void Packet::setReadPos(std::size_t pos) const
{
    this->g_readPos = (pos > this->g_data.size()) ? this->g_data.size() : pos;
}
std::size_t Packet::getReadPos() const
{
    return this->g_readPos;
}
bool Packet::isExtractable(std::size_t size) const
{
    return (this->g_readPos + size) <= this->g_data.size();
}

uint8_t const* Packet::getData(std::size_t pos) const
{
    return (pos < this->g_data.size()) ? &this->g_data[pos] : nullptr;
}
uint8_t* Packet::getData(std::size_t pos)
{
    return (pos < this->g_data.size()) ? &this->g_data[pos] : nullptr;
}
uint8_t const* Packet::getData() const
{
    return this->g_data.data();
}
uint8_t* Packet::getData()
{
    return this->g_data.data();
}

std::size_t Packet::getDataSize() const
{
    return this->g_data.size();
}
uint32_t Packet::getLength() const
{
    uint32_t result = 0;
    this->unpack(this->g_readPos, &result, sizeof(uint32_t));
    return result;
}

void Packet::invalidate() const
{
    this->g_valid = false;
}
void Packet::setValidity(bool validity) const
{
    this->g_valid = validity;
}
bool Packet::isValid() const
{
    return this->g_valid;
}
Packet::operator bool() const
{
    return this->g_valid;
}
bool Packet::endReached() const
{
    return this->g_readPos >= this->g_data.size();
}

std::vector<uint8_t> const& Packet::getTransmitCache() const
{
    return this->_g_transmitCache;
}
std::size_t Packet::getTransmitPos() const
{
    return this->_g_transmitPos;
}
bool Packet::isTransmitCacheValid() const
{
    return this->_g_transmitCacheValid;
}
void Packet::invalidateTransmitCache()
{
    this->_g_transmitCacheValid = false;
}

///

Packet& Packet::operator<<(std::string_view const& data)
{
    SizeType length = static_cast<SizeType>(data.size());

    this->pack(&length, sizeof(length));
    return this->append(data.data(), sizeof(std::string_view::value_type) * length);
}
Packet& Packet::operator<<(char const* data)
{
    SizeType length = static_cast<SizeType>(std::strlen(data));

    this->pack(&length, sizeof(length));
    return this->append(data, sizeof(char) * length);
}
Packet& Packet::operator<<(std::string const& data)
{
    SizeType length = static_cast<SizeType>(data.size());

    this->pack(&length, sizeof(length));
    return this->append(data.data(), sizeof(std::string::value_type) * length);
}
Packet& Packet::operator<<(tiny_utf8::string const& data)
{
    SizeType length = static_cast<SizeType>(data.size());

    this->pack(&length, sizeof(length));
    return this->append(data.data(), sizeof(tiny_utf8::string::data_type) * length);
}
Packet& Packet::operator<<(wchar_t const* data)
{
    SizeType length = static_cast<SizeType>(std::wcslen(data));

    this->pack(&length, sizeof(length));

    for (SizeType i = 0; i < length; ++i)
    {
        uint32_t buff = data[i];
        this->pack(&buff, sizeof(uint32_t));
    }
    return *this;
}
Packet& Packet::operator<<(std::wstring const& data)
{
    SizeType length = static_cast<SizeType>(data.size());

    this->pack(&length, sizeof(length));

    for (SizeType i = 0; i < length; ++i)
    {
        uint32_t buff = data[i];
        this->pack(&buff, sizeof(uint32_t));
    }
    return *this;
}

Packet const& Packet::operator>>(char* data) const
{
    SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->g_readPos + length - 1) < this->g_data.size())
        {
            this->read(data, sizeof(char) * length);
            data[length] = '\0';
        }
        else
        {
            this->g_valid = false;
        }
    }
    else
    {
        data[0] = '\0';
    }
    return *this;
}
Packet const& Packet::operator>>(std::string& data) const
{
    SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->g_readPos + length - 1) < this->g_data.size())
        {
            data.clear();
            data.assign(reinterpret_cast<char const*>(&this->g_data[this->g_readPos]), length);

            this->g_readPos += length;
        }
        else
        {
            this->g_valid = false;
        }
    }
    else
    {
        data.clear();
    }
    return *this;
}
Packet const& Packet::operator>>(tiny_utf8::string& data) const
{
    SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->g_readPos + length - 1) < this->g_data.size())
        {
            data.clear();
            data.assign(reinterpret_cast<char const*>(&this->g_data[this->g_readPos]), length);

            this->g_readPos += length;
        }
        else
        {
            this->g_valid = false;
        }
    }
    else
    {
        data.clear();
    }
    return *this;
}
Packet const& Packet::operator>>(wchar_t* data) const
{
    SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->g_readPos + (length - 1) * sizeof(uint32_t)) < this->g_data.size())
        {
            for (SizeType i = 0; i < length; ++i)
            {
                uint32_t buff = 0;
                this->unpack(&buff, sizeof(uint32_t));
                data[i] = static_cast<wchar_t>(buff);
            }
            data[length] = L'\0';
        }
        else
        {
            this->g_valid = false;
        }
    }
    else
    {
        data[0] = L'\0';
    }
    return *this;
}
Packet const& Packet::operator>>(std::wstring& data) const
{
    SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->g_readPos + (length - 1) * sizeof(uint32_t)) < this->g_data.size())
        {
            data.resize(length);
            for (SizeType i = 0; i < length; ++i)
            {
                uint32_t buff = 0;
                this->unpack(&buff, sizeof(uint32_t));
                data[i] = static_cast<wchar_t>(buff);
            }
        }
        else
        {
            this->g_valid = false;
        }
    }
    else
    {
        data.clear();
    }
    return *this;
}

bool Packet::onSend(std::size_t offset)
{
    this->_g_transmitCacheValid = true;
    this->_g_transmitCache.resize(this->g_data.size() + offset);
    std::memcpy(this->_g_transmitCache.data() + offset, this->g_data.data(), this->g_data.size());
    return true;
}
void Packet::onReceive(std::span<uint8_t const> const& data)
{
    if (data.data() == nullptr || data.empty())
    {
        this->invalidate();
        return;
    }

    this->append(data.data(), data.size());
}

std::size_t Packet::_defaultReserveSize = FGE_PACKET_DEFAULT_RESERVESIZE;

} // namespace fge::net
