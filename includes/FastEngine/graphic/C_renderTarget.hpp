/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "FastEngine/graphic/C_renderStates.hpp"
#include "FastEngine/graphic/C_view.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/vulkan/C_commandBuffer.hpp"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include "FastEngine/vulkan/C_graphicPipeline.hpp"
#include <unordered_map>

#define FGE_RENDER_BAD_IMAGE_INDEX std::numeric_limits<uint32_t>::max()

#define FGE_RENDER_DEFAULT_DESCRIPTOR_SET_TRANSFORM 0
#define FGE_RENDER_DEFAULT_DESCRIPTOR_SET_TEXTURE 1

#define FGE_RENDER_TIMEOUT_BLOCKING UINT64_MAX
#define FGE_RENDER_NO_TIMEOUT 0

namespace fge
{

class Texture;
class Drawable;
class Transformable;
struct TransformUboData;

class FGE_API RenderTarget : public fge::vulkan::ContextAware
{
protected:
    explicit RenderTarget(fge::vulkan::Context const& context);

    void initialize();

public:
    RenderTarget(RenderTarget const& r);
    RenderTarget(RenderTarget&& r) noexcept;
    ~RenderTarget() override = default;

    RenderTarget& operator=(RenderTarget const& r);
    RenderTarget& operator=(RenderTarget&& r) noexcept;

    void setClearColor(fge::Color const& color);
    [[nodiscard]] fge::Color getClearColor() const;

    void setView(View const& view);
    [[nodiscard]] View const& getView() const;
    [[nodiscard]] View const& getDefaultView() const;
    [[nodiscard]] fge::vulkan::Viewport getViewport(View const& view) const;

    [[nodiscard]] Vector2f mapFramebufferCoordsToViewSpace(Vector2i const& point) const;
    [[nodiscard]] Vector2f mapFramebufferCoordsToViewSpace(Vector2i const& point, View const& view) const;
    [[nodiscard]] Vector2f mapFramebufferCoordsToWorldSpace(Vector2i const& point) const;
    [[nodiscard]] Vector2f mapFramebufferCoordsToWorldSpace(Vector2i const& point, View const& view) const;

    [[nodiscard]] Vector2i mapViewCoordsToFramebufferSpace(Vector2f const& point) const;
    [[nodiscard]] Vector2i mapViewCoordsToFramebufferSpace(Vector2f const& point, View const& view) const;
    [[nodiscard]] Vector2i mapWorldCoordsToFramebufferSpace(Vector2f const& point) const;
    [[nodiscard]] Vector2i mapWorldCoordsToFramebufferSpace(Vector2f const& point, View const& view) const;

    [[nodiscard]] RectFloat mapFramebufferRectToViewSpace(RectInt const& rect) const;
    [[nodiscard]] RectFloat mapFramebufferRectToViewSpace(RectInt const& rect, View const& view) const;
    [[nodiscard]] RectFloat mapFramebufferRectToWorldSpace(RectInt const& rect) const;
    [[nodiscard]] RectFloat mapFramebufferRectToWorldSpace(RectInt const& rect, View const& view) const;

    [[nodiscard]] RectInt mapViewRectToFramebufferSpace(RectFloat const& rect) const;
    [[nodiscard]] RectInt mapViewRectToFramebufferSpace(RectFloat const& rect, View const& view) const;
    [[nodiscard]] RectInt mapWorldRectToFramebufferSpace(RectFloat const& rect) const;
    [[nodiscard]] RectInt mapWorldRectToFramebufferSpace(RectFloat const& rect, View const& view) const;

    virtual uint32_t prepareNextFrame(VkCommandBufferInheritanceInfo const* inheritanceInfo, uint64_t timeout_ns) = 0;
    virtual void beginRenderPass(uint32_t imageIndex) = 0;
    void draw(fge::RenderStates& states, fge::vulkan::GraphicPipeline* graphicPipeline = nullptr) const;
    virtual void endRenderPass() = 0;
    virtual void display(uint32_t imageIndex) = 0;

    virtual Vector2u getSize() const = 0;

    [[nodiscard]] virtual VkExtent2D getExtent2D() const = 0;
    [[nodiscard]] virtual fge::vulkan::CommandBuffer& getCommandBuffer() const = 0;
    [[nodiscard]] virtual VkRenderPass getRenderPass() const = 0;

    enum class RequestResults
    {
        ALREADY_INITIALIZED,
        UNINITIALIZED
    };

    [[nodiscard]] std::pair<fge::vulkan::GraphicPipeline&, RequestResults>
    requestGraphicPipeline(vulkan::GraphicPipeline::Key const& key) const;
    void clearGraphicPipelineCache();

    [[nodiscard]] uint32_t requestGlobalTransform(fge::Transformable const& transformable,
                                                  uint32_t parentGlobalTransform) const;
    [[nodiscard]] uint32_t requestGlobalTransform(fge::Transformable const& transformable,
                                                  fge::TransformUboData const& parentTransform) const;
    [[nodiscard]] uint32_t requestGlobalTransform(fge::Transformable const& transformable,
                                                  fge::RenderResourceTransform const& resource) const;
    [[nodiscard]] uint32_t requestGlobalTransform(fge::Transformable const& transformable) const;

    [[nodiscard]] fge::TransformUboData const* getGlobalTransform(fge::RenderResourceTransform const& resource) const;

private:
    View g_defaultView;
    View g_view;

protected:
    void refreshShaderCache();
    void resetDefaultView();

    shader::ShaderManager::DataBlockPointer _g_defaultFragmentShader;
    shader::ShaderManager::DataBlockPointer _g_defaultNoTextureFragmentShader;
    shader::ShaderManager::DataBlockPointer _g_defaultVertexShader;

    VkClearColorValue _g_clearColor;

    bool _g_forceGraphicPipelineUpdate;

    mutable std::unordered_map<vulkan::GraphicPipeline::Key,
                               vulkan::GraphicPipeline,
                               vulkan::GraphicPipeline::Key::Hash,
                               vulkan::GraphicPipeline::Key::Compare>
            _g_graphicPipelineCache;
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_RENDERTARGET_HPP_INCLUDED
