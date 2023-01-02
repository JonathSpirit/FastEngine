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

#include <FastEngine/graphic/C_renderTarget.hpp>
#include <FastEngine/graphic/C_drawable.hpp>
#include <FastEngine/vulkan/C_textureImage.hpp>
#include <FastEngine/vulkan/C_context.hpp>

namespace fge
{

RenderTarget::RenderTarget() :
    g_defaultView(),
    g_view(),
    _g_clearColor()
{}

void RenderTarget::initialize()
{
    this->g_defaultView.reset({0.0f, 0.0f,
                               static_cast<float>(this->getSize().x),
                               static_cast<float>(this->getSize().y)});
    this->g_view = this->g_defaultView;
}

void RenderTarget::setClearColor(const fge::Color& color)
{
    this->_g_clearColor = color;
}
fge::Color RenderTarget::getClearColor() const
{
    return fge::Color(this->_g_clearColor);
}

void RenderTarget::setView(const View& view)
{
    this->g_view = view;
}
const View& RenderTarget::getView() const
{
    return this->g_view;
}
const View& RenderTarget::getDefaultView() const
{
    return this->g_defaultView;
}
fge::vulkan::Viewport RenderTarget::getViewport(const View& view) const
{
    auto size = static_cast<Vector2f>(this->getSize());
    const auto& viewport = view.getFactorViewport();

    return {size.x * viewport._x,
            size.y * viewport._y,
            size.x * viewport._width,
            size.y * viewport._height};
}

Vector2f RenderTarget::mapPixelToCoords(const Vector2i& point) const
{
    return this->mapPixelToCoords(point, this->getView());
}
Vector2f RenderTarget::mapPixelToCoords(const Vector2i& point, const View& view) const
{
    // First, convert from viewport coordinates to homogeneous coordinates
    glm::vec4 normalized;
    auto viewport = this->getViewport(view);
    normalized.x = -1.f + 2.f * (static_cast<float>(point.x) - viewport.getPositionX()) / viewport.getWidth();
    normalized.y =  1.f - 2.f * (static_cast<float>(point.y) - viewport.getPositionY())  / viewport.getHeight();
    normalized.z = 0.0f;
    normalized.w = 1.0f;

    // Then transform by the inverse of the view matrix
    const fge::Vector2f test = view.getInverseTransform() * normalized;
    return test;
}
Vector2i RenderTarget::mapCoordsToPixel(const Vector2f& point) const
{
    return this->mapCoordsToPixel(point, this->getView());
}
Vector2i RenderTarget::mapCoordsToPixel(const Vector2f& point, const View& view) const
{
    glm::vec4 pointVec4(point, 0.0f, 0.0f);

    // First, transform the point by the view matrix
    glm::vec4 normalized = view.getTransform() * pointVec4;

    // Then convert to viewport coordinates
    Vector2i pixel;
    auto viewport = this->getViewport(view);
    pixel.x = static_cast<int>(( normalized.x + 1.f) / 2.f * viewport.getWidth()  + viewport.getPositionX());
    pixel.y = static_cast<int>((-normalized.y + 1.f) / 2.f * viewport.getHeight() + viewport.getPositionY());

    return pixel;
}

void RenderTarget::draw(const Drawable& drawable, const RenderStates& states)
{
    drawable.draw(*this, states);
}

bool RenderTarget::isSrgb() const
{
    return false;
}

}//end fge
