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
#include <FastEngine/graphic/C_transformable.hpp>
#include <FastEngine/vulkan/C_textureImage.hpp>
#include <FastEngine/vulkan/C_context.hpp>
#include <FastEngine/manager/shader_manager.hpp>

namespace fge
{

const fge::vulkan::TextureImage* RenderTarget::gLastTexture = nullptr;

RenderTarget::RenderTarget(const fge::vulkan::Context& context) :
        _g_clearColor(fge::Color::White),
        _g_context(&context),
        _g_forceGraphicPipelineUpdate(false)
{}

void RenderTarget::initialize()
{
    this->g_defaultView.reset({0.0f, 0.0f,
                               static_cast<float>(this->getSize().x),
                               static_cast<float>(this->getSize().y)});
    this->g_view = this->g_defaultView;
}

RenderTarget::RenderTarget(const RenderTarget& r) :
        g_defaultView(r.g_defaultView),
        g_view(r.g_view),
        _g_clearColor(r._g_clearColor),
        _g_context(r._g_context),
        _g_forceGraphicPipelineUpdate(r._g_forceGraphicPipelineUpdate)
{}
RenderTarget::RenderTarget(RenderTarget&& r) noexcept :
        g_defaultView(r.g_defaultView),
        g_view(r.g_view),
        _g_clearColor(r._g_clearColor),
        _g_context(r._g_context),
        _g_forceGraphicPipelineUpdate(r._g_forceGraphicPipelineUpdate),
        _g_defaultGraphicPipelineTexture(std::move(r._g_defaultGraphicPipelineTexture)),
        _g_defaultGraphicPipelineNoTexture(std::move(r._g_defaultGraphicPipelineNoTexture)),
        _g_defaultGraphicPipelineTextureBatches(std::move(r._g_defaultGraphicPipelineTextureBatches)),
        _g_defaultGraphicPipelineNoTextureBatches(std::move(r._g_defaultGraphicPipelineNoTextureBatches))
{}

RenderTarget& RenderTarget::operator=(const RenderTarget& r)
{
    this->g_defaultView = r.g_defaultView;
    this->g_view = r.g_view;
    this->_g_clearColor = r._g_clearColor;
    this->_g_context = r._g_context;
    this->_g_forceGraphicPipelineUpdate = r._g_forceGraphicPipelineUpdate;
    return *this;
}
RenderTarget& RenderTarget::operator=(RenderTarget&& r) noexcept
{
    this->g_defaultView = r.g_defaultView;
    this->g_view = r.g_view;
    this->_g_clearColor = r._g_clearColor;
    this->_g_context = r._g_context;
    this->_g_forceGraphicPipelineUpdate = r._g_forceGraphicPipelineUpdate;
    this->_g_defaultGraphicPipelineTexture = std::move(r._g_defaultGraphicPipelineTexture);
    this->_g_defaultGraphicPipelineNoTexture = std::move(r._g_defaultGraphicPipelineNoTexture);
    this->_g_defaultGraphicPipelineTextureBatches = std::move(r._g_defaultGraphicPipelineTextureBatches);
    this->_g_defaultGraphicPipelineNoTextureBatches = std::move(r._g_defaultGraphicPipelineNoTextureBatches);
    return *this;
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
    return view.getInverseTransform() * normalized;
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
void RenderTarget::draw(const fge::RenderStates& states)
{
    if (states._textureImage == nullptr)
    {
        fge::vulkan::GraphicPipeline* graphicPipeline = nullptr;

        auto it = this->_g_defaultGraphicPipelineNoTexture.find(states._blendMode);
        if (it == this->_g_defaultGraphicPipelineNoTexture.end())
        {
            graphicPipeline = &this->_g_defaultGraphicPipelineNoTexture[states._blendMode];
            graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT)->_shader);
            graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_VERTEX)->_shader);
            graphicPipeline->setBlendMode(states._blendMode);
        }
        else
        {
            graphicPipeline = &it->second;
        }

        this->draw(*graphicPipeline, states);
    }
    else
    {
        fge::vulkan::GraphicPipeline* graphicPipeline = nullptr;

        auto it = this->_g_defaultGraphicPipelineTexture.find(states._blendMode);
        if (it == this->_g_defaultGraphicPipelineTexture.end())
        {
            graphicPipeline = &this->_g_defaultGraphicPipelineTexture[states._blendMode];
            graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_FRAGMENT)->_shader);
            graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_VERTEX)->_shader);
            graphicPipeline->setBlendMode(states._blendMode);
        }
        else
        {
            graphicPipeline = &it->second;
        }

        this->draw(*graphicPipeline, states);
    }
}
void RenderTarget::draw(const fge::vulkan::GraphicPipeline& graphicPipeline, const fge::RenderStates& states)
{
    states._transform->_data._viewTransform = this->getView().getTransform();
    states._transform->updateUniformBuffer(*this->_g_context);

    auto windowSize = static_cast<fge::Vector2f>(this->getSize());
    auto factorViewport = this->getView().getFactorViewport();

    const fge::vulkan::Viewport viewport(windowSize.x*factorViewport._x, windowSize.y*factorViewport._y,
                                         windowSize.x*factorViewport._width,windowSize.y*factorViewport._height);

    graphicPipeline.setViewport(viewport);
    graphicPipeline.setScissor({{0, 0}, this->getExtent2D()});

    VkDescriptorSetLayout layout[] = {this->_g_context->getTransformLayout().getLayout(),
                                      this->_g_context->getTextureLayout().getLayout()};

    graphicPipeline.updateIfNeeded(*this->_g_context,
                                   layout, 2,
                                   this->getRenderPass(),
                                   this->_g_forceGraphicPipelineUpdate);

    auto commandBuffer = this->getCommandBuffer();

#ifndef FGE_DEF_SERVER
    if (states._textureImage != nullptr)
    {
        if (RenderTarget::gLastTexture != states._textureImage)
        {
            RenderTarget::gLastTexture = states._textureImage;
            VkDescriptorSet descriptorSetTexture[] = {states._textureImage->getDescriptorSet().getDescriptorSet()};
            graphicPipeline.bindDescriptorSets(commandBuffer, descriptorSetTexture, 1, 1);
        }
    }
#endif //FGE_DEF_SERVER

    VkDescriptorSet descriptorSetTransform[] = {states._transform->getDescriptorSet().getDescriptorSet()};
    graphicPipeline.bindDescriptorSets(commandBuffer, descriptorSetTransform, 1, 0);

    graphicPipeline.recordCommandBuffer(commandBuffer, states._vertexBuffer, states._indexBuffer);
}

void RenderTarget::drawBatches(const fge::vulkan::BlendMode& blendMode,
                               const fge::vulkan::TextureImage* textureImage,
                               const fge::vulkan::DescriptorSet& transformDescriptorSet,
                               const fge::vulkan::VertexBuffer* vertexBuffer,
                               uint32_t vertexCount,
                               uint32_t instanceCount)
{
    if (textureImage == nullptr)
    {
        fge::vulkan::GraphicPipeline* graphicPipeline = nullptr;

        auto it = this->_g_defaultGraphicPipelineNoTextureBatches.find(blendMode);
        if (it == this->_g_defaultGraphicPipelineNoTextureBatches.end())
        {
            graphicPipeline = &this->_g_defaultGraphicPipelineNoTextureBatches[blendMode];
            graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT)->_shader);
            graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_VERTEX)->_shader);
            graphicPipeline->setBlendMode(blendMode);
        }
        else
        {
            graphicPipeline = &it->second;
        }

        this->drawBatches(*graphicPipeline,
                          textureImage,
                          transformDescriptorSet,
                          vertexBuffer,
                          vertexCount,
                          instanceCount);
    }
    else
    {
        fge::vulkan::GraphicPipeline* graphicPipeline = nullptr;

        auto it = this->_g_defaultGraphicPipelineTextureBatches.find(blendMode);
        if (it == this->_g_defaultGraphicPipelineTextureBatches.end())
        {
            graphicPipeline = &this->_g_defaultGraphicPipelineTextureBatches[blendMode];
            graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_FRAGMENT)->_shader);
            graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_VERTEX)->_shader);
            graphicPipeline->setBlendMode(blendMode);
        }
        else
        {
            graphicPipeline = &it->second;
        }

        this->drawBatches(*graphicPipeline,
                          textureImage,
                          transformDescriptorSet,
                          vertexBuffer,
                          vertexCount,
                          instanceCount);
    }
}
void RenderTarget::drawBatches(const fge::vulkan::GraphicPipeline& graphicPipeline,
                               const fge::vulkan::TextureImage* textureImage,
                               const fge::vulkan::DescriptorSet& transformDescriptorSet,
                               const fge::vulkan::VertexBuffer* vertexBuffer,
                               uint32_t vertexCount,
                               uint32_t instanceCount)
{
    auto windowSize = static_cast<fge::Vector2f>(this->getSize());
    auto factorViewport = this->getView().getFactorViewport();

    const fge::vulkan::Viewport viewport(windowSize.x*factorViewport._x, windowSize.y*factorViewport._y,
                                         windowSize.x*factorViewport._width,windowSize.y*factorViewport._height);

    graphicPipeline.setViewport(viewport);
    graphicPipeline.setScissor({{0, 0}, this->getExtent2D()});

    VkDescriptorSetLayout layout[] = {this->_g_context->getTransformBatchesLayout().getLayout(),
                                      this->_g_context->getTextureLayout().getLayout()};

    graphicPipeline.updateIfNeeded(*this->_g_context,
                                   layout, 2,
                                   this->getRenderPass(),
                                   this->_g_forceGraphicPipelineUpdate);

    auto commandBuffer = this->getCommandBuffer();

    if (textureImage != nullptr)
    {
        if (RenderTarget::gLastTexture != textureImage)
        {
            RenderTarget::gLastTexture = textureImage;
            VkDescriptorSet descriptorSetTexture[] = {textureImage->getDescriptorSet().getDescriptorSet()};
            graphicPipeline.bindDescriptorSets(commandBuffer, descriptorSetTexture, 1, 1);
        }
    }

    VkDescriptorSet descriptorSetTransform[] = {transformDescriptorSet.getDescriptorSet()};

    graphicPipeline.recordCommandBufferWithoutDraw(commandBuffer,
                                                   vertexBuffer,
                                                   nullptr);

    for (std::size_t i=0; i<instanceCount; ++i)
    {
        const uint32_t dynamicOffset = fge::TransformUboData::uboSize * i;

        graphicPipeline.bindDynamicDescriptorSets(commandBuffer,
                                                  descriptorSetTransform,
                                                  1,
                                                  1,
                                                  &dynamicOffset,
                                                  0);

        vkCmdDraw(commandBuffer, vertexCount, 1, vertexCount*i, 0);
    }
}

void RenderTarget::pushExtraCommandBuffer([[maybe_unused]] VkCommandBuffer commandBuffer) const
{}
void RenderTarget::pushExtraCommandBuffer([[maybe_unused]] const std::vector<VkCommandBuffer>& commandBuffers) const
{}

const fge::vulkan::Context* RenderTarget::getContext() const
{
    return this->_g_context;
}

bool RenderTarget::isSrgb() const
{
    return false;
}

}//end fge
