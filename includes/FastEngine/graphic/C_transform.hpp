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

#ifndef _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED
#define _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_uniformBuffer.hpp"
#include "glm/glm.hpp"

namespace fge
{

class Transformable;

struct TransformUboData
{
    alignas(16) mutable glm::mat4 _modelTransform{1.0f};
    alignas(16) mutable glm::mat4 _viewTransform{1.0f};

    constexpr static unsigned int uboSize = sizeof(_modelTransform) + sizeof(_viewTransform);
};

class FGE_API Transform
{
public:
    Transform() = default;
    ~Transform();

    const Transform* start(const fge::Transformable& transformable,
                           const fge::Transform* parentTransform = nullptr) const;
    const Transform* start(const fge::Transform* parentTransform) const;

    void destroy();

    void updateUniformBuffer(const fge::vulkan::Context& context) const;
    [[nodiscard]] const fge::vulkan::DescriptorSet& getDescriptorSet() const;
    [[nodiscard]] const fge::vulkan::UniformBuffer& getUniformBuffer() const;

    TransformUboData _data;

private:
    mutable fge::vulkan::DescriptorSet g_descriptorSet;
    mutable fge::vulkan::UniformBuffer g_uniformBuffer;
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED