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

#ifndef _FGE_C_IPADDRESS_HPP_INCLUDED_
#define _FGE_C_IPADDRESS_HPP_INCLUDED_

#include "FastEngine/fge_extern.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <variant>
#include <optional>

#ifndef _WIN32
    #undef None
#endif

#define FGE_ANYPORT 0

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

    enum class Types
    {
        None,
        Ipv4,
        Ipv6
    };

    /**
     * \brief Build a default invalid IP address
     */
    IpAddress() noexcept;
    /**
     * \brief Build an address from a string
     *
     * The string can be in the form XXX.XXX.XXX.XXX, ipv6, or a hostname.
     *
     * \param address A string representing the address
     */
    IpAddress(std::string const& address);
    IpAddress(char const* address);
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
     * \brief Build an ipv4 address from 8 words
     *
     * \param words The 8 words representing the ipv6 address in host byte order
     */
    IpAddress(std::initializer_list<uint16_t> words) noexcept;
    /**
     * \brief Build an ipv4 address from a host byte order integer
     *
     * \param address The host byte order integer
     */
    IpAddress(uint32_t address) noexcept;
    ~IpAddress() = default;

    /**
     * \brief Build an address from a string
     *
     * The string can be in the form XXX.XXX.XXX.XXX, ipv6, or a hostname.
     *
     * \param address A string representing the address
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(std::string const& address);
    bool set(char const* address);
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
     * \brief Build an ipv4 address from 8 words
     *
     * \param words The 8 words representing the ipv6 address in host byte order
     */
    bool set(std::initializer_list<uint16_t> words);
    /**
     * \brief Build an ipv4 address from a host byte order integer
     *
     * \param address The host byte order integer
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(uint32_t address);
    /**
     * \brief Build an ipv4 address from a network byte order integer
     *
     * \param address The network byte order integer
     * \return \b true if the address is valid, \b false otherwise
     */
    bool setNetworkByteOrdered(uint32_t address);

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

    static IpAddress const Ipv4None; ///< Represent an invalid ipv4 address
    static IpAddress const Ipv6None; ///< Represent an invalid ipv6 address

    static IpAddress const Ipv4Any; ///< Represent an unspecified ipv4 address "0.0.0.0"
    static IpAddress const Ipv6Any; ///< Represent an unspecified ipv6 address "::"

    static IpAddress const Ipv4Loopback; ///< Represent the local host ipv4 address "127.0.0.1"
    static IpAddress const Ipv6Loopback; ///< Represent the local host ipv6 address "::1"

    static IpAddress const Ipv4Broadcast; ///< Represent the broadcast ipv4 address "255.255.255.255"

private:
    std::variant<std::monostate, Ipv4Data, Ipv6Data> g_address; ///< Network byte order address
};

} // namespace fge::net

#endif // _FGE_C_IPADDRESS_HPP_INCLUDED_
