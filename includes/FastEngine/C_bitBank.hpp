#ifndef _FGE_C_BITBANK_HPP_INCLUDED
#define _FGE_C_BITBANK_HPP_INCLUDED

#include <FastEngine/C_packet.hpp>

namespace fge
{

/**
 * \class BitBank
 * \ingroup utility
 * \brief A class to manage a group of bits.
 *
 * \tparam TNbytes The number of bytes in the bank.
 */
template<std::size_t TNbytes>
class BitBank
{
    static_assert(TNbytes>0, "TNbytes must be greater than 0");
public:
    BitBank() = default;
    ~BitBank() = default;

    /**
     * \brief Clear the bank.
     *
     * This function clear the bank by setting all bits to 0.
     */
    void clear();

    /**
     * \brief Set a bit to 1 or 0.
     *
     * This function set the bit at the specified index to 1 or 0.
     *
     * \param index The index of the bit to set.
     */
    void set(std::size_t index, bool flag);
    /**
     * \brief Get the value of a bit.
     *
     * This function get the value of the bit at the specified index.
     *
     * \param index The index of the bit to get.
     * \return The value of the bit or 0 if out of range.
     */
    [[nodiscard]] bool get(std::size_t index) const;
    /**
     * \brief Get the specified byte.
     *
     * \param index The index of the byte to get.
     * \return The wanted byte or 0 if out of range.
     */
    [[nodiscard]] uint8_t getByte(std::size_t index) const;

    /**
     * \brief Get the number of bytes in the bank.
     *
     * \return The number of bytes in the bank.
     */
    [[nodiscard]] std::size_t getSize() const;

    /**
     * \brief Pack the bank data into the packet.
     *
     * \param pck The network packet to pack the bank data into.
     */
    void pack(fge::net::Packet& pck);
    /**
     * \brief Unpack the bank data from the packet.
     *
     * \param pck The network packet to unpack the bank data from.
     */
    void unpack(fge::net::Packet& pck);

private:
    uint8_t g_data[TNbytes]{0};
};

}//end fge

#include <FastEngine/C_bitBank.inl>

#endif // _FGE_C_BITBANK_HPP_INCLUDED
