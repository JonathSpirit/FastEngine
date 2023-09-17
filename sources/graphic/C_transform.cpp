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

#include "FastEngine/graphic/C_transform.hpp"
#include "FastEngine/graphic/C_transformable.hpp"

namespace fge
{

#ifndef FGE_DEF_SERVER

Transform::Transform(const fge::vulkan::Context& context) :
        g_uniformBuffer(context)
{
    this->g_descriptorSet = context.getTransformDescriptorPool()
                                    .allocateDescriptorSet(context.getTransformLayout().getLayout())
                                    .value();

    this->g_uniformBuffer.create(fge::TransformUboData::uboSize);
    const fge::vulkan::DescriptorSet::Descriptor descriptor(this->g_uniformBuffer, FGE_VULKAN_TRANSFORM_BINDING);
    this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);

    new (&this->getData()) fge::TransformUboData();
}

Transform::Transform(const Transform& r) :
        g_uniformBuffer(r.g_uniformBuffer.getContext())
{
    auto const& context = r.g_uniformBuffer.getContext();

    this->g_descriptorSet = context.getTransformDescriptorPool()
                                    .allocateDescriptorSet(context.getTransformLayout().getLayout())
                                    .value();

    this->g_uniformBuffer.create(fge::TransformUboData::uboSize);
    const fge::vulkan::DescriptorSet::Descriptor descriptor(this->g_uniformBuffer, FGE_VULKAN_TRANSFORM_BINDING);
    this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);

    new (&this->getData()) fge::TransformUboData();
}

#else

Transform::Transform([[maybe_unused]] const fge::vulkan::Context& context) {}

Transform::Transform(const Transform& r) :
        g_uboData(r.g_uboData)
{}

#endif //FGE_DEF_SERVER

// clang-format off
Transform::~Transform()
{
    this->destroy();
}
// clang-format on

Transform& Transform::operator=(const Transform& r)
{
#ifndef FGE_DEF_SERVER
    if (this->g_uniformBuffer.getBuffer() == VK_NULL_HANDLE && r.g_uniformBuffer.getBuffer() != VK_NULL_HANDLE)
    {
        auto const& context = r.g_uniformBuffer.getContext();

        if (&context != &this->g_uniformBuffer.getContext())
        {
            this->g_uniformBuffer.swapContext(context);
        }

        this->g_descriptorSet = context.getTransformDescriptorPool()
                                        .allocateDescriptorSet(context.getTransformLayout().getLayout())
                                        .value();

        this->g_uniformBuffer.create(fge::TransformUboData::uboSize);
        const fge::vulkan::DescriptorSet::Descriptor descriptor(this->g_uniformBuffer, FGE_VULKAN_TRANSFORM_BINDING);
        this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);

        new (&this->getData()) fge::TransformUboData();
    }
#else
    this->g_uboData = r.g_uboData;
#endif
    return *this;
}

const Transform* Transform::start(const fge::Transformable& transformable, const fge::Transform* parentTransform) const
{
    if (parentTransform != nullptr)
    {
        this->getData()._modelTransform = parentTransform->getData()._modelTransform * transformable.getTransform();
    }
    else
    {
        this->getData()._modelTransform = transformable.getTransform();
    }
    return this;
}
const Transform* Transform::start(const fge::Transform* parentTransform) const
{
    if (parentTransform != nullptr)
    {
        this->getData()._modelTransform = parentTransform->getData()._modelTransform;
    }
    else
    {
        this->getData()._modelTransform = glm::mat4{1.0f};
    }
    return this;
}

void Transform::destroy()
{
#ifndef FGE_DEF_SERVER
    this->g_descriptorSet.destroy();
    this->g_uniformBuffer.destroy();
#else
    this->g_uboData = fge::TransformUboData();
#endif
}

void Transform::recreateUniformBuffer([[maybe_unused]] const fge::vulkan::Context& context)
{
    this->destroy();
#ifndef FGE_DEF_SERVER
    this->g_descriptorSet = context.getTransformDescriptorPool()
                                    .allocateDescriptorSet(context.getTransformLayout().getLayout())
                                    .value();

    this->g_uniformBuffer.swapContext(context);
    this->g_uniformBuffer.create(fge::TransformUboData::uboSize);
    const fge::vulkan::DescriptorSet::Descriptor descriptor(this->g_uniformBuffer, FGE_VULKAN_TRANSFORM_BINDING);
    this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);

    new (&this->getData()) fge::TransformUboData();
#endif
}

const fge::vulkan::DescriptorSet& Transform::getDescriptorSet() const
{
#ifndef FGE_DEF_SERVER
    return this->g_descriptorSet;
#else
    throw fge::Exception("unimplemented");
#endif
}
const fge::vulkan::UniformBuffer& Transform::getUniformBuffer() const
{
#ifndef FGE_DEF_SERVER
    return this->g_uniformBuffer;
#else
    throw fge::Exception("unimplemented");
#endif
}

TransformUboData& Transform::getData() const
{
#ifndef FGE_DEF_SERVER
    return *reinterpret_cast<TransformUboData*>(this->g_uniformBuffer.getBufferMapped());
#else
    return this->g_uboData;
#endif
}

} // namespace fge
