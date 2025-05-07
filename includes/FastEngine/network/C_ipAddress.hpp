/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_C_IPADDRESS_HPP_INCLUDED_
#define _FGE_C_IPADDRESS_HPP_INCLUDED_

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#ifndef _WIN32
    #undef None
#endif

#define FGE_ANYPORT 0

namespace fge::net
{
class IpAddress;
class Packet;
} // namespace fge::net

template<>
struct std::hash<fge::net::IpAddress>;

namespace fge::net
{

using Port = uint16_t;

/**
 * \class IpAddress
 * \ingroup network
 * \brief A class to represent an IP address
 *
 * This class will automatically handle the byte order of the IP address.
 */
class FGE_API IpAddress
{
public:
    using Ipv4Data = uint32_t;
    using Ipv6Data = std::array<uint16_t, 8>;
    using Data = std::variant<Ipv4Data, Ipv6Data>;

    enum class Types : uint8_t
    {
        None,
        Ipv4,
        Ipv6
    };

    enum class CheckHostname
    {
        No,
        Yes
    };

    /**
     * \brief Build a default invalid IP address
     */
    IpAddress() noexcept;
    /**
     * \brief Build an address from a string
     *
     * The string can be in the ipv4 form XXX.XXX.XXX.XXX, ipv6, or a hostname.
     *
     * \param address A string representing the address
     * \param check Control if the method should check if the address is a hostname or not
     */
    IpAddress(std::string const& address, CheckHostname check = CheckHostname::Yes);
    IpAddress(char const* address, CheckHostname check = CheckHostname::Yes);
    /**
     * \brief Build an ipv4 address from 4 bytes
     *
     * \param byte3 The most significant byte
     * \param byte2 The second most significant byte
     * \param byte1 The second least significant byte
     * \param byte0 The least significant byte
     */
    IpAddress(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0) noexcept;
    /**
     * \brief Build an ipv4 address from 8 words in host byte order
     *
     * \warning the first word is the most significant word.
     *
     * \param words The 8 words representing the ipv6 address in host byte order
     */
    IpAddress(std::initializer_list<uint16_t> words) noexcept;
    /**
     * \brief Build an ipv4 address from Ipv6Data in host byte order
     *
     * \warning the first word is the least significant word.
     *
     * \param data The Ipv6Data representing the address in host byte order
     */
    IpAddress(Ipv6Data const& data) noexcept;
    /**
     * \brief Build an ipv4 address from a host byte order integer
     *
     * \param address The host byte order integer
     */
    IpAddress(Ipv4Data address) noexcept;
    ~IpAddress() = default;

    /**
     * \brief Build an address from a string
     *
     * The string can be in the ipv4 form XXX.XXX.XXX.XXX, ipv6, or a hostname.
     *
     * \param address A string representing the address
     * \param check Control if the method should check if the address is a hostname or not
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(std::string const& address, CheckHostname check = CheckHostname::Yes);
    bool set(char const* address, CheckHostname check = CheckHostname::Yes);
    /**
     * \brief Build an ipv4 address from 4 bytes
     *
     * \param byte3 The most significant byte
     * \param byte2 The second most significant byte
     * \param byte1 The second least significant byte
     * \param byte0 The least significant byte
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0);
    /**
     * \brief Build an ipv6 address from 8 words in host byte order
     *
     * Manualy build an ipv6 address with a initializer list.
     * \warning the first word is the most significant word.
     *
     * \param words The 8 words representing the ipv6 address in host byte order
     */
    bool set(std::initializer_list<uint16_t> words);
    /**
     * \brief Build an ipv6 address from Ipv6Data in host byte order
     *
     * \warning Contrary to the initializer list, the first word is the least significant word.
     *
     * \param data The Ipv6Data representing the ipv6 address in host byte order
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(Ipv6Data const& data);
    /**
     * \brief Build an ipv6 address from a host byte order array of bytes
     *
     * \param bytes The host byte order array
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(uint8_t const bytes[16]);
    /**
     * \brief Build an ipv4 address from a host byte order integer
     *
     * \param address The host byte order integer
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(Ipv4Data address);
    /**
     * \brief Build an ipv4 address from a network byte order integer
     *
     * \param address The network byte order integer
     * \return \b true if the address is valid, \b false otherwise
     */
    bool setNetworkByteOrdered(Ipv4Data address);
    /**
     * \brief Build an ipv6 address from a network byte order data
     *
     * \param data The network byte order data
     * \return \b true if the address is valid, \b false otherwise
     */
    bool setNetworkByteOrdered(Ipv6Data const& data);
    /**
     * \brief Build an ipv6 address from a network byte order array of bytes
     *
     * \param bytes The network byte order array
     * \return \b true if the address is valid, \b false otherwise
     */
    bool setNetworkByteOrdered(uint8_t const bytes[16]);

    [[nodiscard]] bool operator==(IpAddress const& r) const;

    /**
     * \brief Get the ip address in a string format
     *
     * \return A string representing the ip address
     */
    [[nodiscard]] std::optional<std::string> toString() const;

    /**
     * \brief Get the ip address in a network byte order
     *
     * \return The network byte order integer
     */
    [[nodiscard]] std::optional<Data> getNetworkByteOrder() const;
    /**
     * \brief Get the ip address in host byte order
     *
     * \return The host byte order integer
     */
    [[nodiscard]] std::optional<Data> getHostByteOrder() const;

    [[nodiscard]] Types getType() const;

    /**
     * \brief Map an ipv4 address to an ipv6 address
     *
     * When using a socket that can communicate with both ipv4 and ipv6, you have to map the ipv4 address to an ipv6 address.
     *
     * FROM MSDN:
     *
     * Dual-stack sockets always require IPv6 addresses. The ability to interact with an IPv4 address requires the use of the IPv4-mapped IPv6 address format.
     * Any IPv4 addresses must be represented in the IPv4-mapped IPv6 address format which enables an IPv6 only application to communicate with an IPv4 node.
     * The IPv4-mapped IPv6 address format allows the IPv4 address of an IPv4 node to be represented as an IPv6 address.
     * The IPv4 address is encoded into the low-order 32 bits of the IPv6 address, and the high-order 96 bits hold the fixed prefix 0:0:0:0:0:FFFF.
     * The IPv4-mapped IPv6 address format is specified in RFC 4291.
     *
     * \return The mapped ipv6 address or an empty optional if the address is not an ipv4 address
     */
    [[nodiscard]] std::optional<IpAddress> mapToIpv6() const;
    /**
     * \brief Map an ipv6 (ipv4-mapped) address back to an ipv4 address
     *
     * \return The mapped ipv4 address or an empty optional if the address is not an ipv6 address
     */
    [[nodiscard]] std::optional<IpAddress> mapToIpv4() const;
    [[nodiscard]] bool isIpv4MappedIpv6() const;

    /**
     * \brief Get the standard hostname for the local computer
     *
     * \return The hostname
     */
    [[nodiscard]] static std::optional<std::string> getHostName();
    /**
     * \brief Get a list of local IpAddress of the local computer
     *
     * \param type The type of address to get, if None, all addresses will be returned
     * \return A vector containing the local IP addresses or an empty vector if an error occurred
     */
    [[nodiscard]] static std::vector<IpAddress> getLocalAddresses(Types type = Types::None);

    static IpAddress const None; ///< Represent an invalid address

    static IpAddress const Ipv4Any; ///< Represent an unspecified ipv4 address "0.0.0.0"
    static IpAddress const Ipv6Any; ///< Represent an unspecified ipv6 address "::"
    static IpAddress Any(Types addressType);

    static IpAddress const Ipv4Loopback; ///< Represent the local host ipv4 address "127.0.0.1"
    static IpAddress const Ipv6Loopback; ///< Represent the local host ipv6 address "::1"
    static IpAddress Loopback(Types addressType);

    static IpAddress const Ipv4Broadcast; ///< Represent the broadcast ipv4 address "255.255.255.255"

private:
    std::variant<std::monostate, Ipv4Data, Ipv6Data> g_address; ///< Network byte order address

    friend struct std::hash<IpAddress>;
};

FGE_API Packet const& operator>>(Packet const& pck, IpAddress& data);
FGE_API Packet& operator<<(Packet& pck, IpAddress const& data);

} // namespace fge::net

template<>
struct std::hash<fge::net::IpAddress>
{
    inline std::size_t operator()(fge::net::IpAddress const& r) const noexcept
    {
        if (std::holds_alternative<std::monostate>(r.g_address))
        {
            return std::hash<std::monostate>{}(std::monostate{});
        }
        if (std::holds_alternative<fge::net::IpAddress::Ipv4Data>(r.g_address))
        {
            return std::hash<fge::net::IpAddress::Ipv4Data>{}(std::get<fge::net::IpAddress::Ipv4Data>(r.g_address));
        }
        auto const& array = std::get<fge::net::IpAddress::Ipv6Data>(r.g_address);
        return fge::Hash(array.data(), array.size() * 2);
    }
};

#endif // _FGE_C_IPADDRESS_HPP_INCLUDED_
