#ifndef _FGE_ENDIAN_HPP_INCLUDED_
#define _FGE_ENDIAN_HPP_INCLUDED_

#include <FastEngine/fastengine_extern.hpp>
#include <cstdint>

namespace fge
{

uint16_t FGE_API SwapHostNetEndian_16(uint16_t n);
uint32_t FGE_API SwapHostNetEndian_32(uint32_t n);
uint64_t FGE_API SwapHostNetEndian_64(uint64_t n);
float FGE_API SwapHostNetEndian_f(float n);
double FGE_API SwapHostNetEndian_d(double n);

uint16_t FGE_API SwapEndian_16(uint16_t n);
uint32_t FGE_API SwapEndian_32(uint32_t n);
uint64_t FGE_API SwapEndian_64(uint64_t n);
float FGE_API SwapEndian_f(float n);
double FGE_API SwapEndian_d(double n);

bool FGE_API IsBigEndian();

}//end fge

#endif // _FGE_ENDIAN_HPP_INCLUDED_
