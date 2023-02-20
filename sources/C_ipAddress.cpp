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

#include "FastEngine/C_ipAddress.hpp"
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

    #define _WIN32_WINDOWS _WIN32_WINNT_WINXP
    #define WINVER _WIN32_WINNT_WINXP
    #define _WIN32_WINNT _WIN32_WINNT_WINXP

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

const fge::net::IpAddress IpAddress::None{};
const fge::net::IpAddress IpAddress::Any{0, 0, 0, 0};
const fge::net::IpAddress IpAddress::LocalHost{127, 0, 0, 1};
const fge::net::IpAddress IpAddress::Broadcast{255, 255, 255, 255};

IpAddress::IpAddress() noexcept :
        g_address(0),
        g_valid(false)
{}
IpAddress::IpAddress(const std::string& address) :
        g_address(0),
        g_valid(false)
{
    this->set(address.c_str());
}
IpAddress::IpAddress(const char* address) :
        g_address(0),
        g_valid(false)
{
    this->set(address);
}
IpAddress::IpAddress(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0) noexcept :
        g_address(fge::SwapHostNetEndian_32((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0)),
        g_valid(true)
{}
IpAddress::IpAddress(uint32_t address) noexcept :
        g_address(fge::SwapHostNetEndian_32(address)),
        g_valid(true)
{}

bool IpAddress::set(const std::string& address)
{
    return this->set(address.c_str());
}
bool IpAddress::set(const char* address)
{
    if (std::strcmp(address, "255.255.255.255") == 0)
    { //Broadcast
        this->g_address = INADDR_BROADCAST;
        this->g_valid = true;
        return true;
    }

    if (std::strcmp(address, "0.0.0.0") == 0)
    { //Any
        this->g_address = INADDR_ANY;
        this->g_valid = true;
        return true;
    }

    //IP as string xxx.xxx.xxx.xxx
    uint32_t ip = inet_addr(address);

    if (ip != INADDR_NONE)
    {
        this->g_address = ip;
        this->g_valid = true;
        return true;
    }

    //Maybe host name
    addrinfo hints{};
    hints.ai_family = AF_INET;
    addrinfo* result = nullptr;

    if (getaddrinfo(address, nullptr, &hints, &result) == 0)
    {
        if (result != nullptr)
        {
            ip = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
            freeaddrinfo(result);

            this->g_address = ip;
            this->g_valid = true;
            return true;
        }
    }

    //Invalid address
    this->g_address = 0;
    this->g_valid = false;
    return false;
}
bool IpAddress::set(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0)
{
    this->g_address = fge::SwapHostNetEndian_32((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
    this->g_valid = true;
    return true;
}
bool IpAddress::set(uint32_t address)
{
    this->g_address = fge::SwapHostNetEndian_32(address);
    this->g_valid = true;
    return true;
}
bool IpAddress::setNetworkByteOrdered(uint32_t address)
{
    this->g_address = address;
    this->g_valid = true;
    return true;
}

bool IpAddress::operator==(const fge::net::IpAddress& r) const
{
    return (this->g_address == r.g_address) && (this->g_valid == r.g_valid);
}

std::string IpAddress::toString() const
{
    in_addr address{};
    address.s_addr = this->g_address;

    return {inet_ntoa(address)};
}

uint32_t IpAddress::getNetworkByteOrder() const
{
    return this->g_address;
}
uint32_t IpAddress::getHostByteOrder() const
{
    return fge::SwapHostNetEndian_32(this->g_address);
}

std::string IpAddress::getHostName()
{
    char name[80];
    if (gethostname(name, sizeof(name)) == _FGE_SOCKET_ERROR)
    {
        return {};
    }
    return {name};
}

void IpAddress::getLocalAddresses(std::vector<fge::net::IpAddress>& buff)
{
    buff.clear();

    addrinfo hints{};
    hints.ai_family = AF_INET;
    addrinfo* result = nullptr;

    if (getaddrinfo("", nullptr, &hints, &result) == 0)
    {
        if (result != nullptr)
        {
            addrinfo* ptr = result;

            do {
                fge::net::IpAddress ip;
                ip.g_address = reinterpret_cast<sockaddr_in*>(ptr->ai_addr)->sin_addr.s_addr;
                ip.g_valid = true;

                buff.push_back(ip);

                ptr = ptr->ai_next;
            } while (ptr != nullptr);

            freeaddrinfo(result);
        }
    }
}

} // namespace fge::net
