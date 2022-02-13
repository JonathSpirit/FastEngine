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

FGE_API uint8_t ToUint8(const std::string& str);
FGE_API uint16_t ToUint16(const std::string& str);
FGE_API uint32_t ToUint32(const std::string& str);
FGE_API uint64_t ToUint64(const std::string& str);
FGE_API int8_t ToInt8(const std::string& str);
FGE_API int16_t ToInt16(const std::string& str);
FGE_API int32_t ToInt32(const std::string& str);
FGE_API int64_t ToInt64(const std::string& str);

FGE_API unsigned int ToUint(const std::string& str);
FGE_API int ToInt(const std::string& str);

FGE_API unsigned long long int ToUlong(const std::string& str);
FGE_API long long int ToLong(const std::string& str);

FGE_API float ToFloat(const std::string& str);
FGE_API double ToDouble(const std::string& str);
FGE_API long double ToLongDouble(const std::string& str);

FGE_API bool ToBool(const std::string& str);
FGE_API void* ToPtr(const std::string& str);

FGE_API sf::Vector2f ToVec2f(const std::string& str);
FGE_API sf::Vector2u ToVec2u(const std::string& str);
FGE_API sf::Vector2i ToVec2i(const std::string& str);

//Arithmetic type

FGE_API std::string ToStr(bool val);
FGE_API std::string ToStr(char val);
FGE_API std::string ToStr(char16_t val);
FGE_API std::string ToStr(char32_t val);
FGE_API std::string ToStr(wchar_t val);

FGE_API std::string ToStr(signed char val);
FGE_API std::string ToStr(short int val);
FGE_API std::string ToStr(int val);
FGE_API std::string ToStr(long int val);
FGE_API std::string ToStr(long long int val);

FGE_API std::string ToStr(unsigned char val);
FGE_API std::string ToStr(unsigned short int val);
FGE_API std::string ToStr(unsigned int val);
FGE_API std::string ToStr(unsigned long int val);
FGE_API std::string ToStr(unsigned long long int val);

FGE_API std::string ToStr(float val);
FGE_API std::string ToStr(double val);
FGE_API std::string ToStr(long double val);

//Pointer

FGE_API std::string ToStr(void* val);
FGE_API std::string ToStr(const void* val);

//Classes

FGE_API std::string ToStr(const sf::Vector2f& val);
FGE_API std::string ToStr(const sf::Vector2u& val);
FGE_API std::string ToStr(const sf::Vector2i& val);

FGE_API std::string ToStr(const sf::Vector3f& val);
FGE_API std::string ToStr(const sf::Vector3i& val);

FGE_API std::string ToStr(const fge::Value& val);

template <class T>
std::string ToStr (const std::list<T>& val, char separator=' ');
template <class T>
std::string ToStr (const std::vector<T>& val, char separator=' ');

FGE_API std::size_t Split(const std::string& str, std::vector<std::string>& output, char separator);

}//end string
}//end fge

#include <FastEngine/extra_string.inl>

#endif // _FGE_EXTRA_STRING_HPP_INCLUDED
