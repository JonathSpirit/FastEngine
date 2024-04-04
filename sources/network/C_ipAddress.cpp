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

#include "FastEngine/network/C_ipAddress.hpp"
#include "FastEngine/fge_endian.hpp"
#include <cstring>

#ifdef _WIN32
    #ifdef _WIN32_WINDOWS
        #undef _WIN32_WINDOWS
    #endif // _WIN32_WINDOWS
    #ifdef WINVER
        #undef WINVER
    #endif // WINVER
    #ifdef _WIN32_WINNT
        #undef _WIN32_WINNT
    #endif // _WIN32_WINNT

    #define _WIN32_WINDOWS _WIN32_WINNT_VISTA
    #define WINVER _WIN32_WINNT_VISTA
    #define _WIN32_WINNT _WIN32_WINNT_VISTA

    #include <winsock2.h>
    #include <iphlpapi.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif // _WIN32

#ifdef _WIN32
    #define _FGE_SOCKET_INVALID INVALID_SOCKET
    #define _FGE_SOCKET_ERROR SOCKET_ERROR
#else
    #define _FGE_SOCKET_INVALID -1
    #define _FGE_SOCKET_ERROR -1
#endif

namespace fge::net
{

IpAddress const IpAddress::None;

IpAddress const IpAddress::Ipv4Any(0, 0, 0, 0);
IpAddress const IpAddress::Ipv6Any{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
IpAddress IpAddress::Any(Types addressType)
{
    if (addressType == Types::Ipv4 || addressType == Types::None)
    {
        return Ipv4Any;
    }
    return Ipv6Any;
}

IpAddress const IpAddress::Ipv4Loopback(127, 0, 0, 1);
IpAddress const IpAddress::Ipv6Loopback{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001};
IpAddress IpAddress::Loopback(Types addressType)
{
    if (addressType == Types::Ipv4 || addressType == Types::None)
    {
        return Ipv4Loopback;
    }
    return Ipv6Loopback;
}

IpAddress const IpAddress::Ipv4Broadcast(255, 255, 255, 255);

IpAddress::IpAddress() noexcept :
        g_address(std::monostate{})
{}
IpAddress::IpAddress(std::string const& address) :
        g_address(std::monostate{})
{
    this->set(address.c_str());
}
IpAddress::IpAddress(char const* address) :
        g_address(std::monostate{})
{
    this->set(address);
}
IpAddress::IpAddress(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0) noexcept :
        g_address(fge::SwapHostNetEndian_32((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0))
{}
IpAddress::IpAddress(std::initializer_list<uint16_t> words) noexcept :
        g_address(std::monostate{})
{
    this->set(words);
}
IpAddress::IpAddress(uint16_t const words[8]) noexcept :
        g_address(std::monostate{})
{
    this->set(words);
}
IpAddress::IpAddress(uint32_t address) noexcept :
        g_address(fge::SwapHostNetEndian_32(address))
{}

bool IpAddress::set(std::string const& address)
{
    return this->set(address.c_str());
}
bool IpAddress::set(char const* address)
{
    if (std::strcmp(address, "255.255.255.255") == 0)
    { //Ipv4 broadcast
        this->g_address = static_cast<Ipv4Data>(INADDR_BROADCAST);
        return true;
    }

    if (std::strcmp(address, "0.0.0.0") == 0)
    { //Ipv4 any
        this->g_address = static_cast<Ipv4Data>(INADDR_ANY);
        return true;
    }

    { //Ipv4 as string
        in_addr outIp{};

        if (inet_pton(AF_INET, address, &outIp) == 1)
        {
            this->g_address = static_cast<Ipv4Data>(outIp.s_addr);
            return true;
        }
    }

    { //Ipv6 as string
        in6_addr outIp{};

        if (inet_pton(AF_INET6, address, &outIp) == 1)
        {
            auto const* data = outIp.s6_addr;
            std::memcpy(this->g_address.emplace<Ipv6Data>().data(), data, 16);
            return true;
        }
    }

    //Maybe host name
    addrinfo* result = nullptr;

    if (getaddrinfo(address, nullptr, nullptr, &result) == 0 && result != nullptr)
    {
        if (result->ai_family == AF_INET)
        {
            this->g_address = static_cast<Ipv4Data>(reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr);
        }
        else if (result->ai_family == AF_INET6)
        {
            auto const* data = reinterpret_cast<sockaddr_in6*>(result->ai_addr)->sin6_addr.s6_addr;
            std::memcpy(this->g_address.emplace<Ipv6Data>().data(), data, 16);
        }

        freeaddrinfo(result);
        return !std::holds_alternative<std::monostate>(this->g_address);
    }

    //Invalid address
    this->g_address = std::monostate{};
    return false;
}
bool IpAddress::set(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0)
{
    this->g_address = fge::SwapHostNetEndian_32((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
    return true;
}
bool IpAddress::set(std::initializer_list<uint16_t> words)
{
    if (words.size() != 8)
    {
        this->g_address = std::monostate{};
        return false;
    }

    auto& array = this->g_address.emplace<Ipv6Data>();
    std::size_t i = 0;
    for (auto const n: words)
    {
        array[i++] = fge::SwapHostNetEndian_16(n);
    }
    return true;
}
bool IpAddress::set(uint16_t const words[8])
{
    auto& array = this->g_address.emplace<Ipv6Data>();
    for (std::size_t i = 8; i > 0; --i)
    {
        array[i - 1] = fge::SwapHostNetEndian_16(words[8 - i]);
    }
    return true;
}
bool IpAddress::set(uint32_t address)
{
    this->g_address = fge::SwapHostNetEndian_32(address);
    return true;
}
bool IpAddress::setNetworkByteOrdered(uint32_t address)
{
    this->g_address = address;
    return true;
}
bool IpAddress::setNetworkByteOrdered(uint16_t const words[8])
{
    std::memcpy(this->g_address.emplace<Ipv6Data>().data(), words, 16);
    return true;
}
bool IpAddress::setNetworkByteOrdered(uint8_t const words[16])
{
    std::memcpy(this->g_address.emplace<Ipv6Data>().data(), words, 16);
    return true;
}

bool IpAddress::operator==(IpAddress const& r) const
{
    return this->g_address == r.g_address;
}

std::optional<std::string> IpAddress::toString() const
{
    if (std::holds_alternative<std::monostate>(this->g_address))
    {
        return std::nullopt;
    }

    if (std::holds_alternative<Ipv4Data>(this->g_address))
    {
        std::string result(INET_ADDRSTRLEN, '\0');

        in_addr address{};
        address.s_addr = std::get<Ipv4Data>(this->g_address);

        if (inet_ntop(AF_INET, &address, result.data(), result.size()) != nullptr)
        {
            auto firstNull = result.find_first_of('\0');
            result.resize(firstNull);
            return result;
        }
        return std::nullopt;
    }

    std::string result(INET6_ADDRSTRLEN, '\0');

    in6_addr address{};
    std::memcpy(&address.s6_addr, std::get<Ipv6Data>(this->g_address).data(), 16);

    if (inet_ntop(AF_INET6, &address, result.data(), result.size()) != nullptr)
    {
        auto firstNull = result.find_first_of('\0');
        result.resize(firstNull);
        return result;
    }
    return std::nullopt;
}

std::optional<IpAddress::Data> IpAddress::getNetworkByteOrder() const
{
    if (std::holds_alternative<std::monostate>(this->g_address))
    {
        return std::nullopt;
    }

    if (std::holds_alternative<Ipv4Data>(this->g_address))
    {
        return std::get<Ipv4Data>(this->g_address);
    }
    return std::get<Ipv6Data>(this->g_address);
}
std::optional<IpAddress::Data> IpAddress::getHostByteOrder() const
{
    if (std::holds_alternative<std::monostate>(this->g_address))
    {
        return std::nullopt;
    }

    if (std::holds_alternative<Ipv4Data>(this->g_address))
    {
        return fge::SwapHostNetEndian_32(std::get<Ipv4Data>(this->g_address));
    }
    auto data = std::get<Ipv6Data>(this->g_address);
    return std::array{fge::SwapHostNetEndian_16(data[7]), fge::SwapHostNetEndian_16(data[6]),
                      fge::SwapHostNetEndian_16(data[5]), fge::SwapHostNetEndian_16(data[4]),
                      fge::SwapHostNetEndian_16(data[3]), fge::SwapHostNetEndian_16(data[2]),
                      fge::SwapHostNetEndian_16(data[1]), fge::SwapHostNetEndian_16(data[0])};
}

IpAddress::Types IpAddress::getType() const
{
    if (std::holds_alternative<Ipv4Data>(this->g_address))
    {
        return Types::Ipv4;
    }
    if (std::holds_alternative<Ipv6Data>(this->g_address))
    {
        return Types::Ipv6;
    }
    return Types::None;
}

std::optional<std::string> IpAddress::getHostName()
{
    /*
     * https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-gethostname
     * The maximum length, in bytes, of the string returned in the buffer pointed to by the name parameter is
     * dependent on the namespace provider, but this string must be 256 bytes or less. So if a buffer of 256 bytes is
     * passed in the name parameter and the namelen parameter is set to 256, the buffer size will always be adequate.
     */
    std::string name(256, '\0');
    if (gethostname(name.data(), static_cast<int>(name.size())) == _FGE_SOCKET_ERROR)
    {
        return std::nullopt;
    }
    auto firstNull = name.find_first_of('\0');
    name.resize(firstNull);
    return name;
}

std::vector<IpAddress> IpAddress::getLocalAddresses(Types type)
{
    std::vector<IpAddress> buff;

    addrinfo* result = nullptr;

    if (getaddrinfo("", nullptr, nullptr, &result) == 0 && result != nullptr)
    {
        addrinfo const* currentResult = result;

        do {
            if (currentResult->ai_family == AF_INET && (type == Types::None || type == Types::Ipv4))
            {
                IpAddress& ip = buff.emplace_back();
                ip.g_address =
                        static_cast<Ipv4Data>(reinterpret_cast<sockaddr_in*>(currentResult->ai_addr)->sin_addr.s_addr);
            }
            else if (currentResult->ai_family == AF_INET6 && (type == Types::None || type == Types::Ipv6))
            {
                IpAddress& ip = buff.emplace_back();
                auto* data = reinterpret_cast<sockaddr_in6*>(currentResult->ai_addr)->sin6_addr.s6_addr;
                std::memcpy(ip.g_address.emplace<Ipv6Data>().data(), data, 16);
            }

            currentResult = result->ai_next;
        } while (currentResult != nullptr);

        freeaddrinfo(result);
    }

    return buff;
}

} // namespace fge::net
