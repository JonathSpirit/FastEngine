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

#ifndef _FGE_GRAPHIC_C_RENDERTARGET_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RENDERTARGET_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "FastEngine/graphic/C_renderStates.hpp"
#include "FastEngine/graphic/C_view.hpp"
#include "FastEngine/vulkan/C_blendMode.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_graphicPipeline.hpp"
#include "FastEngine/vulkan/C_vertex.hpp"
#include "SDL_video.h"
#include "glm/glm.hpp"
#include <unordered_map>

#define BAD_IMAGE_INDEX std::numeric_limits<uint32_t>::max()

namespace fge
{

class Drawable;

class FGE_API RenderTarget
{
protected:
    explicit RenderTarget(const fge::vulkan::Context& context);

    void initialize();

public:
    RenderTarget(const RenderTarget& r);
    RenderTarget(RenderTarget&& r) noexcept;
    virtual ~RenderTarget() = default;

    RenderTarget& operator=(const RenderTarget& r);
    RenderTarget& operator=(RenderTarget&& r) noexcept;

    void setClearColor(const fge::Color& color);
    [[nodiscard]] fge::Color getClearColor() const;

    void setView(const View& view);
    [[nodiscard]] const View& getView() const;
    [[nodiscard]] const View& getDefaultView() const;
    [[nodiscard]] fge::vulkan::Viewport getViewport(const View& view) const;

    [[nodiscard]] Vector2f mapPixelToCoords(const Vector2i& point) const;
    [[nodiscard]] Vector2f mapPixelToCoords(const Vector2i& point, const View& view) const;
    [[nodiscard]] Vector2i mapCoordsToPixel(const Vector2f& point) const;
    [[nodiscard]] Vector2i mapCoordsToPixel(const Vector2f& point, const View& view) const;

    virtual uint32_t prepareNextFrame(const VkCommandBufferInheritanceInfo* inheritanceInfo) = 0;
    virtual void beginRenderPass(uint32_t imageIndex) = 0;
    void draw(const fge::Drawable& drawable, const fge::RenderStates& states);
    void draw(const fge::RenderStates& states);
    void draw(const fge::vulkan::GraphicPipeline& graphicPipeline, const fge::RenderStates& states);
    virtual void endRenderPass() = 0;
    virtual void display(uint32_t imageIndex) = 0;

    void drawBatches(const fge::vulkan::BlendMode& blendMode,
                     const fge::vulkan::TextureImage* textureImage,
                     const fge::vulkan::DescriptorSet& transformDescriptorSet,
                     const fge::vulkan::VertexBuffer* vertexBuffer,
                     uint32_t vertexCount,
                     uint32_t instanceCount);
    void drawBatches(const fge::vulkan::GraphicPipeline& graphicPipeline,
                     const fge::vulkan::TextureImage* textureImage,
                     const fge::vulkan::DescriptorSet& transformDescriptorSet,
                     const fge::vulkan::VertexBuffer* vertexBuffer,
                     uint32_t vertexCount,
                     uint32_t instanceCount);

    virtual void pushExtraCommandBuffer(VkCommandBuffer commandBuffer) const;
    virtual void pushExtraCommandBuffer(const std::vector<VkCommandBuffer>& commandBuffers) const;

    virtual Vector2u getSize() const = 0;

    [[nodiscard]] const fge::vulkan::Context* getContext() const;

    virtual bool isSrgb() const;

    [[nodiscard]] virtual VkExtent2D getExtent2D() const = 0;
    [[nodiscard]] virtual VkCommandBuffer getCommandBuffer() const = 0;
    [[nodiscard]] virtual VkRenderPass getRenderPass() const = 0;

private:
    View g_defaultView;
    View g_view;

protected:
    VkClearColorValue _g_clearColor;

    const fge::vulkan::Context* _g_context;

    bool _g_forceGraphicPipelineUpdate;

    std::unordered_map<fge::vulkan::BlendMode, fge::vulkan::GraphicPipeline, fge::vulkan::BlendModeHash>
            _g_defaultGraphicPipelineTexture;
    std::unordered_map<fge::vulkan::BlendMode, fge::vulkan::GraphicPipeline, fge::vulkan::BlendModeHash>
            _g_defaultGraphicPipelineNoTexture;

    std::unordered_map<fge::vulkan::BlendMode, fge::vulkan::GraphicPipeline, fge::vulkan::BlendModeHash>
            _g_defaultGraphicPipelineTextureBatches; ///TODO: maybe not have that many maps
    std::unordered_map<fge::vulkan::BlendMode, fge::vulkan::GraphicPipeline, fge::vulkan::BlendModeHash>
            _g_defaultGraphicPipelineNoTextureBatches;

    static const fge::vulkan::TextureImage* gLastTexture;
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_RENDERTARGET_HPP_INCLUDED
