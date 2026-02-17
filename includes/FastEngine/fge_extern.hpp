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

#ifndef _FGE_FGE_EXTERN_HPP_INCLUDED
    #define _FGE_FGE_EXTERN_HPP_INCLUDED

    #ifndef _WIN32
        #define FGE_API
    #else
        #ifdef _FGE_DEF_BUILDDLL
            #define FGE_API __declspec(dllexport)
        #else
            #define FGE_API __declspec(dllimport)
        #endif // _FGE_DEF_BUILDDLL
    #endif     //_WIN32

#endif // _FGE_FGE_EXTERN_HPP_INCLUDED

/**
 * \defgroup objectControl Object control
 * \brief Everything related to objects
 *
 * \defgroup network Network
 * \brief Everything related to network
 *
 * \defgroup networkRules Network rules
 * \brief Everything related to network rules
 *
 * Network rules are utilities that make sure that the data inside of a packet is
 * valid before extracting and using it.
 *
 * \defgroup utility Utility/Tools
 * \brief Everything related to some utility/tools
 *
 * \defgroup callback Callback
 * \brief Everything related to callback
 *
 * \defgroup time Time utility/tools
 * \brief Everything related to time control
 *
 * \defgroup extraString Extra string utility/tools
 * \brief Everything related to strings
 *
 * \defgroup animation Animation utility/tools
 * \brief Everything related to animation
 *
 * \defgroup audio Audio utility/tools
 * \brief Everything related to audio
 *
 * \defgroup graphics Graphics utility/tools
 * \brief Everything related to graphics
 *
 * \defgroup vulkan Vulkan abstraction
 * \brief Everything related to the Vulkan abstraction
 *
 * \defgroup managers Global managers
 * \brief Everything related to global managers
 */
