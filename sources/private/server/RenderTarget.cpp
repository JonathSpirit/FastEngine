////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2022 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

//modified by Guillaume Guillet for FastEngine server compatibility

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace sf
{
////////////////////////////////////////////////////////////
RenderTarget::RenderTarget() :
        m_defaultView(),
        m_view(),
        m_cache(),
        m_id(0)
{
    m_cache.glStatesSet = false;
}


////////////////////////////////////////////////////////////
RenderTarget::~RenderTarget() = default;


////////////////////////////////////////////////////////////
void RenderTarget::clear([[maybe_unused]] const Color& color) {}


////////////////////////////////////////////////////////////
void RenderTarget::setView(const View& view)
{
    m_view = view;
    m_cache.viewChanged = true;
}


////////////////////////////////////////////////////////////
const View& RenderTarget::getView() const
{
    return m_view;
}


////////////////////////////////////////////////////////////
const View& RenderTarget::getDefaultView() const
{
    return m_defaultView;
}


////////////////////////////////////////////////////////////
IntRect RenderTarget::getViewport(const View& view) const
{
    float width = static_cast<float>(getSize().x);
    float height = static_cast<float>(getSize().y);
    const FloatRect& viewport = view.getViewport();

    return IntRect(static_cast<int>(0.5f + width * viewport.left), static_cast<int>(0.5f + height * viewport.top),
                   static_cast<int>(0.5f + width * viewport.width), static_cast<int>(0.5f + height * viewport.height));
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(const Vector2i& point) const
{
    return mapPixelToCoords(point, getView());
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(const Vector2i& point, const View& view) const
{
    // First, convert from viewport coordinates to homogeneous coordinates
    Vector2f normalized;
    FloatRect viewport = FloatRect(getViewport(view));
    normalized.x = -1.f + 2.f * (static_cast<float>(point.x) - viewport.left) / viewport.width;
    normalized.y = 1.f - 2.f * (static_cast<float>(point.y) - viewport.top) / viewport.height;

    // Then transform by the inverse of the view matrix
    return view.getInverseTransform().transformPoint(normalized);
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(const Vector2f& point) const
{
    return mapCoordsToPixel(point, getView());
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(const Vector2f& point, const View& view) const
{
    // First, transform the point by the view matrix
    Vector2f normalized = view.getTransform().transformPoint(point);

    // Then convert to viewport coordinates
    Vector2i pixel;
    FloatRect viewport = FloatRect(getViewport(view));
    pixel.x = static_cast<int>((normalized.x + 1.f) / 2.f * viewport.width + viewport.left);
    pixel.y = static_cast<int>((-normalized.y + 1.f) / 2.f * viewport.height + viewport.top);

    return pixel;
}


////////////////////////////////////////////////////////////
void RenderTarget::draw([[maybe_unused]] const Drawable& drawable, [[maybe_unused]] const RenderStates& states) {}


////////////////////////////////////////////////////////////
void RenderTarget::draw([[maybe_unused]] const Vertex* vertices,
                        [[maybe_unused]] std::size_t vertexCount,
                        [[maybe_unused]] PrimitiveType type,
                        [[maybe_unused]] const RenderStates& states)
{}


////////////////////////////////////////////////////////////
void RenderTarget::draw([[maybe_unused]] const VertexBuffer& vertexBuffer, [[maybe_unused]] const RenderStates& states)
{}


////////////////////////////////////////////////////////////
void RenderTarget::draw([[maybe_unused]] const VertexBuffer& vertexBuffer,
                        [[maybe_unused]] std::size_t firstVertex,
                        [[maybe_unused]] std::size_t vertexCount,
                        [[maybe_unused]] const RenderStates& states)
{}


////////////////////////////////////////////////////////////
bool RenderTarget::isSrgb() const
{
    // By default sRGB encoding is not enabled for an arbitrary RenderTarget
    return false;
}


////////////////////////////////////////////////////////////
bool RenderTarget::setActive([[maybe_unused]] bool active)
{
    return true;
}


////////////////////////////////////////////////////////////
void RenderTarget::pushGLStates() {}


////////////////////////////////////////////////////////////
void RenderTarget::popGLStates() {}


////////////////////////////////////////////////////////////
void RenderTarget::resetGLStates() {}


////////////////////////////////////////////////////////////
void RenderTarget::initialize() {}


////////////////////////////////////////////////////////////
void RenderTarget::applyCurrentView() {}


////////////////////////////////////////////////////////////
void RenderTarget::applyBlendMode([[maybe_unused]] const BlendMode& mode) {}


////////////////////////////////////////////////////////////
void RenderTarget::applyTransform([[maybe_unused]] const Transform& transform) {}


////////////////////////////////////////////////////////////
void RenderTarget::applyTexture([[maybe_unused]] const Texture* texture) {}


////////////////////////////////////////////////////////////
void RenderTarget::applyShader([[maybe_unused]] const Shader* shader) {}


////////////////////////////////////////////////////////////
void RenderTarget::setupDraw([[maybe_unused]] bool useVertexCache, [[maybe_unused]] const RenderStates& states) {}


////////////////////////////////////////////////////////////
void RenderTarget::drawPrimitives([[maybe_unused]] PrimitiveType type,
                                  [[maybe_unused]] std::size_t firstVertex,
                                  [[maybe_unused]] std::size_t vertexCount)
{}


////////////////////////////////////////////////////////////
void RenderTarget::cleanupDraw([[maybe_unused]] const RenderStates& states) {}

} // namespace sf


////////////////////////////////////////////////////////////
// Render states caching strategies
//
// * View
//   If SetView was called since last draw, the projection
//   matrix is updated. We don't need more, the view doesn't
//   change frequently.
//
// * Transform
//   The transform matrix is usually expensive because each
//   entity will most likely use a different transform. This can
//   lead, in worst case, to changing it every 4 vertices.
//   To avoid that, when the vertex count is low enough, we
//   pre-transform them and therefore use an identity transform
//   to render them.
//
// * Blending mode
//   Since it overloads the == operator, we can easily check
//   whether any of the 6 blending components changed and,
//   thus, whether we need to update the blend mode.
//
// * Texture
//   Storing the pointer or OpenGL ID of the last used texture
//   is not enough; if the sf::Texture instance is destroyed,
//   both the pointer and the OpenGL ID might be recycled in
//   a new texture instance. We need to use our own unique
//   identifier system to ensure consistent caching.
//
// * Shader
//   Shaders are very hard to optimize, because they have
//   parameters that can be hard (if not impossible) to track,
//   like matrices or textures. The only optimization that we
//   do is that we avoid setting a null shader if there was
//   already none for the previous draw.
//
////////////////////////////////////////////////////////////
