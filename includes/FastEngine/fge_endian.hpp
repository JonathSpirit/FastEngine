#ifndef _FGE_ENDIAN_HPP_INCLUDED_
#define _FGE_ENDIAN_HPP_INCLUDED_

#include <FastEngine/fastengine_extern.hpp>
#include <cstdint>

namespace fge
{

FGE_API uint16_t SwapHostNetEndian_16(uint16_t n);
FGE_API uint32_t SwapHostNetEndian_32(uint32_t n);
FGE_API uint64_t SwapHostNetEndian_64(uint64_t n);
FGE_API float SwapHostNetEndian_f(float n);
FGE_API double SwapHostNetEndian_d(double n);

FGE_API uint16_t SwapEndian_16(uint16_t n);
FGE_API uint32_t SwapEndian_32(uint32_t n);
FGE_API uint64_t SwapEndian_64(uint64_t n);
FGE_API float SwapEndian_f(float n);
FGE_API double SwapEndian_d(double n);

FGE_API bool IsBigEndian();

}//end fge

#endif // _FGE_ENDIAN_HPP_INCLUDED_
