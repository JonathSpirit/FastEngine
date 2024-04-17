/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/graphic/C_view.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>

namespace fge
{

View::View() :
        g_center(0.0f, 0.0f),
        g_size(0.0f, 0.0f),
        g_rotation(0.0f),
        g_factorViewport({0.0f, 0.0f}, {1.0f, 1.0f}),
        g_transform(),
        g_inverseTransform(),
        g_transformUpdated(false),
        g_invTransformUpdated(false)
{
    this->reset(fge::vulkan::Viewport(0.0f, 0.0f, 1000.0f, 1000.0f));
}
View::View(fge::vulkan::Viewport const& viewport) :
        g_center(0.0f, 0.0f),
        g_size(0.0f, 0.0f),
        g_rotation(0.0f),
        g_factorViewport(),
        g_transform(),
        g_inverseTransform(),
        g_transformUpdated(false),
        g_invTransformUpdated(false)
{
    this->reset(viewport);
}
View::View(Vector2f const& center, Vector2f const& size) :
        g_center(center),
        g_size(size),
        g_rotation(0.0f),
        g_factorViewport({0.0f, 0.0f}, {1.0f, 1.0f}),
        g_transform(),
        g_inverseTransform(),
        g_transformUpdated(false),
        g_invTransformUpdated(false)
{}

void View::setCenter(Vector2f const& center)
{
    this->g_center = center;

    this->g_transformUpdated = false;
    this->g_invTransformUpdated = false;
}
Vector2f const& View::getCenter() const
{
    return this->g_center;
}

void View::setSize(Vector2f const& size)
{
    this->g_size = size;

    this->g_transformUpdated = false;
    this->g_invTransformUpdated = false;
}
Vector2f const& View::getSize() const
{
    return this->g_size;
}

void View::setRotation(float angleDeg)
{
    this->g_rotation = std::fmod(angleDeg, 360.f);
    if (this->g_rotation < 0)
    {
        this->g_rotation += 360.f;
    }

    this->g_transformUpdated = false;
    this->g_invTransformUpdated = false;
}
float View::getRotation() const
{
    return this->g_rotation;
}

void View::setFactorViewport(fge::RectFloat const& factorViewport)
{
    this->g_factorViewport = factorViewport;
}
fge::RectFloat const& View::getFactorViewport() const
{
    return this->g_factorViewport;
}

void View::reset(fge::vulkan::Viewport const& viewport)
{
    this->g_center.x = viewport.getPositionX() + viewport.getWidth() / 2.0f;
    this->g_center.y = viewport.getPositionY() + viewport.getHeight() / 2.0f;
    this->g_size.x = viewport.getWidth();
    this->g_size.y = viewport.getHeight();
    this->g_rotation = 0.0f;

    this->g_transformUpdated = false;
    this->g_invTransformUpdated = false;
}
void View::move(Vector2f const& offset)
{
    this->setCenter(this->g_center + offset);
}
void View::rotate(float angle)
{
    this->setRotation(this->g_rotation + angle);
}
void View::zoom(float factor)
{
    this->setSize({this->g_size.x * factor, this->g_size.y * factor});
}
void View::resizeFixCenter(fge::Vector2f const& newSize)
{
    auto const oldSize = this->getSize();
    this->setSize({newSize.x, newSize.y});
    this->move({(newSize.x - oldSize.x) / 2.0f, (newSize.y - oldSize.y) / 2.0f});
}

glm::mat4 const& View::getTransform() const
{
    if (!this->g_transformUpdated)
    {
        this->g_transform = glm::ortho<float>(0.0f, this->g_size.x, this->g_size.y, 0.0f);

        this->g_transform = glm::translate(this->g_transform, glm::vec3{this->g_size / 2.0f - this->g_center, 0.0f});

        this->g_transform = glm::translate(this->g_transform, glm::vec3{this->g_center, 0.0f});
        this->g_transform =
                glm::rotate(this->g_transform, glm::radians(this->g_rotation), glm::vec3(0.0f, 0.0f, -1.0f));
        this->g_transform = glm::translate(this->g_transform, glm::vec3{-this->g_center, 0.0f});

        this->g_transformUpdated = true;
    }

    return this->g_transform;
}
glm::mat4 const& View::getInverseTransform() const
{
    if (!this->g_invTransformUpdated)
    {
        this->g_inverseTransform = glm::inverse(this->getTransform());
        this->g_invTransformUpdated = true;
    }

    return this->g_inverseTransform;
}
glm::mat4 View::getProjectionMatrix() const
{
    return glm::ortho<float>(0.0f, this->g_size.x, this->g_size.y, 0.0f);
}

} // namespace fge
