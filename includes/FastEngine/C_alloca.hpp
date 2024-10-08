/*
 * Copyright 2024 Guillaume Guillet
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

#ifndef _FGE_C_ALLOCA_HPP_INCLUDED
#define _FGE_C_ALLOCA_HPP_INCLUDED

#if defined(_MSC_VER)
    #include <malloc.h>
    #define FGE_ALLOCA_T(_type, _size) reinterpret_cast<_type*>(_alloca(sizeof(_type) * _size))
    #define FGE_ALLOCA(_size) _alloca(_size)
#else
    #define FGE_ALLOCA_T(_type, _size) reinterpret_cast<_type*>(__builtin_alloca(sizeof(_type) * _size))
    #define FGE_ALLOCA(_size) __builtin_alloca(_size)
#endif

#define FGE_PLACE_CONSTRUCT(_type, _size, _ptr) new (_ptr) _type[_size]
#define FGE_PLACE_DESTRUCT(_type, _size, _ptr)                                                                         \
    for (std::size_t _iii = 0; _iii < _size; ++_iii) (_ptr)[_iii].~_type()

#define FGE_ALLOCA_STRINGVIEW_TO_CSTRING(_var, _str)                                                                   \
    _var = FGE_ALLOCA_T(char, _str.size() + 1);                                                                        \
    std::memcpy(_var, _str.data(), _str.size());                                                                       \
    _var[_str.size()] = '\0'

#endif // _FGE_C_ALLOCA_HPP_INCLUDED
