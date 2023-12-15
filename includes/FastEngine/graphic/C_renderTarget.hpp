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

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "FastEngine/graphic/C_renderStates.hpp"
#include "FastEngine/graphic/C_view.hpp"
#include "FastEngine/vulkan/C_blendMode.hpp"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_graphicPipeline.hpp"
#include "FastEngine/vulkan/C_vertex.hpp"
#include "SDL_video.h"
#include <map>
#include <unordered_map>

#define FGE_RENDERTARGET_BAD_IMAGE_INDEX std::numeric_limits<uint32_t>::max()
#define FGE_RENDERTARGET_DEFAULT_PIPELINE_CACHE_NAME ""
#define FGE_RENDERTARGET_DEFAULT_ID 0
#define FGE_RENDERTARGET_DEFAULT_ID_TEXTURE 1

#define FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TRANSFORM 0
#define FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TEXTURE 1

namespace fge
{

class Texture;
class Drawable;

class FGE_API RenderTarget : public fge::vulkan::ContextAware
{
protected:
    explicit RenderTarget(fge::vulkan::Context const& context);

    void initialize();

public:
    struct GraphicPipelineKey
    {
        [[nodiscard]] inline std::size_t operator()(GraphicPipelineKey const& k) const
        {
            uint64_t const val = (static_cast<uint64_t>(k._topology) << 38) | (static_cast<uint64_t>(k._id) << 30) |
                                 (static_cast<uint64_t>(k._blendMode._srcColorBlendFactor) << 25) |
                                 (static_cast<uint64_t>(k._blendMode._dstColorBlendFactor) << 20) |
                                 (static_cast<uint64_t>(k._blendMode._colorBlendOp) << 15) |
                                 (static_cast<uint64_t>(k._blendMode._srcAlphaBlendFactor) << 10) |
                                 (static_cast<uint64_t>(k._blendMode._dstAlphaBlendFactor) << 5) |
                                 (static_cast<uint64_t>(k._blendMode._alphaBlendOp));
            return std::hash<uint64_t>{}(val);
        }
        [[nodiscard]] inline bool operator==(GraphicPipelineKey const& k) const
        {
            return this->_topology == k._topology && this->_blendMode == k._blendMode && this->_id == k._id;
        }

        VkPrimitiveTopology _topology;
        fge::vulkan::BlendMode _blendMode;
        uint8_t _id{0};
    };

    using GraphicPipelineCache =
            std::map<std::string,
                     std::unordered_map<GraphicPipelineKey, fge::vulkan::GraphicPipeline, GraphicPipelineKey>,
                     std::less<>>;
    using GraphicPipelineConstructor = void (*)(fge::vulkan::Context const&,
                                                GraphicPipelineKey const&,
                                                fge::vulkan::GraphicPipeline*);

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

    [[nodiscard]] Vector2f mapPixelToCoords(Vector2i const& point) const;
    [[nodiscard]] Vector2f mapPixelToCoords(Vector2i const& point, View const& view) const;
    [[nodiscard]] Vector2i mapCoordsToPixel(Vector2f const& point) const;
    [[nodiscard]] Vector2i mapCoordsToPixel(Vector2f const& point, View const& view) const;

    virtual uint32_t prepareNextFrame(VkCommandBufferInheritanceInfo const* inheritanceInfo) = 0;
    virtual void beginRenderPass(uint32_t imageIndex) = 0;
    void draw(fge::Drawable const& drawable, fge::RenderStates const& states);
    void draw(fge::RenderStates const& states, fge::vulkan::GraphicPipeline const* graphicPipeline = nullptr);
    virtual void endRenderPass() = 0;
    virtual void display(uint32_t imageIndex) = 0;

    virtual Vector2u getSize() const = 0;

    virtual bool isSrgb() const;

    [[nodiscard]] virtual VkExtent2D getExtent2D() const = 0;
    [[nodiscard]] virtual VkCommandBuffer getCommandBuffer() const = 0;
    [[nodiscard]] virtual VkRenderPass getRenderPass() const = 0;

    [[nodiscard]] fge::vulkan::GraphicPipeline* getGraphicPipeline(std::string_view name,
                                                                   GraphicPipelineKey const& key,
                                                                   GraphicPipelineConstructor constructor) const;
    void clearGraphicPipelineCache();

private:
    View g_defaultView;
    View g_view;

protected:
    VkClearColorValue _g_clearColor;

    bool _g_forceGraphicPipelineUpdate;

    mutable GraphicPipelineCache _g_graphicPipelineCache;
};

} // namespace fge

#endif // _FGE_GRAPHIC_C_RENDERTARGET_HPP_INCLUDED
