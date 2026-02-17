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

#ifndef _FGE_FGE_DEBUG_HPP_INCLUDED
#define _FGE_FGE_DEBUG_HPP_INCLUDED

#ifdef FGE_DEF_DEBUG
    #include <format>
    #include <iostream>
    #define FGE_DEBUG_PRINT(_format, ...)                                                                              \
        std::cout << "FGE: " << __func__ << "() line:" << __LINE__ << " : "                                            \
                  << std::vformat(_format, std::make_format_args(__VA_ARGS__)) << std::endl
#else
    #define FGE_DEBUG_PRINT(...)
#endif // FGE_DEF_DEBUG

#endif // _FGE_FGE_DEBUG_HPP_INCLUDED