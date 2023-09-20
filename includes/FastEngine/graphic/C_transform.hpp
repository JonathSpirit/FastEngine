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

#ifndef _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED
#define _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_uniformBuffer.hpp"

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
    explicit Transform(fge::vulkan::Context const& context = fge::vulkan::GetActiveContext());
    Transform(Transform const& r);
    Transform(Transform&& r) noexcept = default;
    ~Transform();

    Transform& operator=(Transform const& r);
    Transform& operator=(Transform&& r) noexcept = default;

    Transform const* start(fge::Transformable const& transformable,
                           fge::Transform const* parentTransform = nullptr) const;
    Transform const* start(fge::Transform const* parentTransform) const;

    void destroy();

    void recreateUniformBuffer(fge::vulkan::Context const& context);

    [[nodiscard]] fge::vulkan::DescriptorSet const& getDescriptorSet() const;
    [[nodiscard]] fge::vulkan::UniformBuffer const& getUniformBuffer() const;

    [[nodiscard]] TransformUboData& getData() const;

private:
#ifndef FGE_DEF_SERVER
    mutable fge::vulkan::DescriptorSet g_descriptorSet;
    mutable fge::vulkan::UniformBuffer g_uniformBuffer;
#else
    mutable TransformUboData g_uboData;
#endif
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_TRANSFORM_HPP_INCLUDED
