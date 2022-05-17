#ifndef _FGE_ENDIAN_HPP_INCLUDED_
#define _FGE_ENDIAN_HPP_INCLUDED_

#include <FastEngine/fastengine_extern.hpp>
#include <cstdint>

namespace fge
{

/**
 * \ingroup network
 * \brief Swap the endianness of a 16-bit integer
 *
 * This function does nothing if the endianness is already big endian.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API uint16_t SwapHostNetEndian_16(uint16_t n);
/**
 * \ingroup network
 * \brief Swap the endianness of a 32-bit integer
 *
 * This function does nothing if the endianness is already big endian.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API uint32_t SwapHostNetEndian_32(uint32_t n);
/**
 * \ingroup network
 * \brief Swap the endianness of a 64-bit integer
 *
 * This function does nothing if the endianness is already big endian.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API uint64_t SwapHostNetEndian_64(uint64_t n);
/**
 * \ingroup network
 * \brief Swap the endianness of a float
 *
 * This function does nothing if the endianness is already little endian.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API float SwapHostNetEndian_f(float n);
/**
 * \ingroup network
 * \brief Swap the endianness of a double
 *
 * This function does nothing if the endianness is already little endian.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API double SwapHostNetEndian_d(double n);

/**
 * \ingroup network
 * \brief Swap the endianness of a 16-bit integer
 *
 * This function swap whatever the endianness of the value is.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API uint16_t SwapEndian_16(uint16_t n);
/**
 * \ingroup network
 * \brief Swap the endianness of a 32-bit integer
 *
 * This function swap whatever the endianness of the value is.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API uint32_t SwapEndian_32(uint32_t n);
/**
 * \ingroup network
 * \brief Swap the endianness of a 64-bit integer
 *
 * This function swap whatever the endianness of the value is.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API uint64_t SwapEndian_64(uint64_t n);
/**
 * \ingroup network
 * \brief Swap the endianness of a float
 *
 * This function swap whatever the endianness of the value is.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API float SwapEndian_f(float n);
/**
 * \ingroup network
 * \brief Swap the endianness of a double
 *
 * This function swap whatever the endianness of the value is.
 *
 * \param n The value to swap
 * \return The maybe swapped value
 */
FGE_API double SwapEndian_d(double n);

/**
 * \ingroup network
 * \brief Check if the endianness of the system is big endian
 *
 * \return \b true if the endianness is big endian, \b false otherwise
 */
FGE_API bool IsBigEndian();

}//end fge

#endif // _FGE_ENDIAN_HPP_INCLUDED_
