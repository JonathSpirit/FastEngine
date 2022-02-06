#ifndef _FGE_C_PACKET_HPP_INCLUDED_
#define _FGE_C_PACKET_HPP_INCLUDED_

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_matrix.hpp>
#include <FastEngine/C_ipAddress.hpp>
#include <string>
#include <vector>
#include <list>
#include <forward_list>
#include <cstdint>
#include <stdexcept>

#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Color.hpp>

#define FGE_PACKET_DEFAULT_RESERVESIZE 2048

namespace fge
{
namespace net
{

class SocketTcp;
class SocketUdp;

class FGE_API Packet
{
public:
    Packet();
    Packet(fge::net::Packet&& pck) noexcept;
    Packet(fge::net::Packet& pck) = default;
    explicit Packet(std::size_t reserveSize);
    virtual ~Packet() = default;

    void clear();
    void flush();
    void reserve(std::size_t reserveSize);

    fge::net::Packet& append(const void* data, std::size_t dsize); //Will push to host byte order
    fge::net::Packet& pack(const void* data, std::size_t dsize); //Will push and auto convert to network byte order

    bool write(std::size_t pos, const void* data, std::size_t dsize); //Will write to host byte order
    bool pack(std::size_t pos, const void* data, std::size_t dsize); //Will write and auto convert to network byte order

    fge::net::Packet& read(void* buff, std::size_t bsize); //Will read to network byte order
    fge::net::Packet& unpack(void* buff, std::size_t bsize); //Will read and auto convert to host byte order

    bool read(std::size_t pos, void* buff, std::size_t bsize) const; //Will read to network byte order
    bool unpack(std::size_t pos, void* buff, std::size_t bsize) const; //Will read and auto convert to host byte order

    void setReadPos(std::size_t pos);
    [[nodiscard]] std::size_t getReadPos() const;

    [[nodiscard]] const uint8_t* getData(std::size_t pos) const; //Get data pointer to pos
    [[nodiscard]] uint8_t* getData(std::size_t pos);
    [[nodiscard]] const uint8_t* getData() const; //Get data pointer to pos 0
    [[nodiscard]] uint8_t* getData();

    [[nodiscard]] std::size_t getDataSize() const;
    [[nodiscard]] uint32_t getLength() const; //Get length of a string or others at the read position
                                //can be useful to allocate a char buffer before reading

    void setValidity(bool validity);
    [[nodiscard]] bool isValid() const;
    [[nodiscard]] operator bool() const;
    [[nodiscard]] bool endReached() const;

    inline fge::net::Packet& operator <<(bool data);

    inline fge::net::Packet& operator <<(int8_t data);
    inline fge::net::Packet& operator <<(int16_t data);
    inline fge::net::Packet& operator <<(int32_t data);
    inline fge::net::Packet& operator <<(int64_t data);

    inline fge::net::Packet& operator <<(uint8_t data);
    inline fge::net::Packet& operator <<(uint16_t data);
    inline fge::net::Packet& operator <<(uint32_t data);
    inline fge::net::Packet& operator <<(uint64_t data);

    inline fge::net::Packet& operator <<(float data);
    inline fge::net::Packet& operator <<(double data);

    fge::net::Packet& operator <<(const char* data);
    fge::net::Packet& operator <<(const std::string& data);
    fge::net::Packet& operator <<(const wchar_t* data);
    fge::net::Packet& operator <<(const std::wstring& data);
    fge::net::Packet& operator <<(const sf::String& data);

    inline fge::net::Packet& operator <<(const fge::net::IpAddress& data);

    template <typename T>
    fge::net::Packet& operator <<(const std::forward_list<T>& data);
    template <typename T>
    fge::net::Packet& operator <<(const std::list<T>& data);
    template <typename T>
    fge::net::Packet& operator <<(const std::vector<T>& data);

    template<typename T>
    fge::net::Packet& operator <<(const sf::Vector2<T>& data);
    template<typename T>
    fge::net::Packet& operator <<(const sf::Vector3<T>& data);

    template<typename T>
    fge::net::Packet& operator <<(const fge::Matrix<T>& data);

    inline fge::net::Packet& operator <<(const sf::Color& data);

    ///

    inline fge::net::Packet& operator >>(bool& data);

    inline fge::net::Packet& operator >>(int8_t& data);
    inline fge::net::Packet& operator >>(int16_t& data);
    inline fge::net::Packet& operator >>(int32_t& data);
    inline fge::net::Packet& operator >>(int64_t& data);

    inline fge::net::Packet& operator >>(uint8_t& data);
    inline fge::net::Packet& operator >>(uint16_t& data);
    inline fge::net::Packet& operator >>(uint32_t& data);
    inline fge::net::Packet& operator >>(uint64_t& data);

    inline fge::net::Packet& operator >>(float& data);
    inline fge::net::Packet& operator >>(double& data);

    fge::net::Packet& operator >>(char* data);
    fge::net::Packet& operator >>(std::string& data);
    fge::net::Packet& operator >>(wchar_t* data);
    fge::net::Packet& operator >>(std::wstring& data);
    fge::net::Packet& operator >>(sf::String& data);

    inline fge::net::Packet& operator >>(fge::net::IpAddress& data);

    template <typename T>
    fge::net::Packet& operator >>(std::forward_list<T>& data);
    template <typename T>
    fge::net::Packet& operator >>(std::list<T>& data);
    template <typename T>
    fge::net::Packet& operator >>(std::vector<T>& data);

    template<typename T>
    fge::net::Packet& operator >>(sf::Vector2<T>& data);
    template<typename T>
    fge::net::Packet& operator >>(sf::Vector3<T>& data);

    template<typename T>
    fge::net::Packet& operator >>(fge::Matrix<T>& data);

    inline fge::net::Packet& operator >>(sf::Color& data);

    bool operator ==(const Packet& right) const = delete;
    bool operator !=(const Packet& right) const = delete;

    static std::size_t _defaultReserveSize;

    virtual void onSend(std::vector<uint8_t>& buffer, std::size_t offset);
    virtual void onReceive(void* data, std::size_t dsize);

protected:
    friend class fge::net::SocketTcp;
    friend class fge::net::SocketUdp;

    std::size_t _g_sendPos;
    std::vector<uint8_t> _g_lastData;
    bool _g_lastDataValidity;

    std::vector<uint8_t> _g_data;
    std::size_t _g_readPos;
    bool _g_valid;
};

}//end net
}//end fge

#include "FastEngine/C_packet.inl"

#endif // _FGE_C_PACKET_HPP_INCLUDED_
