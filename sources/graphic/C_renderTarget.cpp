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

#include "FastEngine/graphic/C_renderTarget.hpp"
#include "FastEngine/C_texture.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/graphic/C_drawable.hpp"
#include "FastEngine/graphic/C_transformable.hpp"
#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_textureImage.hpp"

namespace fge
{

namespace
{

void DefaultGraphicPipelineWithTexture_constructor(fge::vulkan::Context const& context,
                                                   fge::RenderTarget::GraphicPipelineKey const& key,
                                                   fge::vulkan::GraphicPipeline* graphicPipeline)
{
    graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_FRAGMENT)->_shader);
    graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_VERTEX)->_shader);
    graphicPipeline->setBlendMode(key._blendMode);
    graphicPipeline->setPrimitiveTopology(key._topology);

    graphicPipeline->setDescriptorSetLayouts(
            {context.getTransformLayout().getLayout(), context.getTextureLayout().getLayout()});
}
void DefaultGraphicPipeline_constructor(fge::vulkan::Context const& context,
                                        fge::RenderTarget::GraphicPipelineKey const& key,
                                        fge::vulkan::GraphicPipeline* graphicPipeline)
{
    graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT)->_shader);
    graphicPipeline->setShader(fge::shader::GetShader(FGE_SHADER_DEFAULT_VERTEX)->_shader);
    graphicPipeline->setBlendMode(key._blendMode);
    graphicPipeline->setPrimitiveTopology(key._topology);

    graphicPipeline->setDescriptorSetLayouts({context.getTransformLayout().getLayout()});
}

} // end namespace

RenderTarget::RenderTarget(fge::vulkan::Context const& context) :
        fge::vulkan::ContextAware(context),
        _g_clearColor(fge::Color::White),
        _g_forceGraphicPipelineUpdate(false)
{}

void RenderTarget::initialize()
{
    this->resetDefaultView();
    this->g_view = this->g_defaultView;
}

RenderTarget::RenderTarget(RenderTarget const& r) :
        fge::vulkan::ContextAware(r),
        g_defaultView(r.g_defaultView),
        g_view(r.g_view),
        _g_clearColor(r._g_clearColor),
        _g_forceGraphicPipelineUpdate(r._g_forceGraphicPipelineUpdate)
{}
RenderTarget::RenderTarget(RenderTarget&& r) noexcept :
        fge::vulkan::ContextAware(static_cast<fge::vulkan::ContextAware&&>(r)),
        g_defaultView(r.g_defaultView),
        g_view(r.g_view),
        _g_clearColor(r._g_clearColor),
        _g_forceGraphicPipelineUpdate(r._g_forceGraphicPipelineUpdate),
        _g_graphicPipelineCache(std::move(r._g_graphicPipelineCache))
{}

RenderTarget& RenderTarget::operator=(RenderTarget const& r)
{
    this->verifyContext(r);
    this->g_defaultView = r.g_defaultView;
    this->g_view = r.g_view;
    this->_g_clearColor = r._g_clearColor;
    this->_g_forceGraphicPipelineUpdate = r._g_forceGraphicPipelineUpdate;
    return *this;
}
RenderTarget& RenderTarget::operator=(RenderTarget&& r) noexcept
{
    this->verifyContext(r);
    this->g_defaultView = r.g_defaultView;
    this->g_view = r.g_view;
    this->_g_clearColor = r._g_clearColor;
    this->_g_forceGraphicPipelineUpdate = r._g_forceGraphicPipelineUpdate;
    this->_g_graphicPipelineCache = std::move(r._g_graphicPipelineCache);
    return *this;
}

void RenderTarget::setClearColor(fge::Color const& color)
{
    this->_g_clearColor = color;
}
fge::Color RenderTarget::getClearColor() const
{
    return fge::Color(this->_g_clearColor);
}

void RenderTarget::setView(View const& view)
{
    this->g_view = view;
}
View const& RenderTarget::getView() const
{
    return this->g_view;
}
View const& RenderTarget::getDefaultView() const
{
    return this->g_defaultView;
}
fge::vulkan::Viewport RenderTarget::getViewport(View const& view) const
{
    auto const size = static_cast<Vector2f>(this->getSize());
    auto const& factorViewport = view.getFactorViewport();

    return {size.x * factorViewport._x, size.y * factorViewport._y, size.x * factorViewport._width,
            size.y * factorViewport._height};
}

Vector2f RenderTarget::mapFramebufferCoordsToViewSpace(Vector2i const& point) const
{
    return this->mapFramebufferCoordsToViewSpace(point, this->getView());
}
Vector2f RenderTarget::mapFramebufferCoordsToViewSpace(Vector2i const& point, View const& view) const
{
    // Transform the point to normalized device coordinates (assuming it's already in homogeneous coordinates)
    glm::vec4 normalized;
    auto const viewport = this->getViewport(view);
    normalized.x = -1.f + 2.f * (static_cast<float>(point.x) - viewport.getPositionX()) / viewport.getWidth();
    normalized.y = 1.f - 2.f * (static_cast<float>(point.y) - viewport.getPositionY()) / viewport.getHeight();
    normalized.z = 0.0f;
    normalized.w = 1.0f;

    // Transform the homogeneous coordinates to world coordinates
    return view.getInverseProjectionMatrix() * normalized;
}
Vector2f RenderTarget::mapFramebufferCoordsToWorldSpace(Vector2i const& point) const
{
    return this->mapFramebufferCoordsToWorldSpace(point, this->getView());
}
Vector2f RenderTarget::mapFramebufferCoordsToWorldSpace(Vector2i const& point, View const& view) const
{
    return view.getInverseTransform() * this->mapFramebufferCoordsToViewSpace(point, view);
}
Vector2i RenderTarget::mapViewCoordsToFramebufferSpace(Vector2f const& point) const
{
    return this->mapViewCoordsToFramebufferSpace(point, this->getView());
}
Vector2i RenderTarget::mapViewCoordsToFramebufferSpace(Vector2f const& point, View const& view) const
{
    glm::vec4 const pointVec4(point, 0.0f, 1.0f);

    // Transform the point to clip space (assuming it's already NDC)
    glm::vec4 ndc = view.getProjectionMatrix() * pointVec4;

    // Transform the NDC to framebuffer space with the viewport
    ndc.x = (ndc.x + 1.0f) / 2.0f;
    ndc.y = (-ndc.y + 1.0f) / 2.0f;

    auto const viewport = this->getViewport(view);

    Vector2i const pixel{static_cast<int>(ndc.x * viewport.getWidth() + viewport.getPositionX()),
                         static_cast<int>(ndc.y * viewport.getHeight() + viewport.getPositionY())};

    return pixel;
}
Vector2i RenderTarget::mapWorldCoordsToFramebufferSpace(Vector2f const& point) const
{
    return this->mapWorldCoordsToFramebufferSpace(point, this->getView());
}
Vector2i RenderTarget::mapWorldCoordsToFramebufferSpace(Vector2f const& point, View const& view) const
{
    return this->mapViewCoordsToFramebufferSpace(view.getTransform() * point, view);
}

RectFloat RenderTarget::mapFramebufferRectToViewSpace(RectInt const& rect) const
{
    return this->mapFramebufferRectToViewSpace(rect, this->getView());
}
RectFloat RenderTarget::mapFramebufferRectToViewSpace(RectInt const& rect, View const& view) const
{
    Vector2f const positions[4] = {
            this->mapFramebufferCoordsToViewSpace(Vector2i{rect._x, rect._y}, view),
            this->mapFramebufferCoordsToViewSpace(Vector2i{rect._x + rect._width, rect._y}, view),
            this->mapFramebufferCoordsToViewSpace(Vector2i{rect._x, rect._y + rect._height}, view),
            this->mapFramebufferCoordsToViewSpace(Vector2i{rect._x + rect._width, rect._y + rect._height}, view)};

    return ToRect(positions, 4);
}
RectFloat RenderTarget::mapFramebufferRectToWorldSpace(RectInt const& rect) const
{
    return this->mapFramebufferRectToWorldSpace(rect, this->getView());
}
RectFloat RenderTarget::mapFramebufferRectToWorldSpace(RectInt const& rect, View const& view) const
{
    Vector2f const positions[4] = {
            this->mapFramebufferCoordsToWorldSpace(Vector2i{rect._x, rect._y}, view),
            this->mapFramebufferCoordsToWorldSpace(Vector2i{rect._x + rect._width, rect._y}, view),
            this->mapFramebufferCoordsToWorldSpace(Vector2i{rect._x, rect._y + rect._height}, view),
            this->mapFramebufferCoordsToWorldSpace(Vector2i{rect._x + rect._width, rect._y + rect._height}, view)};

    return ToRect(positions, 4);
}

RectInt RenderTarget::mapViewRectToFramebufferSpace(RectFloat const& rect) const
{
    return this->mapViewRectToFramebufferSpace(rect, this->getView());
}
RectInt RenderTarget::mapViewRectToFramebufferSpace(RectFloat const& rect, View const& view) const
{
    Vector2i const positions[4] = {
            this->mapViewCoordsToFramebufferSpace(Vector2f{rect._x, rect._y}, view),
            this->mapViewCoordsToFramebufferSpace(Vector2f{rect._x + rect._width, rect._y}, view),
            this->mapViewCoordsToFramebufferSpace(Vector2f{rect._x, rect._y + rect._height}, view),
            this->mapViewCoordsToFramebufferSpace(Vector2f{rect._x + rect._width, rect._y + rect._height}, view)};

    return ToRect(positions, 4);
}
RectInt RenderTarget::mapWorldRectToFramebufferSpace(RectFloat const& rect) const
{
    return this->mapWorldRectToFramebufferSpace(rect, this->getView());
}
RectInt RenderTarget::mapWorldRectToFramebufferSpace(RectFloat const& rect, View const& view) const
{
    Vector2i const positions[4] = {
            this->mapWorldCoordsToFramebufferSpace(Vector2f{rect._x, rect._y}, view),
            this->mapWorldCoordsToFramebufferSpace(Vector2f{rect._x + rect._width, rect._y}, view),
            this->mapWorldCoordsToFramebufferSpace(Vector2f{rect._x, rect._y + rect._height}, view),
            this->mapWorldCoordsToFramebufferSpace(Vector2f{rect._x + rect._width, rect._y + rect._height}, view)};

    return ToRect(positions, 4);
}

void RenderTarget::draw(Drawable const& drawable, RenderStates const& states)
{
    drawable.draw(*this, states); ///TODO: Inline that
}
void RenderTarget::draw(fge::RenderStates const& states, fge::vulkan::GraphicPipeline const* graphicPipeline) const
{
    bool const haveTextures = states._resTextures.getCount() != 0;

    //See if we can set a default graphicPipeline for this rendering call
    if (graphicPipeline == nullptr)
    {
        if (states._resInstances.getInstancesCount() == 1 && states._resTransform.get() != nullptr &&
            states._resDescriptors.getCount() == 0 && states._vertexBuffer != nullptr)
        { //Simple rendering: There must be 1 instance, a transform, no descriptors
            if (states._resTextures.getCount() == 1 || states._resTextures.getCount() == 0)
            { //1 or no texture
                GraphicPipelineKey const graphicPipelineKey{
                        states._vertexBuffer->getPrimitiveTopology(), states._blendMode,
                        uint8_t(haveTextures ? FGE_RENDERTARGET_DEFAULT_ID_TEXTURE : FGE_RENDERTARGET_DEFAULT_ID)};

                graphicPipeline =
                        this->getGraphicPipeline(FGE_RENDERTARGET_DEFAULT_PIPELINE_CACHE_NAME, graphicPipelineKey,
                                                 haveTextures ? DefaultGraphicPipelineWithTexture_constructor
                                                              : DefaultGraphicPipeline_constructor);
            }
        }
    }

    //If we still don't have any graphicPipeline
    if (graphicPipeline == nullptr)
    {
        return; ///TODO: error handling
    }

    //Apply view transform
    if (states._resTransform.get() != nullptr)
    {
        states._resTransform.get()->getData()._viewTransform =
                this->getView().getProjectionMatrix() * this->getView().getTransform();
    }

    //Updating graphicPipeline
    auto const viewport = this->getViewport(this->getView());

    graphicPipeline->setScissor({{0, 0}, this->getExtent2D()});

    graphicPipeline->updateIfNeeded(this->getRenderPass(), this->_g_forceGraphicPipelineUpdate);

    auto& commandBuffer = this->getCommandBuffer();

    if (states._resDescriptors.getCount() > 0)
    {
        ///TODO: test that
        for (uint32_t i = 0; i < states._resDescriptors.getCount(); ++i)
        {
            auto descriptor = states._resDescriptors.getDescriptorSet(i)->get();
            commandBuffer.bindDescriptorSets(graphicPipeline->getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                             &descriptor, 1, states._resDescriptors.getSet(i));
        }
    }

    // Apply "global" textures
#ifndef FGE_DEF_SERVER
    if (haveTextures)
    {
        if (states._resInstances.getTextureIndices() == nullptr)
        { //Instances don't have any texture index
            if (states._resTextures.getCount() > 1)
            { //Instance have multiple textures
                ///TODO: test that
                for (uint32_t i = 0; i < states._resTextures.getCount(); ++i)
                {
                    fge::vulkan::TextureImage const* textureImage = nullptr;
                    switch (states._resTextures.getPointerType())
                    {
                    case RenderResourceTextures::PtrTypes::TEXTURE:
                        textureImage =
                                states._resTextures.getTextureImage<RenderResourceTextures::PtrTypes::TEXTURE>(i);
                        break;
                    case RenderResourceTextures::PtrTypes::TEXTURE_IMAGE:
                        textureImage =
                                states._resTextures.getTextureImage<RenderResourceTextures::PtrTypes::TEXTURE_IMAGE>(i);
                        break;
                    }
                    auto descriptorSetTexture = textureImage->getDescriptorSet().get();
                    commandBuffer.bindDescriptorSets(graphicPipeline->getPipelineLayout(),
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS, &descriptorSetTexture, 1,
                                                     FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TEXTURE + i);
                }
            }
            else
            { //Instance have 1
                fge::vulkan::TextureImage const* textureImage = nullptr;
                switch (states._resTextures.getPointerType())
                {
                case RenderResourceTextures::PtrTypes::TEXTURE:
                    textureImage = states._resTextures.getTextureImage<RenderResourceTextures::PtrTypes::TEXTURE>(0);
                    break;
                case RenderResourceTextures::PtrTypes::TEXTURE_IMAGE:
                    textureImage =
                            states._resTextures.getTextureImage<RenderResourceTextures::PtrTypes::TEXTURE_IMAGE>(0);
                    break;
                }

                auto descriptorSetTexture = textureImage->getDescriptorSet().get();
                commandBuffer.bindDescriptorSets(graphicPipeline->getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 &descriptorSetTexture, 1,
                                                 FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TEXTURE);
            }
        }
    }
#endif //FGE_DEF_SERVER

    graphicPipeline->recordCommandBuffer(commandBuffer, viewport, states._vertexBuffer, states._indexBuffer);

    //Binding default transform
    if (states._resTransform.get() != nullptr)
    {
        auto descriptorSetTransform = states._resTransform.get()->getDescriptorSet().get();
        commandBuffer.bindDescriptorSets(graphicPipeline->getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                         &descriptorSetTransform, 1, FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TRANSFORM);
    }

    //Check instances
    for (uint32_t iInstance = 0; iInstance < states._resInstances.getInstancesCount(); ++iInstance)
    {
#ifndef FGE_DEF_SERVER
        if (haveTextures)
        {
            if (states._resInstances.getTextureIndices() != nullptr)
            { //Instance have a texture index
                fge::vulkan::TextureImage const* textureImage = nullptr;
                uint32_t const textureIndex = states._resInstances.getTextureIndices(iInstance);
                if (textureIndex < states._resTextures.getCount())
                {
                    switch (states._resTextures.getPointerType())
                    {
                    case RenderResourceTextures::PtrTypes::TEXTURE:
                        textureImage = states._resTextures.getTextureImage<RenderResourceTextures::PtrTypes::TEXTURE>(
                                textureIndex);
                        break;
                    case RenderResourceTextures::PtrTypes::TEXTURE_IMAGE:
                        textureImage =
                                states._resTextures.getTextureImage<RenderResourceTextures::PtrTypes::TEXTURE_IMAGE>(
                                        textureIndex);
                        break;
                    }
                }
                else
                {
                    textureImage = fge::texture::GetBadTexture()->_texture.get();
                }

                auto descriptorSet = textureImage->getDescriptorSet().get();
                commandBuffer.bindDescriptorSets(graphicPipeline->getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 &descriptorSet, 1, FGE_RENDERTARGET_DEFAULT_DESCRIPTOR_SET_TEXTURE);
            }
        }
#endif //FGE_DEF_SERVER

        //Binding dynamic descriptors
        for (uint32_t i = 0; i < states._resInstances.getDynamicCount(); ++i)
        {
            uint32_t const dynamicOffset = states._resInstances.getDynamicBufferSizes(i) * iInstance +
                                           states._resInstances.getDynamicBufferOffsets(i);
            auto descriptorSet = states._resInstances.getDynamicDescriptors(i)->get();
            commandBuffer.bindDescriptorSets(graphicPipeline->getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                             &descriptorSet, 1, 1, &dynamicOffset, 0);
        }

        uint32_t const vertexCount = states._resInstances.getVertexCount() == 0 ? states._vertexBuffer->getCount()
                                                                                : states._resInstances.getVertexCount();
        uint32_t const vertexOffset = (states._resInstances.getVertexCount() == 0 ? 0 : vertexCount * iInstance) +
                                      states._resInstances.getVertexOffset();

        if (states._resInstances.hasUniqueDrawCall())
        {
            if (states._resInstances.getIndirectBuffer() != VK_NULL_HANDLE)
            { //Indirect draw
                commandBuffer.drawIndirect(states._resInstances.getIndirectBuffer(), 0,
                                           states._resInstances.getInstancesCount(), sizeof(VkDrawIndirectCommand));
            }
            else
            {
                commandBuffer.draw(vertexCount, states._resInstances.getInstancesCount(), vertexOffset,
                                   states._resInstances.getFirstInstance());
            }
            break;
        }
        commandBuffer.draw(vertexCount, 1, vertexOffset, iInstance);
    }
}

fge::vulkan::GraphicPipeline* RenderTarget::getGraphicPipeline(std::string_view name,
                                                               GraphicPipelineKey const& key,
                                                               GraphicPipelineConstructor constructor) const
{
    fge::vulkan::GraphicPipeline* graphicPipeline = nullptr;

    auto itName = this->_g_graphicPipelineCache.find(name);
    if (itName == this->_g_graphicPipelineCache.end())
    {
        itName = this->_g_graphicPipelineCache.insert({std::string(name), {}}).first;
    }

    auto itPipeline = itName->second.find(key);
    if (itPipeline != itName->second.end())
    {
        graphicPipeline = &itPipeline->second;
    }
    else
    {
        graphicPipeline = &itName->second.emplace(key, fge::vulkan::GraphicPipeline{this->getContext()}).first->second;

        if (constructor != nullptr)
        {
            constructor(this->getContext(), key, graphicPipeline);
        }
    }

    return graphicPipeline;
}
void RenderTarget::clearGraphicPipelineCache()
{
    this->_g_graphicPipelineCache.clear();
}

void RenderTarget::resetDefaultView()
{
    this->g_defaultView.reset(
            {0.0f, 0.0f, static_cast<float>(this->getSize().x), static_cast<float>(this->getSize().y)});
}

} // namespace fge
