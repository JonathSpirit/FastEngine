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

#ifndef _FGE_GRAPHIC_SHADERRESSOURCES_HPP_INCLUDED
#define _FGE_GRAPHIC_SHADERRESSOURCES_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"

namespace fge::res
{

/**
 * All shaders here is in GLSL language
 * \ingroup graphic
 * @{
 */

FGE_API extern const char gDefaultVertexShader[];
FGE_API extern const char gDefaultFragmentShader[];
FGE_API extern const char gDefaultFragmentTextureShader[];

FGE_API extern const unsigned int gDefaultVertexShaderSize;
FGE_API extern const unsigned int gDefaultFragmentShaderSize;
FGE_API extern const unsigned int gDefaultFragmentTextureShaderSize;

/**
 * @}
 */

} // namespace fge::res

#endif //_FGE_GRAPHIC_SHADERRESSOURCES_HPP_INCLUDED