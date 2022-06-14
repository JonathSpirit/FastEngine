/*
 * Copyright 2022 Guillaume Guillet
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

#ifndef _FGE_C_SMOOTHFLOAT_HPP_INCLUDED
#define _FGE_C_SMOOTHFLOAT_HPP_INCLUDED

#include <SFML/System/Vector2.hpp>

namespace fge
{
namespace net
{

struct SmoothVec2Float
{
    sf::Vector2f _real;
    float _errorRange;

    sf::Vector2f _cache;
};

struct SmoothFloat
{
    float _real;
    float _errorRange;

    float _cache;
};

}//end net
}//end fge

#endif // _FGE_C_SMOOTHFLOAT_HPP_INCLUDED
