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

#include "FastEngine/graphic/C_transformable.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <cmath>

namespace fge
{

Transformable::Transformable() :
        g_origin(0.0f, 0.0f),
        g_position(0.0f, 0.0f),
        g_rotation(0.0f),
        g_scale(1.0f, 1.0f),
        g_transform(1.0f),
        g_transformNeedUpdate(true),
        g_inverseTransform(1.0f),
        g_inverseTransformNeedUpdate(true)
{}

void Transformable::setPosition(Vector2f const& position)
{
    this->g_position = position;
    this->g_transformNeedUpdate = true;
    this->g_inverseTransformNeedUpdate = true;
}
Vector2f const& Transformable::getPosition() const
{
    return this->g_position;
}
void Transformable::move(Vector2f const& offset)
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

void Transformable::setScale(Vector2f const& factors)
{
    this->g_scale = factors;
    this->g_transformNeedUpdate = true;
    this->g_inverseTransformNeedUpdate = true;
}
Vector2f const& Transformable::getScale() const
{
    return this->g_scale;
}
void Transformable::scale(Vector2f const& factor)
{
    this->setScale(this->g_scale * factor);
}

void Transformable::setOrigin(Vector2f const& origin)
{
    this->g_origin = origin;
    this->g_transformNeedUpdate = true;
    this->g_inverseTransformNeedUpdate = true;
}
Vector2f const& Transformable::getOrigin() const
{
    return this->g_origin;
}

glm::mat4 const& Transformable::getTransform() const
{
    if (this->g_transformNeedUpdate)
    {
        this->g_transform = glm::translate(glm::mat4(1.0f), glm::vec3(this->g_position, 0.0f));

        this->g_transform = glm::rotate(this->g_transform, glm::radians(this->g_rotation), glm::vec3(0.0f, 0.0f, 1.0f));

        this->g_transform = glm::scale(this->g_transform, glm::vec3(this->g_scale, 1.0f));

        this->g_transform = glm::translate(this->g_transform, glm::vec3(-this->g_origin, 0.0f));

        this->g_transformNeedUpdate = false;
    }

    return this->g_transform;
}

glm::mat4 const& Transformable::getInverseTransform() const
{
    if (this->g_inverseTransformNeedUpdate)
    {
        this->g_inverseTransform = glm::inverse(this->getTransform());
        this->g_inverseTransformNeedUpdate = false;
    }

    return this->g_inverseTransform;
}

} // namespace fge
