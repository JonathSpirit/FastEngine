/*
 * Copyright 2023 Guillaume Guillet
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

#ifndef _FGE_EXTRA_STRING_HPP_INCLUDED
#define _FGE_EXTRA_STRING_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_vector.hpp"
#include <list>
#include <optional>
#include <string>
#include <vector>

namespace fge
{

class Property;

namespace string
{

/**
 * \ingroup extraString
 * \brief Check if the provided string has valid utf8 encoded chars
 *
 * \param str The string to check
 * \return \b True if the string is valid, \b False otherwise
 */
FGE_API bool IsValidUtf8String(const std::string& str);

/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an uint8_t
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API uint8_t ToUint8(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an uint16_t
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API uint16_t ToUint16(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an uint32_t
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API uint32_t ToUint32(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an uint64_t
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API uint64_t ToUint64(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an int8_t
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API int8_t ToInt8(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an int16_t
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API int16_t ToInt16(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an int32_t
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API int32_t ToInt32(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an int64_t
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API int64_t ToInt64(const std::string& str);

/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an unsigned int
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API unsigned int ToUint(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an int
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API int ToInt(const std::string& str);

/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an unsigned long long int
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API unsigned long long int ToUlong(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to an long long int
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API long long int ToLong(const std::string& str);

/**
 * \ingroup extraString
 * \brief Convert efficiently a string to a float
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API float ToFloat(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to a double
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API double ToDouble(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert efficiently a string to a long double
 *
 * \param str The string to convert
 * \return The converted value or 0 if there is an error
 */
FGE_API long double ToLongDouble(const std::string& str);

/**
 * \ingroup extraString
 * \brief Convert efficiently a string to a bool
 *
 * \param str The string to convert
 * \return The converted value or \b false if there is an error
 */
FGE_API bool ToBool(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert a string to a pointer (address)
 *
 * The contained string must be a hexadecimal number and can
 * be prefixed by 0x or 0X.
 *
 * \warning This function is not really safe and should be used with caution
 *
 * \param str The string to convert
 * \return The converted address or \b nullptr if there is an error
 */
FGE_API void* ToPtr(const std::string& str);

/**
 * \ingroup extraString
 * \brief Convert a string to a vector2<float>
 *
 * The vector value contained in the string must be separated by a space
 *
 * \param str The string to convert
 * \return The converted vector or a vector with 0,0 if there is an error
 */
FGE_API fge::Vector2f ToVec2f(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert a string to a vector2<unsigned int>
 *
 * The vector value contained in the string must be separated by a space
 *
 * \param str The string to convert
 * \return The converted vector or a vector with 0,0 if there is an error
 */
FGE_API fge::Vector2u ToVec2u(const std::string& str);
/**
 * \ingroup extraString
 * \brief Convert a string to a vector2<int>
 *
 * The vector value contained in the string must be separated by a space
 *
 * \param str The string to convert
 * \return The converted vector or a vector with 0,0 if there is an error
 */
FGE_API fge::Vector2i ToVec2i(const std::string& str);

//Arithmetic type

/**
 * \ingroup extraString
 * \brief Convert efficiently a boolean to a string
 *
 * Note that this function return "1" for \b true and "0" for \b false.
 *
 * \param val The boolean to convert
 * \return The converted string
 */
FGE_API std::string ToStr(bool val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a char to a string
 *
 * \param val The char to convert
 * \return The converted string
 */
FGE_API std::string ToStr(char val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a char16_t to a string
 *
 * \param val The char16_t to convert
 * \return The converted string
 */
FGE_API std::string ToStr(char16_t val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a char32_t to a string
 *
 * \param val The char32_t to convert
 * \return The converted string
 */
FGE_API std::string ToStr(char32_t val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a wchar_t to a string
 *
 * \param val The wchar_t to convert
 * \return The converted string
 */
FGE_API std::string ToStr(wchar_t val);

/**
 * \ingroup extraString
 * \brief Convert efficiently a signed char to a string
 *
 * \param val The signed char to convert
 * \return The converted string
 */
FGE_API std::string ToStr(signed char val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a short int to a string
 *
 * \param val The short int to convert
 * \return The converted string
 */
FGE_API std::string ToStr(short int val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a int to a string
 *
 * \param val The int to convert
 * \return The converted string
 */
FGE_API std::string ToStr(int val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a long int to a string
 *
 * \param val The long int to convert
 * \return The converted string
 */
FGE_API std::string ToStr(long int val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a long long int to a string
 *
 * \param val The long long int to convert
 * \return The converted string
 */
FGE_API std::string ToStr(long long int val);

/**
 * \ingroup extraString
 * \brief Convert efficiently a unsigned char to a string
 *
 * \param val The unsigned char to convert
 * \return The converted string
 */
FGE_API std::string ToStr(unsigned char val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a unsigned short int to a string
 *
 * \param val The unsigned short int to convert
 * \return The converted string
 */
FGE_API std::string ToStr(unsigned short int val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a unsigned int to a string
 *
 * \param val The unsigned int to convert
 * \return The converted string
 */
FGE_API std::string ToStr(unsigned int val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a unsigned long int to a string
 *
 * \param val The unsigned long int to convert
 * \return The converted string
 */
FGE_API std::string ToStr(unsigned long int val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a unsigned long long int to a string
 *
 * \param val The unsigned long long int to convert
 * \return The converted string
 */
FGE_API std::string ToStr(unsigned long long int val);

/**
 * \ingroup extraString
 * \brief Convert efficiently a float to a string
 *
 * \param val The float to convert
 * \return The converted string
 */
FGE_API std::string ToStr(float val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a double to a string
 *
 * \param val The double to convert
 * \return The converted string
 */
FGE_API std::string ToStr(double val);
/**
 * \ingroup extraString
 * \brief Convert efficiently a long double to a string
 *
 * \param val The long double to convert
 * \return The converted string
 */
FGE_API std::string ToStr(long double val);

//Pointer
/**
 * \ingroup extraString
 * \brief Convert a pointer (address) to a string
 *
 * This function write the address in hexadecimal format.
 *
 * \see ToPtr
 *
 * \param val The address to convert
 * \return The converted string
 */
FGE_API std::string ToStr(void* val);
/**
 * \ingroup extraString
 * \brief Convert a pointer (address) to a string
 *
 * This function write the address in hexadecimal format.
 *
 * \see ToPtr
 *
 * \param val The address to convert
 * \return The converted string
 */
FGE_API std::string ToStr(const void* val);

/**
 * \ingroup extraString
 * \brief Convert a optional value to a string
 *
 * Return NO_VALUE if the optional doesn't have a value.
 *
 * \param val The optional value to convert
 * \return The converted string
 */
template<class T>
std::string ToStr(const std::optional<T>& val);

//Classes

/**
 * \ingroup extraString
 * \brief Convert a vector2<float> to a string
 *
 * \see ToVec2f
 *
 * \param val The vector2 to convert
 * \return The converted string
 */
FGE_API std::string ToStr(const fge::Vector2f& val);
/**
 * \ingroup extraString
 * \brief Convert a vector2<unsigned int> to a string
 *
 * \see ToVec2u
 *
 * \param val The vector2 to convert
 * \return The converted string
 */
FGE_API std::string ToStr(const fge::Vector2u& val);
/**
 * \ingroup extraString
 * \brief Convert a vector2<int> to a string
 *
 * \see ToVec2i
 *
 * \param val The vector2 to convert
 * \return The converted string
 */
FGE_API std::string ToStr(const fge::Vector2i& val);

/**
 * \ingroup extraString
 * \brief Convert a vector3<float> to a string
 *
 * \see ToVec3f
 *
 * \param val The vector3 to convert
 * \return The converted string
 */
FGE_API std::string ToStr(const fge::Vector3f& val);
/**
 * \ingroup extraString
 * \brief Convert a vector3<int> to a string
 *
 * \see ToVec3i
 *
 * \param val The vector3 to convert
 * \return The converted string
 */
FGE_API std::string ToStr(const fge::Vector3i& val);

/**
 * \ingroup extraString
 * \brief Convert a Property to a string
 *
 * This function meant to be a simple workaround for Property::ToString().
 *
 * \param val The Property to convert
 * \return The converted string
 */
FGE_API std::string ToStr(const fge::Property& val);

/**
 * \ingroup extraString
 * \brief Convert a list of value to a string
 *
 * \tparam T The type of the list
 * \param val The list to convert
 * \param separator The wanted separator between each value
 * \return A string containing all the values of the list
 */
template<class T>
std::string ToStr(const std::list<T>& val, char separator = ' ');
/**
 * \ingroup extraString
 * \brief Convert a vector of value to a string
 *
 * \tparam T The type of the list
 * \param val The vector to convert
 * \param separator The wanted separator between each value
 * \return A string containing all the values of the vector
 */
template<class T>
std::string ToStr(const std::vector<T>& val, char separator = ' ');

/**
 * \ingroup extraString
 * \brief Split a string into a vector of string
 *
 * \param str The string to split
 * \param output The vector to fill
 * \param separator The separator to use
 * \return The number of element added to the vector
 */
FGE_API std::size_t Split(const std::string& str, std::vector<std::string>& output, char separator);

} // namespace string
} // namespace fge

#include "extra_string.inl"

#endif // _FGE_EXTRA_STRING_HPP_INCLUDED
