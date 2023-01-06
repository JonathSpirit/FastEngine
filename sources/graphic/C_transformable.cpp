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

#include <FastEngine/graphic/C_transformable.hpp>
#include <cmath>
#include "glm/ext/matrix_transform.hpp"

namespace fge
{

Transformable::Transformable() :
        g_origin                    (0.0f, 0.0f),
        g_position                  (0.0f, 0.0f),
        g_rotation                  (0.0f),
        g_scale                     (1.0f, 1.0f),
        g_transform                 (1.0f),
        g_transformNeedUpdate       (true),
        g_inverseTransform          (1.0f),
        g_inverseTransformNeedUpdate(true),
        g_descriptorSet()
{}
Transformable::~Transformable()
{
    this->destroy();
}

void Transformable::setPosition(const Vector2f& position)
{
    this->g_position = position;
    this->g_transformNeedUpdate = true;
    this->g_inverseTransformNeedUpdate = true;
}
const Vector2f& Transformable::getPosition() const
{
    return this->g_position;
}
void Transformable::move(const Vector2f& offset)
{
    this->setPosition(this->g_position + offset);
}

void Transformable::setRotation(float angle)
{
    this->g_rotation = std::fmod(angle, 360.f);
    if (this->g_rotation < 0.0f)
    {
        this->g_rotation += 360.f;
    }

    this->g_transformNeedUpdate = true;
    this->g_inverseTransformNeedUpdate = true;
}
float Transformable::getRotation() const
{
    return this->g_rotation;
}
void Transformable::rotate(float angle)
{
    this->setRotation(this->g_rotation + angle);
}

void Transformable::setScale(const Vector2f& factors)
{
    this->g_scale = factors;
    this->g_transformNeedUpdate = true;
    this->g_inverseTransformNeedUpdate = true;
}
const Vector2f& Transformable::getScale() const
{
    return this->g_scale;
}
void Transformable::scale(const Vector2f& factor)
{
    this->setScale(this->g_scale * factor);
}

void Transformable::setOrigin(const Vector2f& origin)
{
    this->g_origin = origin;
    this->g_transformNeedUpdate = true;
    this->g_inverseTransformNeedUpdate = true;
}
const Vector2f& Transformable::getOrigin() const
{
    return this->g_origin;
}

const glm::mat4& Transformable::getTransform() const
{
    if (this->g_transformNeedUpdate)
    {
        this->g_transform = glm::translate(glm::mat4(1.0f), glm::vec3(this->g_position, 0.0f));

        this->g_transform = glm::scale(this->g_transform, glm::vec3(this->g_scale, 1.0f));

        this->g_transform = glm::translate(this->g_transform, glm::vec3(-this->g_origin, 0.0f));

        this->g_transform = glm::rotate(this->g_transform,
                                        glm::radians(this->g_rotation),
                                        glm::vec3(0.0f, 0.0f, 1.0f));

        this->g_transformNeedUpdate = false;
    }

    return this->g_transform;
}

const glm::mat4& Transformable::getInverseTransform() const
{
    if (this->g_inverseTransformNeedUpdate)
    {
        this->g_inverseTransform = glm::inverse(this->getTransform());
        this->g_inverseTransformNeedUpdate = false;
    }

    return this->g_inverseTransform;
}

void Transformable::destroy()
{
    this->g_descriptorSet.destroy();
    this->g_uniformBuffer.destroy();
}

void Transformable::updateUniformBuffer(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const fge::vulkan::Context& context) const
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

    const fge::Transform transform{modelMatrix, viewMatrix};
    this->g_uniformBuffer.copyData(&transform, sizeof(transform));
}
const fge::vulkan::DescriptorSet& Transformable::getDescriptorSet() const
{
    return this->g_descriptorSet;
}

}//end fge
