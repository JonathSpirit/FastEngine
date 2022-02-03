#include "FastEngine/extra_string.hpp"

#include "FastEngine/C_value.hpp"
#include <algorithm>
#include <sstream>

#define strtk_no_tr1_or_boost
#include <strtk.hpp>

#include <fmt/compile.h>
#include <fmt/format.h>

namespace fge
{
namespace string
{

uint8_t FGE_API ToUint8(const std::string& str)
{
    uint8_t result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
uint16_t FGE_API ToUint16(const std::string& str)
{
    uint16_t result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
uint32_t FGE_API ToUint32(const std::string& str)
{
    uint32_t result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
uint64_t FGE_API ToUint64(const std::string& str)
{
    uint64_t result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
int8_t FGE_API ToInt8(const std::string& str)
{
    int8_t result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
int16_t FGE_API ToInt16(const std::string& str)
{
    int16_t result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
int32_t FGE_API ToInt32(const std::string& str)
{
    int32_t result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
int64_t FGE_API ToInt64(const std::string& str)
{
    int64_t result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}

unsigned int FGE_API ToUint(const std::string& str)
{
    unsigned int result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
int FGE_API ToInt(const std::string& str)
{
    int result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}

unsigned long long int FGE_API ToUlong(const std::string& str)
{
    unsigned long long int result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}
long long int FGE_API ToLong(const std::string& str)
{
    long long int result=0;
    strtk::string_to_type_converter(str, result);
    return result;
}

float FGE_API ToFloat(const std::string& str)
{
    float result=0.0f;
    strtk::string_to_type_converter(str, result);
    return result;
}
double FGE_API ToDouble(const std::string& str)
{
    double result=0.0;
    strtk::string_to_type_converter(str, result);
    return result;
}
long double FGE_API ToLongDouble(const std::string& str)
{
    long double result=0.0;
    strtk::string_to_type_converter(str, result);
    return result;
}

bool FGE_API ToBool(const std::string& str)
{
    bool result=false;
    strtk::string_to_type_converter(str, result);
    return result;
}
void* FGE_API ToPtr(const std::string& str)
{
    unsigned int i = 0;

    if (str.size() >= 2)
    {
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        {//Skipping the 0x / 0X
            i = 2;
        }
    }

    if (str.size() == i)
    {//Empty hex number, can't convert that
        return nullptr;
    }

    if (str.size() > (sizeof(void*)*2 +i))
    {//Too big hex number, can't convert that
        return nullptr;
    }

    uint64_t result = 0;
    unsigned int shift = (str.size()-i)*4 - 4;

    //Conversion
    for (; i<str.size(); ++i)
    {
        uint64_t n = 0;

        if ( str[i] >= '0' && str[i] <= '9' )
        {
            n = str[i] - '0';
        }
        else if ( str[i] >= 'A' && str[i] <= 'F' )
        {
            n = str[i] - 'A' + 10;
        }
        else if ( str[i] >= 'a' && str[i] <= 'f' )
        {
            n = str[i] - 'a' + 10;
        }
        else
        {//Bad hex value
            return nullptr;
        }

        result |= (n<<shift);
        shift -= 4;
    }

    return reinterpret_cast<void*>( result );
}

sf::Vector2f FGE_API ToVec2f(const std::string& str)
{
    size_t tagpos = str.find(' ');
    if ( tagpos == std::string::npos )
    {
        return sf::Vector2f(0.0f,0.0f);
    }
    return sf::Vector2f( fge::string::ToFloat(str.substr(0, tagpos)) , fge::string::ToFloat(str.substr(tagpos+1, std::string::npos)) );
}
sf::Vector2u FGE_API ToVec2u(const std::string& str)
{
    size_t tagpos = str.find(' ');
    if ( tagpos == std::string::npos )
    {
        return sf::Vector2u(0,0);
    }
    return sf::Vector2u( fge::string::ToUint(str.substr(0, tagpos)) , fge::string::ToUint(str.substr(tagpos+1, std::string::npos)) );
}
sf::Vector2i FGE_API ToVec2i(const std::string& str)
{
    size_t tagpos = str.find(' ');
    if ( tagpos == std::string::npos )
    {
        return sf::Vector2i(0.0f,0.0f);
    }
    return sf::Vector2i( fge::string::ToInt(str.substr(0, tagpos)) , fge::string::ToInt(str.substr(tagpos+1, std::string::npos)) );
}

//Arithmetic type

std::string FGE_API ToStr(bool val)
{
    return val ? "1" : "0";
}
std::string FGE_API ToStr(char val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(char16_t val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(char32_t val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(wchar_t val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}

std::string FGE_API ToStr(signed char val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(short int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(long int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(long long int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}

std::string FGE_API ToStr(unsigned char val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(unsigned short int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(unsigned int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(unsigned long int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(unsigned long long int val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}

std::string FGE_API ToStr(float val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(double val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(long double val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}

//Pointer

std::string FGE_API ToStr(void* val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}
std::string FGE_API ToStr(const void* val)
{
    return fmt::format(FMT_COMPILE("{}"), val);
}

//Classes

std::string FGE_API ToStr(const sf::Vector2f& val)
{
    return fmt::format(FMT_COMPILE("{} {}"), val.x, val.y);
}
std::string FGE_API ToStr(const sf::Vector2u& val)
{
    return fmt::format(FMT_COMPILE("{} {}"), val.x, val.y);
}
std::string FGE_API ToStr(const sf::Vector2i& val)
{
    return fmt::format(FMT_COMPILE("{} {}"), val.x, val.y);
}

std::string FGE_API ToStr(const sf::Vector3f& val)
{
    return fmt::format(FMT_COMPILE("{} {} {}"), val.x, val.y, val.z);
}
std::string FGE_API ToStr(const sf::Vector3i& val)
{
    return fmt::format(FMT_COMPILE("{} {} {}"), val.x, val.y, val.z);
}

std::string FGE_API ToStr(const fge::Value& val)
{
    return val.toString();
}

///-------------------------------------------------------------------------------

std::size_t FGE_API Split(const std::string& str, std::vector<std::string>& output, char separator)
{
    output.clear();
    std::istringstream f(str);
    std::string sres;

    while ( std::getline(f, sres, separator) )
    {
        output.push_back(std::move(sres));
    }

    return output.size();
}

}//end string
}//end fge
