#ifndef _FGE_C_IPADDRESS_HPP_INCLUDED_
#define _FGE_C_IPADDRESS_HPP_INCLUDED_

#include <FastEngine/fastengine_extern.hpp>
#include <string>
#include <vector>
#include <cstdint>

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
    /**
     * \brief Build a default invalid IP address
     */
    IpAddress() noexcept;
    /**
     * \brief Build an IP address from a string
     *
     * The string can be in the form XXX.XXX.XXX.XXX, or a hostname.
     *
     * \param address A string representing the IP address
     */
    IpAddress(const std::string& address);
    IpAddress(const char* address);
    /**
     * \brief Build an IP address from a 4 bytes
     *
     * \param byte3 The most significant byte
     * \param byte2 The second most significant byte
     * \param byte1 The second least significant byte
     * \param byte0 The least significant byte
     */
    IpAddress(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0) noexcept;
    /**
     * \brief Build an IP address from a host byte order integer
     *
     * \param address The host byte order integer
     */
    IpAddress(uint32_t address) noexcept;
    ~IpAddress() = default;

    /**
     * \brief Build an IP address from a string
     *
     * The string can be in the form XXX.XXX.XXX.XXX, or a hostname.
     *
     * \param address A string representing the IP address
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(const std::string& address);
    bool set(const char* address);
    /**
     * \brief Build an IP address from a 4 bytes
     *
     * \param byte3 The most significant byte
     * \param byte2 The second most significant byte
     * \param byte1 The second least significant byte
     * \param byte0 The least significant byte
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0);
    /**
     * \brief Build an IP address from a host byte order integer
     *
     * \param address The host byte order integer
     * \return \b true if the address is valid, \b false otherwise
     */
    bool set(uint32_t address);
    /**
     * \brief Build an IP address from a network byte order integer
     *
     * \param address The network byte order integer
     * \return \b true if the address is valid, \b false otherwise
     */
    bool setNetworkByteOrdered(uint32_t address);

    bool operator==(const fge::net::IpAddress& r) const;

    /**
     * \brief Get the IP address in a string format
     *
     * \return A string representing the IP address
     */
    [[nodiscard]] std::string toString() const;

    /**
     * \brief Get the IP address in a network byte order integer
     *
     * \return The network byte order integer
     */
    [[nodiscard]] uint32_t getNetworkByteOrder() const;
    /**
     * \brief Get the IP address in a host byte order integer
     *
     * \return The host byte order integer
     */
    [[nodiscard]] uint32_t getHostByteOrder() const;

    /**
     * \brief Get the standard hostname for the local computer
     *
     * \return The hostname
     */
    static std::string getHostName();
    /**
     * \brief Get a list of local IpAddress of the local computer
     *
     * \param buff The buffer to fill with the local IP addresses
     */
    static void getLocalAddresses(std::vector<fge::net::IpAddress>& buff);

    /**
     * \brief Represent an invalid IpAddress
     */
    static const fge::net::IpAddress None;
    /**
     * \brief Represent the IpAddress 0.0.0.0
     */
    static const fge::net::IpAddress Any;
    /**
     * \brief Represent the IpAddress 127.0.0.1
     */
    static const fge::net::IpAddress LocalHost;
    /**
     * \brief Represent the IpAddress 255.255.255.255
     */
    static const fge::net::IpAddress Broadcast;

private:
    uint32_t g_address; //Network byte order address
    bool g_valid;
};

}//end fge::net

#endif // _FGE_C_IPADDRESS_HPP_INCLUDED_
