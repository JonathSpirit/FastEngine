#ifndef _FGE_EXTRA_STRING_HPP_INCLUDED
#define _FGE_EXTRA_STRING_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <vector>
#include <list>
#include <string>

namespace fge
{

class Value;

namespace string
{

uint8_t FGE_API ToUint8(const std::string& str);
uint16_t FGE_API ToUint16(const std::string& str);
uint32_t FGE_API ToUint32(const std::string& str);
uint64_t FGE_API ToUint64(const std::string& str);
int8_t FGE_API ToInt8(const std::string& str);
int16_t FGE_API ToInt16(const std::string& str);
int32_t FGE_API ToInt32(const std::string& str);
int64_t FGE_API ToInt64(const std::string& str);

unsigned int FGE_API ToUint(const std::string& str);
int FGE_API ToInt(const std::string& str);

unsigned long long int FGE_API ToUlong(const std::string& str);
long long int FGE_API ToLong(const std::string& str);

float FGE_API ToFloat(const std::string& str);
double FGE_API ToDouble(const std::string& str);
long double FGE_API ToLongDouble(const std::string& str);

bool FGE_API ToBool(const std::string& str);
void* FGE_API ToPtr(const std::string& str);

sf::Vector2f FGE_API ToVec2f(const std::string& str);
sf::Vector2u FGE_API ToVec2u(const std::string& str);
sf::Vector2i FGE_API ToVec2i(const std::string& str);

//Arithmetic type

std::string FGE_API ToStr(bool val);
std::string FGE_API ToStr(char val);
std::string FGE_API ToStr(char16_t val);
std::string FGE_API ToStr(char32_t val);
std::string FGE_API ToStr(wchar_t val);

std::string FGE_API ToStr(signed char val);
std::string FGE_API ToStr(short int val);
std::string FGE_API ToStr(int val);
std::string FGE_API ToStr(long int val);
std::string FGE_API ToStr(long long int val);

std::string FGE_API ToStr(unsigned char val);
std::string FGE_API ToStr(unsigned short int val);
std::string FGE_API ToStr(unsigned int val);
std::string FGE_API ToStr(unsigned long int val);
std::string FGE_API ToStr(unsigned long long int val);

std::string FGE_API ToStr(float val);
std::string FGE_API ToStr(double val);
std::string FGE_API ToStr(long double val);

//Pointer

std::string FGE_API ToStr(void* val);
std::string FGE_API ToStr(const void* val);

//Classes

std::string FGE_API ToStr(const sf::Vector2f& val);
std::string FGE_API ToStr(const sf::Vector2u& val);
std::string FGE_API ToStr(const sf::Vector2i& val);

std::string FGE_API ToStr(const sf::Vector3f& val);
std::string FGE_API ToStr(const sf::Vector3i& val);

std::string FGE_API ToStr(const fge::Value& val);

template <class T>
std::string ToStr (const std::list<T>& val, char separator=' ');
template <class T>
std::string ToStr (const std::vector<T>& val, char separator=' ');

std::size_t FGE_API Split(const std::string& str, std::vector<std::string>& output, char separator);

}//end string
}//end fge

#include <FastEngine/extra_string.inl>

#endif // _FGE_EXTRA_STRING_HPP_INCLUDED
