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
        g_transform                 {glm::mat4(1.0f), glm::mat4(1.0f)},
        g_transformNeedUpdate       (true),
        g_inverseTransform          (1.0f),
        g_inverseTransformNeedUpdate(true),
        g_descriptorSet(),
        g_uniformBufferNeedUpdate(true)
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
    this->g_uniformBufferNeedUpdate = true;
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
    this->g_uniformBufferNeedUpdate = true;
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
    this->g_uniformBufferNeedUpdate = true;
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
    this->g_uniformBufferNeedUpdate = true;
}
const Vector2f& Transformable::getOrigin() const
{
    return this->g_origin;
}

const glm::mat4& Transformable::getTransform() const
{
    if (this->g_transformNeedUpdate)
    {
        this->g_transform._modelTransform = glm::translate(this->g_transform._modelTransform,
                                                           glm::vec3(this->g_position, 0.0f));

        this->g_transform._modelTransform = glm::rotate(this->g_transform._modelTransform,
                                                        glm::radians(this->g_rotation),
                                                        glm::vec3(0.0f, 0.0f, 1.0f));

        this->g_transform._modelTransform = glm::scale(this->g_transform._modelTransform,
                                                       glm::vec3(this->g_scale, 1.0f));

        this->g_transform._modelTransform = glm::translate(this->g_transform._modelTransform, glm::vec3(-this->g_origin, 0.0f));

        this->g_transformNeedUpdate = false;
    }

    return this->g_transform._modelTransform;
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

void Transformable::setViewMatrix(const glm::mat4& viewMatrix) const
{
    this->g_transform._viewTransform = viewMatrix;
    this->g_uniformBufferNeedUpdate = true;
}
void Transformable::updateUniformBuffer(const fge::vulkan::Context& context) const
{
    if (this->g_descriptorSet.getDescriptorSet() == VK_NULL_HANDLE)
    {
        this->g_descriptorSet.create(context.getLogicalDevice(), &context.getDescriptorSetLayout(),
                                     1, context.getTransformDescriptorPool(), true);
        this->g_uniformBufferNeedUpdate = true;
    }

    if (this->g_uniformBuffer.getBuffer() == VK_NULL_HANDLE)
    {
        this->g_uniformBuffer.create(context.getLogicalDevice(), context.getPhysicalDevice(), sizeof(fge::Transform));
        const fge::vulkan::DescriptorSet::Descriptor descriptor(this->g_uniformBuffer, 0);
        this->g_descriptorSet.updateDescriptorSet(&descriptor, 1);
        this->g_uniformBufferNeedUpdate = true;
    }

    if (this->g_uniformBufferNeedUpdate)
    {
        this->g_transform._modelTransform = this->getTransform();
        this->g_uniformBuffer.copyData(&this->g_transform, sizeof(this->g_transform));
        this->g_uniformBufferNeedUpdate = false;
    }
}
const fge::vulkan::DescriptorSet& Transformable::getDescriptorSet() const
{
    return this->g_descriptorSet;
}

}//end fge
