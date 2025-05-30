/*
 * Copyright 2025 Guillaume Guillet
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
#include "FastEngine/accessor/C_texture.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/graphic/C_drawable.hpp"
#include "FastEngine/graphic/C_transform.hpp"
#include "FastEngine/graphic/C_transformable.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_textureImage.hpp"

namespace fge
{

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
    return view.getInverseProjection() * normalized;
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
    glm::vec4 ndc = view.getProjection() * pointVec4;

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

void RenderTarget::draw(fge::RenderStates& states, fge::vulkan::GraphicPipeline* graphicPipeline) const
{
    bool const haveTextures = states._resTextures.getCount() != 0;

    //Global transforms
    auto const globalTransformsIndex = states._resTransform.getGlobalTransformsIndex();
    uint32_t firstInstance{0};
    switch (states._resTransform.getConfig())
    {
    case RenderResourceTransform::Configs::GLOBAL_TRANSFORMS_INDEX_IS_ADDED_TO_FIRST_INSTANCE:
        firstInstance = states._resInstances.getFirstInstance() + globalTransformsIndex.value_or(0);
        break;
    case RenderResourceTransform::Configs::GLOBAL_TRANSFORMS_INDEX_OVERWRITE_FIRST_INSTANCE:
        firstInstance = globalTransformsIndex.value_or(states._resInstances.getFirstInstance());
        break;
    case RenderResourceTransform::Configs::GLOBAL_TRANSFORMS_INDEX_IS_IGNORED:
        firstInstance = states._resInstances.getFirstInstance();
        break;
    }

    //See if we can set default shaders for this rendering call
    if (states._shaderFragment == nullptr && states._shaderGeometry == nullptr && states._shaderVertex == nullptr &&
        graphicPipeline == nullptr)
    {
        if (states._resInstances.getInstancesCount() == 1 && globalTransformsIndex &&
            states._resDescriptors.getCount() == 0 && states._vertexBuffer != nullptr &&
            (states._resTextures.getCount() == 1 || states._resTextures.getCount() == 0))
        {
            states._shaderFragment = haveTextures ? this->_g_defaultFragmentShader->_ptr.get()
                                                  : this->_g_defaultNoTextureFragmentShader->_ptr.get();
            states._shaderVertex = this->_g_defaultVertexShader->_ptr.get();
        }
        else
        {
            return; ///TODO: error handling
        }
    }

    if (graphicPipeline == nullptr)
    {
        auto& layoutPipeline = this->getContext().requestLayoutPipeline(states._shaderVertex, states._shaderGeometry,
                                                                        states._shaderFragment);
        layoutPipeline.updateIfNeeded();

        //Set a default graphicPipeline for this rendering call
        vulkan::GraphicPipeline::Key const graphicPipelineKey{
                VK_NULL_HANDLE,
                states._shaderVertex->getShaderModule(),
                states._shaderFragment->getShaderModule(),
                VK_NULL_HANDLE,
                states._vertexBuffer == nullptr ? states._topology : states._vertexBuffer->getPrimitiveTopology(),
                states._blendMode,
                layoutPipeline.get()};

        auto cacheResultGraphicPipeline = this->requestGraphicPipeline(graphicPipelineKey);
        graphicPipeline = &cacheResultGraphicPipeline.first;
        if (cacheResultGraphicPipeline.second == RequestResults::UNINITIALIZED)
        {
            if (states._shaderVertex != nullptr)
            {
                graphicPipeline->setShader(*states._shaderVertex);
            }
            if (states._shaderFragment != nullptr)
            {
                graphicPipeline->setShader(*states._shaderFragment);
            }
            if (states._shaderGeometry != nullptr)
            {
                graphicPipeline->setShader(*states._shaderGeometry);
            }

            graphicPipeline->setBlendMode(states._blendMode);
            graphicPipeline->setPrimitiveTopology(graphicPipelineKey._primitiveTopology);
            graphicPipeline->setPipelineLayout(layoutPipeline);
        }
    }

    //If we still don't have any graphicPipeline
    if (graphicPipeline == nullptr)
    {
        return; ///TODO: error handling
    }

    auto& commandBuffer = this->getCommandBuffer();

    //Push constants
    for (uint32_t i = 0; i < states._resPushConstants.getCount(); ++i)
    {
        auto const* pushConstant = states._resPushConstants.getPushConstants(i);
        commandBuffer.pushConstants(graphicPipeline->getPipelineLayout(), pushConstant->g_stages,
                                    pushConstant->g_offset, pushConstant->g_size, pushConstant->g_data);
    }

    //Apply view transform
    if (globalTransformsIndex)
    {
        auto* transform = this->getContext().getGlobalTransform(globalTransformsIndex.value());
        transform->_viewTransform = this->getView().getProjection() * this->getView().getTransform();
    }

    //Updating graphicPipeline
    auto const viewport = this->getViewport(this->getView());

    graphicPipeline->updateIfNeeded(this->getRenderPass(), this->_g_forceGraphicPipelineUpdate);

    if (states._resDescriptors.getCount() > 0)
    {
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
                                                     FGE_RENDER_DEFAULT_DESCRIPTOR_SET_TEXTURE + i);
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
                                                 &descriptorSetTexture, 1, FGE_RENDER_DEFAULT_DESCRIPTOR_SET_TEXTURE);
            }
        }
    }
#endif //FGE_DEF_SERVER

    graphicPipeline->recordCommandBuffer(commandBuffer, viewport, {{0, 0}, this->getExtent2D()}, states._vertexBuffer,
                                         states._indexBuffer);

    //Binding global transforms
    if (globalTransformsIndex)
    {
        auto descriptorSetTransform = this->getContext().getGlobalTransform()._descriptorSet.get();
        commandBuffer.bindDescriptorSets(graphicPipeline->getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                         &descriptorSetTransform, 1, FGE_RENDER_DEFAULT_DESCRIPTOR_SET_TRANSFORM);
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
                    textureImage = fge::texture::gManager.getBadElement()->_ptr.get();
                }

                auto descriptorSet = textureImage->getDescriptorSet().get();
                commandBuffer.bindDescriptorSets(graphicPipeline->getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 &descriptorSet, 1, FGE_RENDER_DEFAULT_DESCRIPTOR_SET_TEXTURE);
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
                commandBuffer.draw(vertexCount, states._resInstances.getInstancesCount(), vertexOffset, firstInstance);
            }
            break;
        }
        commandBuffer.draw(vertexCount, 1, vertexOffset, firstInstance + iInstance);
    }
}

std::pair<fge::vulkan::GraphicPipeline&, RenderTarget::RequestResults>
RenderTarget::requestGraphicPipeline(vulkan::GraphicPipeline::Key const& key) const
{
    auto it = this->_g_graphicPipelineCache.find(key);
    if (it != this->_g_graphicPipelineCache.end())
    {
        return {it->second, RequestResults::ALREADY_INITIALIZED};
    }

    auto& graphicPipeline = this->_g_graphicPipelineCache
                                    .emplace(std::piecewise_construct, std::forward_as_tuple(key),
                                             std::forward_as_tuple(this->getContext()))
                                    .first->second;

    return {graphicPipeline, RequestResults::UNINITIALIZED};
}
void RenderTarget::clearGraphicPipelineCache()
{
    this->_g_graphicPipelineCache.clear();
}

uint32_t RenderTarget::requestGlobalTransform(fge::Transformable const& transformable,
                                              uint32_t parentGlobalTransform) const
{
    auto transform = this->getContext().requestGlobalTransform();

    auto const* parentTransform = this->getContext().getGlobalTransform(parentGlobalTransform);
    if (parentTransform != nullptr)
    {
        transform.second->_modelTransform = parentTransform->_modelTransform * transformable.getTransform();
    }
    else
    {
        transform.second->_modelTransform = transformable.getTransform();
    }
    return transform.first;
}
uint32_t RenderTarget::requestGlobalTransform(fge::Transformable const& transformable,
                                              fge::TransformUboData const& parentTransform) const
{
    auto transform = this->getContext().requestGlobalTransform();
    transform.second->_modelTransform = parentTransform._modelTransform * transformable.getTransform();
    return transform.first;
}
uint32_t RenderTarget::requestGlobalTransform(fge::Transformable const& transformable,
                                              fge::RenderResourceTransform const& resource) const
{
    if (resource.getTransformData() != nullptr)
    {
        return this->requestGlobalTransform(transformable, *resource.getTransformData());
    }
    if (auto const index = resource.getGlobalTransformsIndex())
    {
        return this->requestGlobalTransform(transformable, *index);
    }
    return this->requestGlobalTransform(transformable);
}
uint32_t RenderTarget::requestGlobalTransform(fge::Transformable const& transformable) const
{
    auto transform = this->getContext().requestGlobalTransform();
    transform.second->_modelTransform = transformable.getTransform();
    return transform.first;
}

fge::TransformUboData const* RenderTarget::getGlobalTransform(fge::RenderResourceTransform const& resource) const
{
    if (auto const index = resource.getGlobalTransformsIndex())
    {
        return this->getContext().getGlobalTransform(*index);
    }
    return resource.getTransformData();
}

void RenderTarget::refreshShaderCache()
{
    this->_g_defaultFragmentShader = shader::gManager.getElement(FGE_SHADER_DEFAULT_FRAGMENT);
    this->_g_defaultNoTextureFragmentShader = shader::gManager.getElement(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT);
    this->_g_defaultVertexShader = shader::gManager.getElement(FGE_SHADER_DEFAULT_VERTEX);
}
void RenderTarget::resetDefaultView()
{
    this->g_defaultView.reset(
            {0.0f, 0.0f, static_cast<float>(this->getSize().x), static_cast<float>(this->getSize().y)});
}

} // namespace fge
