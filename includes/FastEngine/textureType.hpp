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

#ifndef _FGE_TEXTURETYPE_HPP_INCLUDED
#define _FGE_TEXTURETYPE_HPP_INCLUDED

#ifdef FGE_DEF_SERVER
    #include "FastEngine/graphic/C_surface.hpp"
#else
    #include "FastEngine/vulkan/C_textureImage.hpp"
#endif //FGE_DEF_SERVER

namespace fge
{
#ifdef FGE_DEF_SERVER
using TextureType = fge::Surface;
#else
using TextureType = fge::vulkan::TextureImage;
#endif //FGE_DEF_SERVER
} // namespace fge

#endif //_FGE_TEXTURETYPE_HPP_INCLUDED
