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

#include "FastEngine/graphic/C_transform.hpp"
#include "FastEngine/graphic/C_transformable.hpp"

namespace fge
{

Transform::~Transform()
{
    this->destroy();
}

const Transform* Transform::start(const fge::Transformable& transformable, const fge::Transform* parentTransform) const
{
    if (parentTransform != nullptr)
    {
        this->_modelTransform = parentTransform->_modelTransform * transformable.getTransform();
    }
    else
    {
        this->_modelTransform = transformable.getTransform();
    }
    return this;
}
const Transform* Transform::start(const fge::Transform* parentTransform) const
{
    if (parentTransform != nullptr)
    {
        this->_modelTransform = parentTransform->_modelTransform;
    }
    else
    {
        this->_modelTransform = glm::mat4{1.0f};
    }
    return this;
}

void Transform::destroy()
{
    this->g_descriptorSet.destroy();
    this->g_uniformBuffer.destroy();
}

void Transform::updateUniformBuffer(const fge::vulkan::Context& context) const
{
    if (this->g_descriptorSet.getDescriptorSet() == VK_NULL_HANDLE)
    {
        this->g_descriptorSet.create(context.getLogicalDevice(), &context.getTransformLayout(),
                                     1, context.getTransformDescriptorPool(), true);
    }

    if (this->g_uniformBuffer.getBuffer() == VK_NULL_HANDLE)
    {
        this->g_uniformBuffer.create(context, sizeof(fge::Transform));
        const fge::vulkan::DescriptorSet::Descriptor descriptor(this->g_uniformBuffer, FGE_VULKAN_TRANSFORM_BINDING);
        this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);
    }

    this->g_uniformBuffer.copyData(this, fge::Transform::uboSize);
}
const fge::vulkan::DescriptorSet& Transform::getDescriptorSet() const
{
    return this->g_descriptorSet;
}
const fge::vulkan::UniformBuffer& Transform::getUniformBuffer() const
{
    return this->g_uniformBuffer;
}

}//end fge
