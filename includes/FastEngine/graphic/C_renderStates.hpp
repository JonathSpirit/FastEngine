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

#ifndef _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include <glm/glm.hpp>

namespace fge
{

namespace vulkan
{

class TextureImage;
class VertexBuffer;

}//end vulkan

class Transformable;

class RenderStates
{
public:
    RenderStates() = default;
    RenderStates(const RenderStates& r) = delete;
    RenderStates(RenderStates&& r) noexcept = default;
    explicit RenderStates(const fge::Transformable* transformable, const fge::vulkan::TextureImage* textureImage=nullptr) :
            _transformable(transformable),
            _textureImage(textureImage)
    {}
    RenderStates(const fge::Transformable* transformable, const fge::vulkan::VertexBuffer* vertexBuffer, const fge::vulkan::TextureImage* textureImage=nullptr) :
            _transformable(transformable),
            _textureImage(textureImage),
            _vertexBuffer(vertexBuffer)
    {}
    RenderStates(const glm::mat4& modelTransform,
                 const fge::Transformable* transformable,
                 const fge::vulkan::VertexBuffer* vertexBuffer,
                 const fge::vulkan::TextureImage* textureImage=nullptr,
                 const fge::vulkan::BlendMode& blendMode={}) :
            _modelTransform(modelTransform),
            _transformable(transformable),
            _textureImage(textureImage),
            _vertexBuffer(vertexBuffer),
            _blendMode(blendMode)
    {}

    [[nodiscard]] RenderStates copy(const fge::Transformable* transformable, const fge::vulkan::TextureImage* textureImage=nullptr) const
    {
        return RenderStates{this->_modelTransform, transformable, nullptr, textureImage, this->_blendMode};
    }

    RenderStates& operator=(const RenderStates& r) = delete;
    RenderStates& operator=(RenderStates&& r) noexcept = default;

    glm::mat4 _modelTransform{1.0f};
    const fge::Transformable* _transformable{nullptr};
    const fge::vulkan::TextureImage* _textureImage{nullptr};
    const fge::vulkan::VertexBuffer* _vertexBuffer{nullptr};
    fge::vulkan::BlendMode _blendMode{};
};

}//end fge

#endif // _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED
