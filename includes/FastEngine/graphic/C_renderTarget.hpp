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

#ifndef _FGE_VULKAN_C_RENDERTARGET_HPP_INCLUDED
#define _FGE_VULKAN_C_RENDERTARGET_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fastengine_extern.hpp"
#include <SDL_video.h>
#include <FastEngine/graphic/C_view.hpp>
#include <glm/glm.hpp>
#include <FastEngine/vulkan/C_blendMode.hpp>
#include <FastEngine/graphic/C_renderStates.hpp>
#include <FastEngine/graphic/C_rect.hpp>
#include <FastEngine/graphic/C_color.hpp>
#include <FastEngine/vulkan/C_graphicPipeline.hpp>
#include <FastEngine/vulkan/C_vertex.hpp>

#define BAD_IMAGE_INDEX std::numeric_limits<uint32_t>::max()

namespace fge
{

class Drawable;

class FGE_API RenderTarget
{
protected:
    RenderTarget();

    void initialize();

public:
    virtual ~RenderTarget() = default;

    void setClearColor(const fge::Color& color);
    [[nodiscard]] fge::Color getClearColor() const;

    void setView(const View& view);
    const View& getView() const;
    const View& getDefaultView() const;
    fge::vulkan::Viewport getViewport(const View& view) const;

    Vector2f mapPixelToCoords(const Vector2i& point) const;
    Vector2f mapPixelToCoords(const Vector2i& point, const View& view) const;
    Vector2i mapCoordsToPixel(const Vector2f& point) const;
    Vector2i mapCoordsToPixel(const Vector2f& point, const View& view) const;

    [[nodiscard]] virtual uint32_t prepareNextFrame(const VkCommandBufferInheritanceInfo* inheritanceInfo) = 0;
    virtual void beginRenderPass(uint32_t imageIndex) = 0;
    void draw(const Drawable& drawable, const RenderStates& states);
    virtual void draw(const fge::vulkan::GraphicPipeline& graphicPipeline, const RenderStates& states) = 0;
    virtual void endRenderPass() = 0;
    virtual void display(uint32_t imageIndex, const VkCommandBuffer* extraCommandBuffer, std::size_t extraCommandBufferSize) = 0;

    virtual Vector2u getSize() const = 0;

    virtual bool isSrgb() const;

private:
    View g_defaultView;
    View g_view;

protected:
    VkClearColorValue _g_clearColor;
};

}//end fge

#endif // _FGE_VULKAN_C_RENDERTARGET_HPP_INCLUDED
