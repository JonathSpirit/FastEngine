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

#ifndef _FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
#define _FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include "C_blendMode.hpp"
#include "C_contextAware.hpp"
#include "C_shader.hpp"
#include "C_vertex.hpp"
#include "C_vertexBuffer.hpp"
#include "C_viewport.hpp"
#include "vulkanGlobal.hpp"
#include <initializer_list>

namespace fge::vulkan
{

class FGE_API GraphicPipeline : public ContextAware
{
public:
    explicit GraphicPipeline(Context const& context);
    GraphicPipeline(GraphicPipeline const& r);
    GraphicPipeline(GraphicPipeline&& r) noexcept;
    ~GraphicPipeline() override;

    GraphicPipeline& operator=(GraphicPipeline const& r) = delete;
    GraphicPipeline& operator=(GraphicPipeline&& r) noexcept = delete;

    bool updateIfNeeded(VkRenderPass renderPass, bool force = false) const;

    void setDescriptorSetLayouts(std::initializer_list<VkDescriptorSetLayout> descriptorSetLayouts);
    [[nodiscard]] std::vector<VkDescriptorSetLayout> const& getDescriptorSetLayouts() const;

    void clearShader(Shader::Type type = Shader::Type::SHADER_NONE);
    void setShader(Shader const& shader);
    [[nodiscard]] Shader const* getShader(Shader::Type type = Shader::Type::SHADER_NONE) const;

    void setBlendMode(BlendMode const& blendMode);
    [[nodiscard]] BlendMode const& getBlendMode() const;

    void setPrimitiveTopology(VkPrimitiveTopology topology) const;
    [[nodiscard]] VkPrimitiveTopology getPrimitiveTopology() const;

    void setDefaultVertexCount(uint32_t count) const;
    [[nodiscard]] uint32_t getDefaultVertexCount() const;

    void setScissor(VkRect2D const& scissor) const;
    [[nodiscard]] VkRect2D const& getScissor() const;

    void setPushConstantRanges(std::initializer_list<VkPushConstantRange> pushConstantRanges);
    [[nodiscard]] std::vector<VkPushConstantRange> const& getPushConstantRanges() const;

    void recordCommandBuffer(VkCommandBuffer commandBuffer,
                             Viewport const& viewport,
                             VertexBuffer const* vertexBuffer,
                             IndexBuffer const* indexBuffer) const;
    void recordCommandBufferWithoutDraw(VkCommandBuffer commandBuffer,
                                        Viewport const& viewport,
                                        VertexBuffer const* vertexBuffer,
                                        IndexBuffer const* indexBuffer) const;
    void bindDescriptorSets(VkCommandBuffer commandBuffer,
                            VkDescriptorSet const* descriptorSet,
                            uint32_t descriptorCount,
                            uint32_t firstSet = 0) const;
    void bindDynamicDescriptorSets(VkCommandBuffer commandBuffer,
                                   VkDescriptorSet const* descriptorSet,
                                   uint32_t descriptorCount,
                                   uint32_t dynamicOffsetCount,
                                   uint32_t const* pDynamicOffsets,
                                   uint32_t firstSet = 0) const;

    void pushConstants(VkCommandBuffer commandBuffer,
                       VkShaderStageFlags stageFlags,
                       uint32_t offset,
                       uint32_t size,
                       void const* pValues) const;

    [[nodiscard]] VkPipelineLayout getPipelineLayout() const;
    [[nodiscard]] VkPipeline getPipeline() const;

    void destroy() final;

private:
    void updatePipelineLayout() const;
    void cleanPipelineLayout() const;
    void cleanPipeline() const;

    mutable bool g_needUpdate;

    Shader const* g_shaderCompute;
    Shader const* g_shaderVertex;
    Shader const* g_shaderFragment;
    Shader const* g_shaderGeometry;

    mutable VkPrimitiveTopology g_primitiveTopology;
    mutable uint32_t g_defaultVertexCount;

    mutable BlendMode g_blendMode;
    mutable VkRect2D g_scissor;

    mutable VkPipelineLayout g_pipelineLayout;
    mutable VkPipeline g_graphicsPipeline;

    std::vector<VkPushConstantRange> g_pushConstantRanges;
    std::vector<VkDescriptorSetLayout> g_descriptorSetLayouts;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
