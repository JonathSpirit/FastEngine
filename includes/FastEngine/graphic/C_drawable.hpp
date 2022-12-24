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

#ifndef _FGE_VULKAN_C_DRAWABLE_HPP_INCLUDED
#define _FGE_VULKAN_C_DRAWABLE_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include <FastEngine/vulkan/C_graphicPipeline.hpp>
#include <FastEngine/graphic/C_renderStates.hpp>
#include <FastEngine/graphic/C_renderTarget.hpp>

namespace fge
{

class Drawable
{
public:
    Drawable() = default;
    virtual ~Drawable() = default;

    virtual void draw(RenderTarget& target, const RenderStates& states) const = 0;

protected:
    fge::vulkan::GraphicPipeline _g_graphicPipeline;
};

}//end fge


#endif // _FGE_VULKAN_C_DRAWABLE_HPP_INCLUDED
