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

#ifndef _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED

#include "FastEngine/textureType.hpp"
#include "FastEngine/vulkan/C_blendMode.hpp"
#include "glm/glm.hpp"

namespace fge
{

namespace vulkan
{

class TextureImage;
class VertexBuffer;
class IndexBuffer;

} // namespace vulkan

class Transform;

class RenderStates
{
public:
    RenderStates() = default;
    RenderStates(const RenderStates& r) = delete;
    RenderStates(RenderStates&& r) noexcept = default;
    explicit RenderStates(const fge::Transform* transform, const fge::TextureType* textureImage = nullptr) :
            _transform(transform),
            _textureImage(textureImage)
    {}
    RenderStates(const fge::Transform* transform,
                 const fge::vulkan::VertexBuffer* vertexBuffer,
                 const fge::TextureType* textureImage = nullptr,
                 const fge::vulkan::BlendMode& blendMode = {}) :
            _transform(transform),
            _textureImage(textureImage),
            _vertexBuffer(vertexBuffer),
            _blendMode(blendMode)
    {}

    [[nodiscard]] RenderStates copy(const fge::Transform* transform,
                                    const fge::TextureType* textureImage = nullptr) const
    {
        return RenderStates{transform, nullptr, textureImage, this->_blendMode};
    }

    RenderStates& operator=(const RenderStates& r) = delete;
    RenderStates& operator=(RenderStates&& r) noexcept = default;

    const fge::Transform* _transform{nullptr};
    const fge::TextureType* _textureImage{nullptr};
    const fge::vulkan::VertexBuffer* _vertexBuffer{nullptr};
    const fge::vulkan::IndexBuffer* _indexBuffer{nullptr};
    fge::vulkan::BlendMode _blendMode{};
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_RENDERSTATES_HPP_INCLUDED
