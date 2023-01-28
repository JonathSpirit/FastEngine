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

#ifndef _FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
#define _FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"
#include "C_blendMode.hpp"
#include "C_shader.hpp"
#include "C_vertex.hpp"
#include "C_vertexBuffer.hpp"
#include "C_viewport.hpp"
#include "vulkanGlobal.hpp"

namespace fge::vulkan
{

class SwapChain;
class LogicalDevice;

class FGE_API GraphicPipeline
{
public:
    GraphicPipeline();
    GraphicPipeline(const GraphicPipeline& r);
    GraphicPipeline(GraphicPipeline&& r) noexcept;
    ~GraphicPipeline();

    GraphicPipeline& operator=(const GraphicPipeline& r) = delete;
    GraphicPipeline& operator=(GraphicPipeline&& r) noexcept = delete;

    bool updateIfNeeded(const Context& context,
                        const VkDescriptorSetLayout* descriptorSetLayouts,
                        std::size_t descriptorSetLayoutSize,
                        VkRenderPass renderPass,
                        bool force = false) const;

    void clearShader(Shader::Type type = Shader::Type::SHADER_NONE);
    void setShader(const Shader& shader);
    [[nodiscard]] const Shader* getShader(Shader::Type type = Shader::Type::SHADER_NONE) const;

    void setBlendMode(const BlendMode& blendMode);
    [[nodiscard]] const BlendMode& getBlendMode() const;

    void setDefaultPrimitiveTopology(VkPrimitiveTopology topology) const;
    [[nodiscard]] VkPrimitiveTopology getDefaultPrimitiveTopology() const;
    void setDefaultVertexCount(uint32_t count) const;
    [[nodiscard]] uint32_t getDefaultVertexCount() const;

    void setViewport(const VkExtent2D& extent2D) const;
    void setViewport(const Viewport& viewport) const;
    [[nodiscard]] const Viewport& getViewport() const;

    void setScissor(const VkRect2D& scissor) const;
    [[nodiscard]] const VkRect2D& getScissor() const;

    void recordCommandBuffer(VkCommandBuffer commandBuffer,
                             const VertexBuffer* vertexBuffer,
                             const IndexBuffer* indexBuffer) const;
    void recordCommandBufferWithoutDraw(VkCommandBuffer commandBuffer,
                                        const VertexBuffer* vertexBuffer,
                                        const IndexBuffer* indexBuffer) const;
    void bindDescriptorSets(VkCommandBuffer commandBuffer,
                            const VkDescriptorSet* descriptorSet,
                            uint32_t descriptorCount,
                            uint32_t firstSet = 0) const;
    void bindDynamicDescriptorSets(VkCommandBuffer commandBuffer,
                                   const VkDescriptorSet* descriptorSet,
                                   uint32_t descriptorCount,
                                   uint32_t dynamicOffsetCount,
                                   const uint32_t* pDynamicOffsets,
                                   uint32_t firstSet = 0) const;

    [[nodiscard]] VkPipelineLayout getPipelineLayout() const;
    [[nodiscard]] VkPipeline getPipeline() const;
    [[nodiscard]] const Context* getContext();

    void destroy();

private:
    void cleanPipeline() const;

    mutable bool g_needUpdate;

    const Shader* g_shaderCompute;
    const Shader* g_shaderVertex;
    const Shader* g_shaderFragment;
    const Shader* g_shaderGeometry;

    mutable VkPrimitiveTopology g_defaultPrimitiveTopology;
    mutable uint32_t g_defaultVertexCount;

    mutable Viewport g_viewport;
    mutable BlendMode g_blendMode;
    mutable VkRect2D g_scissor;

    mutable VkPipelineLayout g_pipelineLayout;
    mutable VkPipeline g_graphicsPipeline;

    mutable const Context* g_context;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
