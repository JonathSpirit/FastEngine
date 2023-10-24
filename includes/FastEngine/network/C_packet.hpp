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

#ifndef _FGE_C_PACKET_HPP_INCLUDED_
#define _FGE_C_PACKET_HPP_INCLUDED_

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fge_extern.hpp"
#include "C_ipAddress.hpp"
#include "FastEngine/C_matrix.hpp"
#include "tinyutf8.h"
#include <cstdint>
#include <forward_list>
#include <list>
#include <string>
#include <vector>

#include "FastEngine/C_vector.hpp"
#include "FastEngine/graphic/C_color.hpp"

#define FGE_PACKET_DEFAULT_RESERVESIZE 4096

namespace fge::net
{

struct Error
{
    enum class Types
    {
        ERR_NONE,
        ERR_ALREADY_INVALID,
        ERR_EXTRACT,
        ERR_RULE,

        ERR_SCENE_NEED_CACHING,
        ERR_SCENE_OLD_PACKET
    };

    Types _type{Types::ERR_NONE};
    std::size_t _readPos{0};
    char const* _error{nullptr};
    char const* _function{nullptr};
};

class SocketTcp;
class SocketUdp;

using SizeType = uint16_t;

class FGE_API Packet
{
public:
    Packet();
    Packet(fge::net::Packet&& pck) noexcept;
    Packet(fge::net::Packet& pck) = default;
    Packet(fge::net::Packet const& pck) = default;
    explicit Packet(std::size_t reserveSize);
    virtual ~Packet() = default;

    void clear();
    void flush();
    void reserve(std::size_t reserveSize);

    fge::net::Packet& append(std::size_t size); //Will push to host byte order without data (increasing size)
    fge::net::Packet& append(void const* data, std::size_t size); //Will push to host byte order
    fge::net::Packet& pack(void const* data, std::size_t size);   //Will push and auto convert to network byte order

    bool write(std::size_t pos, void const* data, std::size_t size); //Will write to host byte order
    bool pack(std::size_t pos, void const* data, std::size_t size);  //Will write and auto convert to network byte order

    fge::net::Packet const& read(void* buff, std::size_t size) const;   //Will read to network byte order
    fge::net::Packet const& unpack(void* buff, std::size_t size) const; //Will read and auto convert to host byte order

    bool read(std::size_t pos, void* buff, std::size_t size) const;   //Will read to network byte order
    bool unpack(std::size_t pos, void* buff, std::size_t size) const; //Will read and auto convert to host byte order

    fge::net::Packet& shrink(std::size_t size);
    bool erase(std::size_t pos, std::size_t size);
    fge::net::Packet const& skip(std::size_t size) const;

    void setReadPos(std::size_t pos) const;
    [[nodiscard]] std::size_t getReadPos() const;
    [[nodiscard]] bool isExtractable(std::size_t size) const;

    [[nodiscard]] uint8_t const* getData(std::size_t pos) const; //Get data pointer to pos
    [[nodiscard]] uint8_t* getData(std::size_t pos);
    [[nodiscard]] uint8_t const* getData() const; //Get data pointer to pos 0
    [[nodiscard]] uint8_t* getData();

    [[nodiscard]] std::size_t getDataSize() const;
    [[nodiscard]] uint32_t getLength() const; //Get length of a string or others at the read position
                                              //can be useful to allocate a char buffer before reading
    void invalidate() const;
    void setValidity(bool validity) const;
    [[nodiscard]] bool isValid() const;
    [[nodiscard]] explicit operator bool() const;
    [[nodiscard]] bool endReached() const;

    inline fge::net::Packet& operator<<(bool data);

    inline fge::net::Packet& operator<<(int8_t data);
    inline fge::net::Packet& operator<<(int16_t data);
    inline fge::net::Packet& operator<<(int32_t data);
    inline fge::net::Packet& operator<<(int64_t data);

    inline fge::net::Packet& operator<<(uint8_t data);
    inline fge::net::Packet& operator<<(uint16_t data);
    inline fge::net::Packet& operator<<(uint32_t data);
    inline fge::net::Packet& operator<<(uint64_t data);

    inline fge::net::Packet& operator<<(float data);
    inline fge::net::Packet& operator<<(double data);
    inline fge::net::Packet& operator<<(long double data);

    fge::net::Packet& operator<<(std::string_view const& data);
    fge::net::Packet& operator<<(char const* data);
    fge::net::Packet& operator<<(std::string const& data);
    fge::net::Packet& operator<<(tiny_utf8::string const& data);
    fge::net::Packet& operator<<(wchar_t const* data);
    fge::net::Packet& operator<<(std::wstring const& data);

    inline fge::net::Packet& operator<<(fge::net::IpAddress const& data);

    template<typename T>
    fge::net::Packet& operator<<(std::forward_list<T> const& data);
    template<typename T>
    fge::net::Packet& operator<<(std::list<T> const& data);
    template<typename T>
    fge::net::Packet& operator<<(std::vector<T> const& data);

    template<typename T>
    fge::net::Packet& operator<<(fge::Vector2<T> const& data);
    template<typename T>
    fge::net::Packet& operator<<(fge::Vector3<T> const& data);

    template<typename T>
    fge::net::Packet& operator<<(fge::Matrix<T> const& data);

    inline fge::net::Packet& operator<<(fge::Color const& data);

    ///

    inline fge::net::Packet const& operator>>(bool& data) const;

    inline fge::net::Packet const& operator>>(int8_t& data) const;
    inline fge::net::Packet const& operator>>(int16_t& data) const;
    inline fge::net::Packet const& operator>>(int32_t& data) const;
    inline fge::net::Packet const& operator>>(int64_t& data) const;

    inline fge::net::Packet const& operator>>(uint8_t& data) const;
    inline fge::net::Packet const& operator>>(uint16_t& data) const;
    inline fge::net::Packet const& operator>>(uint32_t& data) const;
    inline fge::net::Packet const& operator>>(uint64_t& data) const;

    inline fge::net::Packet const& operator>>(float& data) const;
    inline fge::net::Packet const& operator>>(double& data) const;
    inline fge::net::Packet const& operator>>(long double& data) const;

    fge::net::Packet const& operator>>(char* data) const;
    fge::net::Packet const& operator>>(std::string& data) const;
    fge::net::Packet const& operator>>(tiny_utf8::string& data) const;
    fge::net::Packet const& operator>>(wchar_t* data) const;
    fge::net::Packet const& operator>>(std::wstring& data) const;

    inline fge::net::Packet const& operator>>(fge::net::IpAddress& data) const;

    template<typename T>
    fge::net::Packet const& operator>>(std::forward_list<T>& data) const;
    template<typename T>
    fge::net::Packet const& operator>>(std::list<T>& data) const;
    template<typename T>
    fge::net::Packet const& operator>>(std::vector<T>& data) const;

    template<typename T>
    fge::net::Packet const& operator>>(fge::Vector2<T>& data) const;
    template<typename T>
    fge::net::Packet const& operator>>(fge::Vector3<T>& data) const;

    template<typename T>
    fge::net::Packet const& operator>>(fge::Matrix<T>& data) const;

    inline fge::net::Packet const& operator>>(fge::Color& data) const;

    bool operator==(Packet const& right) const = delete;
    bool operator!=(Packet const& right) const = delete;

    static std::size_t _defaultReserveSize;

    virtual void onSend(std::vector<uint8_t>& buffer, std::size_t offset);
    virtual void onReceive(void* data, std::size_t size);

protected:
    friend class fge::net::SocketTcp;
    friend class fge::net::SocketUdp;

    std::size_t _g_sendPos;
    std::vector<uint8_t> _g_lastData;
    bool _g_lastDataValidity;

    std::vector<uint8_t> _g_data;
    mutable std::size_t _g_readPos;
    mutable bool _g_valid;
};

} // namespace fge::net

#include "C_packet.inl"

#endif // _FGE_C_PACKET_HPP_INCLUDED_
