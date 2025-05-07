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

#include "FastEngine/extra/extra_string.hpp"
#include "FastEngine/C_property.hpp"

#include "fmt/compile.h"
#include "fmt/format.h"
#include "private/fast_float.h"
#include <sstream>

namespace fge::string
{

bool IsValidUtf8String(std::string const& str)
{
    uint8_t requiredBytes{0};

    for (char c: str)
    {
        if ((c & 0xC0) == 0x80)
        { //Multibyte
            if (requiredBytes == 0)
            { //No start sequence
                return false;
            }
            --requiredBytes;
        }
        else if ((c & 0xF1) == 0xF0)
        {
            if (requiredBytes != 0)
            { //Invalid sequence
                return false;
            }
            requiredBytes = 3;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            if (requiredBytes != 0)
            { //Invalid sequence
                return false;
            }
            requiredBytes = 2;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            if (requiredBytes != 0)
            { //Invalid sequence
                return false;
            }
            requiredBytes = 1;
        }
    }
    return requiredBytes == 0;
}

uint8_t ToUint8(std::string const& str)
{
    uint8_t result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
uint16_t ToUint16(std::string const& str)
{
    uint16_t result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
uint32_t ToUint32(std::string const& str)
{
    uint32_t result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
uint64_t ToUint64(std::string const& str)
{
    uint64_t result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
int8_t ToInt8(std::string const& str)
{
    int8_t result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
int16_t ToInt16(std::string const& str)
{
    int16_t result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
int32_t ToInt32(std::string const& str)
{
    int32_t result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
int64_t ToInt64(std::string const& str)
{
    int64_t result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}

unsigned int ToUint(std::string const& str)
{
    unsigned int result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
int ToInt(std::string const& str)
{
    int result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}

unsigned long long int ToUlong(std::string const& str)
{
    unsigned long long int result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
long long int ToLong(std::string const& str)
{
    long long int result = 0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}

float ToFloat(std::string const& str)
{
    float result = 0.0f;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
double ToDouble(std::string const& str)
{
    double result = 0.0;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}

bool ToBool(std::string const& str)
{
    //String equality check with case insensitive
    if (str.size() == 4 && (str[0] == 't' || str[0] == 'T') && (str[1] == 'r' || str[1] == 'R') &&
        (str[2] == 'u' || str[2] == 'U') && (str[3] == 'e' || str[3] == 'E'))
    {
        return true;
    }
    if (str.size() == 5 && (str[0] == 'f' || str[0] == 'F') && (str[1] == 'a' || str[1] == 'A') &&
        (str[2] == 'l' || str[2] == 'L') && (str[3] == 's' || str[3] == 'S') && (str[4] == 'e' || str[4] == 'E'))
    {
        return false;
    }

    bool result = false;
    fast_float::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
void* ToPtr(std::string const& str)
{
    unsigned int i = 0;

    if (str.size() >= 2)
    {
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        { //Skipping the 0x / 0X
            i = 2;
        }
    }

    if (str.size() == i)
    { //Empty hex number, can't convert that
        return nullptr;
    }

    if (str.size() > (sizeof(void*) * 2 + i))
    { //Too big hex number, can't convert that
        return nullptr;
    }

    uint64_t result = 0;
    unsigned int shift = (str.size() - i) * 4 - 4;

    //Conversion
    for (; i < str.size(); ++i)
    {
        uint64_t n = 0;

        if (str[i] >= '0' && str[i] <= '9')
        {
            n = str[i] - '0';
        }
        else if (str[i] >= 'A' && str[i] <= 'F')
        {
            n = str[i] - 'A' + 10;
        }
        else if (str[i] >= 'a' && str[i] <= 'f')
        {
            n = str[i] - 'a' + 10;
        }
        else
        { //Bad hex value
            return nullptr;
        }

        result |= (n << shift);
        shift -= 4;
    }

    return reinterpret_cast<void*>(result);
}

fge::Vector2f ToVec2f(std::string const& str)
{
    size_t tagpos = str.find(' ');
    if (tagpos == std::string::npos)
    {
        return {0.0f, 0.0f};
    }
    return {fge::string::ToFloat(str.substr(0, tagpos)),
            fge::string::ToFloat(str.substr(tagpos + 1, std::string::npos))};
}
fge::Vector2u ToVec2u(std::string const& str)
{
    size_t tagpos = str.find(' ');
    if (tagpos == std::string::npos)
    {
        return {0, 0};
    }
    return {fge::string::ToUint(str.substr(0, tagpos)), fge::string::ToUint(str.substr(tagpos + 1, std::string::npos))};
}
fge::Vector2i ToVec2i(std::string const& str)
{
    size_t tagpos = str.find(' ');
    if (tagpos == std::string::npos)
    {
        return {0, 0};
    }
    return {fge::string::ToInt(str.substr(0, tagpos)), fge::string::ToInt(str.substr(tagpos + 1, std::string::npos))};
}

//Arithmetic type

std::string ToStr(bool val)
{
    return val ? "1" : "0";
}
std::string ToStr(char val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(char16_t val)
{
    return fmt::format(FMT_COMPILE("{}"), static_cast<int16_t>(val));
}
std::string ToStr(char32_t val)
{
    return fmt::format(FMT_COMPILE("{}"), static_cast<int32_t>(val));
}
std::string ToStr(wchar_t val)
{
    return fmt::format(FMT_COMPILE("{}"), static_cast<int32_t>(val));
}

std::string ToStr(signed char val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(short int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(long int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(long long int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}

std::string ToStr(unsigned char val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(unsigned short int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(unsigned int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(unsigned long int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(unsigned long long int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}

std::string ToStr(float val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(float val, unsigned int precision, bool keepExtraZeros)
{
    std::string result = fmt::format(FMT_COMPILE("{:.{}f}"), val, precision);
    if (!keepExtraZeros)
    {
        auto pos = result.find_last_not_of('0');
        pos += 1 + static_cast<unsigned>(result[pos] == '.');
        if (pos < result.length())
        {
            result.erase(pos, std::string::npos);
        }
    }
    return result;
}
std::string ToStr(double val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(double val, unsigned int precision, bool keepExtraZeros)
{
    std::string result = fmt::format(FMT_COMPILE("{:.{}f}"), val, precision);
    if (!keepExtraZeros)
    {
        auto pos = result.find_last_not_of('0');
        pos += 1 + static_cast<unsigned>(result[pos] == '.');
        if (pos < result.length())
        {
            result.erase(pos, std::string::npos);
        }
    }
    return result;
}
std::string ToStr(long double val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(long double val, unsigned int precision, bool keepExtraZeros)
{
    std::string result = fmt::format(FMT_COMPILE("{:.{}f}"), val, precision);
    if (!keepExtraZeros)
    {
        auto pos = result.find_last_not_of('0');
        pos += 1 + static_cast<unsigned>(result[pos] == '.');
        if (pos < result.length())
        {
            result.erase(pos, std::string::npos);
        }
    }
    return result;
}

//Pointer

std::string ToStr(void* val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string ToStr(void const* val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}

//Classes

std::string ToStr(fge::Vector2f const& val)
{
    return fmt::format(FMT_COMPILE("{} {}"), val.x, val.y);
}
std::string ToStr(fge::Vector2u const& val)
{
    return fmt::format(FMT_COMPILE("{} {}"), val.x, val.y);
}
std::string ToStr(fge::Vector2i const& val)
{
    return fmt::format(FMT_COMPILE("{} {}"), val.x, val.y);
}

std::string ToStr(fge::Vector3f const& val)
{
    return fmt::format(FMT_COMPILE("{} {} {}"), val.x, val.y, val.z);
}
std::string ToStr(fge::Vector3i const& val)
{
    return fmt::format(FMT_COMPILE("{} {} {}"), val.x, val.y, val.z);
}

std::string ToStr(fge::Property const& val)
{
    return val.toString();
}

///-------------------------------------------------------------------------------

std::size_t Split(std::string const& str, std::vector<std::string>& output, char separator)
{
    output.clear();
    std::istringstream f(str);
    std::string sres;

    while (std::getline(f, sres, separator))
    {
        output.push_back(std::move(sres));
    }

    return output.size();
}

} // namespace fge::string
