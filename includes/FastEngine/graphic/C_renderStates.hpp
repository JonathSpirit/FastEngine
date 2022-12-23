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

#ifndef _FGE_VULKAN_C_RENDERSTATES_HPP_INCLUDED
#define _FGE_VULKAN_C_RENDERSTATES_HPP_INCLUDED

#include <glm/glm.hpp>

namespace fge
{

namespace vulkan
{

class TextureImage;

}//end vulkan

class Transformable;

class RenderStates
{
public:
    RenderStates() = default;
    explicit RenderStates(const fge::Transformable* transformable) :
            _transformable(transformable)
    {}
    explicit RenderStates(const fge::vulkan::TextureImage* textureImage) :
            _textureImage(textureImage)
    {}
    RenderStates(const fge::Transformable* transformable, const fge::vulkan::TextureImage* textureImage) :
            _transformable(transformable),
            _textureImage(textureImage)
    {}

    //glm::mat4 _modelTransform{1.0f};
    const fge::Transformable* _transformable{nullptr};
    const fge::vulkan::TextureImage* _textureImage{nullptr};
};

}//end fge

#endif // _FGE_VULKAN_C_RENDERSTATES_HPP_INCLUDED
