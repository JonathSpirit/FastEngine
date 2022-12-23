#ifndef _FGE_VULKAN_C_RENDERTARGET_HPP_INCLUDED
#define _FGE_VULKAN_C_RENDERTARGET_HPP_INCLUDED

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

class RenderTarget
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
    virtual void draw(fge::vulkan::GraphicPipeline& graphicPipeline, const RenderStates& states) = 0;
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
