/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/fge_endian.hpp"

#ifdef __GNUC__
    #if __GNUC__ > 8
        #include <bit>
    #else
        #include <bit>
        #include <type_traits>
    #endif
#else
    #include <bit>
    #include <type_traits>
#endif

namespace fge
{

uint16_t SwapHostNetEndian_16(uint16_t n)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        return n;
    }
    else
    {
        uint16_t r;
        static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
        static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
        return r;
    }
}
uint32_t SwapHostNetEndian_32(uint32_t n)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        return n;
    }
    else
    {
        uint32_t r;
        static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[3];
        static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[2];
        static_cast<uint8_t*>(static_cast<void*>(&r))[2] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
        static_cast<uint8_t*>(static_cast<void*>(&r))[3] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
        return r;
    }
}
uint64_t SwapHostNetEndian_64(uint64_t n)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        return n;
    }
    else
    {
        uint64_t r;
        static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[7];
        static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[6];
        static_cast<uint8_t*>(static_cast<void*>(&r))[2] = static_cast<uint8_t*>(static_cast<void*>(&n))[5];
        static_cast<uint8_t*>(static_cast<void*>(&r))[3] = static_cast<uint8_t*>(static_cast<void*>(&n))[4];
        static_cast<uint8_t*>(static_cast<void*>(&r))[4] = static_cast<uint8_t*>(static_cast<void*>(&n))[3];
        static_cast<uint8_t*>(static_cast<void*>(&r))[5] = static_cast<uint8_t*>(static_cast<void*>(&n))[2];
        static_cast<uint8_t*>(static_cast<void*>(&r))[6] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
        static_cast<uint8_t*>(static_cast<void*>(&r))[7] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
        return r;
    }
}
float SwapHostNetEndian_f(float n)
{
    static_assert(sizeof(float) == 4, "float doesn't have a size of 4 bytes");

    if constexpr (std::endian::native == std::endian::big)
    {
        return n;
    }
    else
    {
        float r;
        static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[3];
        static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[2];
        static_cast<uint8_t*>(static_cast<void*>(&r))[2] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
        static_cast<uint8_t*>(static_cast<void*>(&r))[3] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
        return r;
    }
}
double SwapHostNetEndian_d(double n)
{
    static_assert(sizeof(double) == 8, "double doesn't have a size of 8 bytes");

    if constexpr (std::endian::native == std::endian::big)
    {
        return n;
    }
    else
    {
        double r;
        static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[7];
        static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[6];
        static_cast<uint8_t*>(static_cast<void*>(&r))[2] = static_cast<uint8_t*>(static_cast<void*>(&n))[5];
        static_cast<uint8_t*>(static_cast<void*>(&r))[3] = static_cast<uint8_t*>(static_cast<void*>(&n))[4];
        static_cast<uint8_t*>(static_cast<void*>(&r))[4] = static_cast<uint8_t*>(static_cast<void*>(&n))[3];
        static_cast<uint8_t*>(static_cast<void*>(&r))[5] = static_cast<uint8_t*>(static_cast<void*>(&n))[2];
        static_cast<uint8_t*>(static_cast<void*>(&r))[6] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
        static_cast<uint8_t*>(static_cast<void*>(&r))[7] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
        return r;
    }
}

///

uint16_t SwapEndian_16(uint16_t n)
{
    uint16_t r;
    static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
    static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
    return r;
}
uint32_t SwapEndian_32(uint32_t n)
{
    uint32_t r;
    static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[3];
    static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[2];
    static_cast<uint8_t*>(static_cast<void*>(&r))[2] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
    static_cast<uint8_t*>(static_cast<void*>(&r))[3] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
    return r;
}
uint64_t SwapEndian_64(uint64_t n)
{
    uint64_t r;
    static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[7];
    static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[6];
    static_cast<uint8_t*>(static_cast<void*>(&r))[2] = static_cast<uint8_t*>(static_cast<void*>(&n))[5];
    static_cast<uint8_t*>(static_cast<void*>(&r))[3] = static_cast<uint8_t*>(static_cast<void*>(&n))[4];
    static_cast<uint8_t*>(static_cast<void*>(&r))[4] = static_cast<uint8_t*>(static_cast<void*>(&n))[3];
    static_cast<uint8_t*>(static_cast<void*>(&r))[5] = static_cast<uint8_t*>(static_cast<void*>(&n))[2];
    static_cast<uint8_t*>(static_cast<void*>(&r))[6] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
    static_cast<uint8_t*>(static_cast<void*>(&r))[7] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
    return r;
}
float SwapEndian_f(float n)
{
    static_assert(sizeof(float) == 4, "float doesn't have a size of 4 bytes");

    float r;
    static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[3];
    static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[2];
    static_cast<uint8_t*>(static_cast<void*>(&r))[2] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
    static_cast<uint8_t*>(static_cast<void*>(&r))[3] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
    return r;
}
double SwapEndian_d(double n)
{
    static_assert(sizeof(double) == 8, "double doesn't have a size of 8 bytes");

    double r;
    static_cast<uint8_t*>(static_cast<void*>(&r))[0] = static_cast<uint8_t*>(static_cast<void*>(&n))[7];
    static_cast<uint8_t*>(static_cast<void*>(&r))[1] = static_cast<uint8_t*>(static_cast<void*>(&n))[6];
    static_cast<uint8_t*>(static_cast<void*>(&r))[2] = static_cast<uint8_t*>(static_cast<void*>(&n))[5];
    static_cast<uint8_t*>(static_cast<void*>(&r))[3] = static_cast<uint8_t*>(static_cast<void*>(&n))[4];
    static_cast<uint8_t*>(static_cast<void*>(&r))[4] = static_cast<uint8_t*>(static_cast<void*>(&n))[3];
    static_cast<uint8_t*>(static_cast<void*>(&r))[5] = static_cast<uint8_t*>(static_cast<void*>(&n))[2];
    static_cast<uint8_t*>(static_cast<void*>(&r))[6] = static_cast<uint8_t*>(static_cast<void*>(&n))[1];
    static_cast<uint8_t*>(static_cast<void*>(&r))[7] = static_cast<uint8_t*>(static_cast<void*>(&n))[0];
    return r;
}

bool IsBigEndian()
{
    if constexpr (std::endian::native == std::endian::big)
    {
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace fge
