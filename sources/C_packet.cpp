#include "FastEngine/C_packet.hpp"
#include <cstring>

#ifdef __GNUC__
    #if __GNUC__ > 8
        #include <bit>
    #else
        #include <type_traits>
    #endif
#else
    #include <bit>
#endif

namespace fge
{
namespace net
{

///Packet

Packet::Packet() :
    _g_sendPos(0),
    _g_lastData(),
    _g_lastDataValidity(false),
    _g_data(),
    _g_readPos(0),
    _g_valid(true)
{
    this->_g_data.reserve(fge::net::Packet::_defaultReserveSize);
}

Packet::Packet(fge::net::Packet&& pck) noexcept :
    _g_sendPos(pck._g_sendPos),
    _g_lastData(std::move(pck._g_lastData)),
    _g_lastDataValidity(pck._g_lastDataValidity),
    _g_data(std::move(pck._g_data)),
    _g_readPos(pck._g_readPos),
    _g_valid(pck._g_valid)
{
    pck._g_lastDataValidity = false;
    pck._g_valid = true;
    pck._g_readPos = 0;
    pck._g_sendPos = 0;
}

Packet::Packet(std::size_t reserveSize) :
    _g_sendPos(0),
    _g_lastData(),
    _g_lastDataValidity(false),
    _g_data(),
    _g_readPos(0),
    _g_valid(true)
{
    this->_g_data.reserve(reserveSize);
}

void Packet::clear()
{
    this->_g_sendPos = 0;
    this->_g_lastData.clear();
    this->_g_lastDataValidity = false;

    this->_g_data.clear();
    this->_g_readPos = 0;
    this->_g_valid = true;
}
void Packet::flush()
{
    this->_g_sendPos = 0;
    this->_g_lastData.clear();
    this->_g_lastDataValidity = false;
}
void Packet::reserve(std::size_t reserveSize)
{
    this->_g_data.reserve(reserveSize);
}

fge::net::Packet& Packet::append(std::size_t dsize)
{
    if (dsize > 0)
    {
        std::size_t startPos = this->_g_data.size();
        this->_g_data.resize(startPos + dsize);

        this->_g_lastDataValidity = false;
    }
    return *this;
}
fge::net::Packet& Packet::append(const void* data, std::size_t dsize)
{
    if (data && (dsize > 0))
    {
        std::size_t startPos = this->_g_data.size();
        this->_g_data.resize(startPos + dsize);

        //Copy memory
        for (std::size_t i=0; i<dsize; ++i)
        {
            this->_g_data[startPos+i] = static_cast<const uint8_t*>(data)[i];
        }
        this->_g_lastDataValidity = false;
    }
    return *this;
}
fge::net::Packet& Packet::pack(const void* data, std::size_t dsize)
{
    if (data && (dsize > 0))
    {
        std::size_t startPos = this->_g_data.size();
        this->_g_data.resize(startPos + dsize);

        if constexpr (std::endian::native == std::endian::big)
        {
            //Copy memory
            for (std::size_t i=0; i<dsize; ++i)
            {
                this->_g_data[startPos+i] = static_cast<const uint8_t*>(data)[i];
            }
        }
        else
        {
            //Copy memory
            for (std::size_t i=0; i<dsize; ++i)
            {
                this->_g_data[startPos+i] = static_cast<const uint8_t*>(data)[dsize-1-i];
            }
        }
        this->_g_lastDataValidity = false;
    }
    return *this;
}

bool Packet::write(std::size_t pos, const void* data, std::size_t dsize)
{
    if (data && (dsize > 0) && (pos < this->_g_data.size()))
    {
        //Copy memory
        for (std::size_t i=0; i<dsize; ++i)
        {
            this->_g_data[pos+i] = static_cast<const uint8_t*>(data)[i];
        }
        this->_g_lastDataValidity = false;
        return true;
    }
    return false;
}
bool Packet::pack(std::size_t pos, const void* data, std::size_t dsize)
{
    if (data && (dsize > 0) && (pos < this->_g_data.size()))
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            //Copy memory
            for (std::size_t i=0; i<dsize; ++i)
            {
                this->_g_data[pos+i] = static_cast<const uint8_t*>(data)[i];
            }
        }
        else
        {
            //Copy memory
            for (std::size_t i=0; i<dsize; ++i)
            {
                this->_g_data[pos+i] = static_cast<const uint8_t*>(data)[dsize-1-i];
            }
        }
        this->_g_lastDataValidity = false;
        return true;
    }
    return false;
}

fge::net::Packet& Packet::read(void* buff, std::size_t bsize)
{
    if (buff && (bsize > 0) && (this->_g_readPos+bsize <= this->_g_data.size()))
    {
        //Copy to buff
        for (std::size_t i=0; i<bsize; ++i)
        {
            static_cast<uint8_t*>(buff)[i] = this->_g_data[this->_g_readPos+i];
        }
        this->_g_readPos += bsize;
        this->_g_valid = true;
        return *this;
    }
    this->_g_valid = false;
    return *this;
}
fge::net::Packet& Packet::unpack(void* buff, std::size_t bsize)
{
    if (buff && (bsize > 0) && (this->_g_readPos+bsize <= this->_g_data.size()))
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            //Copy to buff
            for (std::size_t i=0; i<bsize; ++i)
            {
                static_cast<uint8_t*>(buff)[i] = this->_g_data[this->_g_readPos+i];
            }
        }
        else
        {
            //Copy to buff
            for (std::size_t i=0; i<bsize; ++i)
            {
                static_cast<uint8_t*>(buff)[bsize-1-i] = this->_g_data[this->_g_readPos+i];
            }
        }
        this->_g_readPos += bsize;
        this->_g_valid = true;
        return *this;
    }
    this->_g_valid = false;
    return *this;
}

bool Packet::read(std::size_t pos, void* buff, std::size_t bsize) const
{
    if (buff && (bsize > 0) && (pos+bsize <= this->_g_data.size()))
    {
        //Copy to buff
        for (std::size_t i=0; i<bsize; ++i)
        {
            static_cast<uint8_t*>(buff)[i] = this->_g_data[pos+i];
        }
        return true;
    }
    return false;
}
bool Packet::unpack(std::size_t pos, void* buff, std::size_t bsize) const
{
    if (buff && (bsize > 0) && (pos+bsize <= this->_g_data.size()))
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            //Copy to buff
            for (std::size_t i=0; i<bsize; ++i)
            {
                static_cast<uint8_t*>(buff)[i] = this->_g_data[pos+i];
            }
        }
        else
        {
            //Copy to buff
            for (std::size_t i=0; i<bsize; ++i)
            {
                static_cast<uint8_t*>(buff)[bsize-1-i] = this->_g_data[pos+i];
            }
        }
        return true;
    }
    return false;
}

fge::net::Packet& Packet::shrink(std::size_t dsize)
{
    if (dsize > 0)
    {
        if (dsize >= this->_g_data.size())
        {
            this->_g_data.resize(0);
        }
        else
        {
            std::size_t startPos = this->_g_data.size();
            this->_g_data.resize(startPos - dsize);
        }

        this->_g_lastDataValidity = false;
    }
    return *this;
}
bool Packet::erase(std::size_t pos, std::size_t dsize)
{
    if ((dsize > 0) && (pos+dsize <= this->_g_data.size()))
    {
        this->_g_data.erase(this->_g_data.begin()+pos, this->_g_data.begin()+pos+dsize);
        this->_g_lastDataValidity = false;
    }
    return false;
}

void Packet::setReadPos(std::size_t pos)
{
    this->_g_readPos = (pos>this->_g_data.size()) ? this->_g_data.size() : pos;
}
std::size_t Packet::getReadPos() const
{
    return this->_g_readPos;
}

const uint8_t* Packet::getData(std::size_t pos) const
{
    return (pos < this->_g_data.size()) ? &this->_g_data[pos] : nullptr;
}
uint8_t* Packet::getData(std::size_t pos)
{
    return (pos < this->_g_data.size()) ? &this->_g_data[pos] : nullptr;
}
const uint8_t* Packet::getData() const
{
    return this->_g_data.data();
}
uint8_t* Packet::getData()
{
    return this->_g_data.data();
}

std::size_t Packet::getDataSize() const
{
    return this->_g_data.size();
}
uint32_t Packet::getLength() const
{
    uint32_t result = 0;
    this->unpack(this->_g_readPos, &result, sizeof(uint32_t));
    return result;
}

void Packet::setValidity(bool validity)
{
    this->_g_valid = validity;
}
bool Packet::isValid() const
{
    return this->_g_valid;
}
Packet::operator bool() const
{
    return this->_g_valid;
}
bool Packet::endReached() const
{
    return this->_g_readPos >= this->_g_data.size();
}

///

fge::net::Packet& Packet::operator <<(const char* data)
{
    fge::net::SizeType length = static_cast<fge::net::SizeType>( std::strlen(data) );

    this->pack(&length, sizeof(length));
    return this->append(data, sizeof(char) * length);
}
fge::net::Packet& Packet::operator <<(const std::string& data)
{
    fge::net::SizeType length = static_cast<fge::net::SizeType>( data.size() );

    this->pack(&length, sizeof(length));
    return this->append(data.data(), sizeof(std::string::value_type) * length);
}
fge::net::Packet& Packet::operator <<(const wchar_t* data)
{
    fge::net::SizeType length = static_cast<fge::net::SizeType>( std::wcslen(data) );

    this->pack(&length, sizeof(length));

    for (fge::net::SizeType i=0; i<length; ++i)
    {
        uint32_t buff = data[i];
        this->pack(&buff, sizeof(uint32_t));
    }
    return *this;
}
fge::net::Packet& Packet::operator <<(const std::wstring& data)
{
    fge::net::SizeType length = static_cast<fge::net::SizeType>( data.size() );

    this->pack(&length, sizeof(length));

    for (fge::net::SizeType i=0; i<length; ++i)
    {
        uint32_t buff = data[i];
        this->pack(&buff, sizeof(uint32_t));
    }
    return *this;
}
fge::net::Packet& Packet::operator <<(const sf::String& data)
{
    fge::net::SizeType length = static_cast<fge::net::SizeType>( data.getSize() );

    this->pack(&length, sizeof(length));

    for (fge::net::SizeType i=0; i<length; ++i)
    {
        uint32_t buff = data[i];
        this->pack(&buff, sizeof(uint32_t));
    }
    return *this;
}

fge::net::Packet& Packet::operator >>(char* data)
{
    fge::net::SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->_g_readPos + length - 1) < this->_g_data.size())
        {
            this->read(data, sizeof(char)*length);
            data[length] = '\0';
        }
        else
        {
            this->_g_valid = false;
        }
    }
    else
    {
        data[0] = '\0';
    }
    return *this;
}
fge::net::Packet& Packet::operator >>(std::string& data)
{
    fge::net::SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->_g_readPos + length - 1) < this->_g_data.size())
        {
            data.clear();
            data.assign(reinterpret_cast<char*>(&this->_g_data[this->_g_readPos]), length);

            this->_g_readPos += length;
        }
        else
        {
            this->_g_valid = false;
        }
    }
    else
    {
        data.clear();
    }
    return *this;
}
fge::net::Packet& Packet::operator >>(wchar_t* data)
{
    fge::net::SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->_g_readPos + (length - 1) * sizeof(uint32_t) ) < this->_g_data.size())
        {
            for (fge::net::SizeType i = 0; i<length; ++i)
            {
                uint32_t buff = 0;
                this->unpack(&buff, sizeof(uint32_t));
                data[i] = static_cast<wchar_t>(buff);
            }
            data[length] = L'\0';
        }
        else
        {
            this->_g_valid = false;
        }
    }
    else
    {
        data[0] = L'\0';
    }
    return *this;
}
fge::net::Packet& Packet::operator >>(std::wstring& data)
{
    fge::net::SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->_g_readPos + (length - 1) * sizeof(uint32_t) ) < this->_g_data.size())
        {
            data.resize(length);
            for (fge::net::SizeType i = 0; i<length; ++i)
            {
                uint32_t buff = 0;
                this->unpack(&buff, sizeof(uint32_t));
                data[i] = static_cast<wchar_t>(buff);
            }
        }
        else
        {
            this->_g_valid = false;
        }
    }
    else
    {
        data.clear();
    }
    return *this;
}
fge::net::Packet& Packet::operator >>(sf::String& data)
{
    fge::net::SizeType length = 0;
    this->unpack(&length, sizeof(length));

    if (length > 0)
    {
        if ((this->_g_readPos + (length - 1) * sizeof(uint32_t) ) < this->_g_data.size())
        {
            for (fge::net::SizeType i = 0; i<length; ++i)
            {
                uint32_t buff = 0;
                this->unpack(&buff, sizeof(uint32_t));
                data += buff;
            }
        }
        else
        {
            this->_g_valid = false;
        }
    }
    else
    {
        data.clear();
    }
    return *this;
}

void Packet::onSend(std::vector<uint8_t>& buffer, std::size_t offset)
{
    this->_g_lastDataValidity = true;
    buffer.resize(this->_g_data.size() + offset);
    for (std::size_t i=0; i<this->_g_data.size(); ++i)
    {
        buffer[i + offset] = this->_g_data[i];
    }
}
void Packet::onReceive(void* data, std::size_t dsize)
{
    this->append(data, dsize);
}

std::size_t Packet::_defaultReserveSize = FGE_PACKET_DEFAULT_RESERVESIZE;

}//end net
}//end fge
