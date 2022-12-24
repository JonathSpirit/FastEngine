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
#include "C_shader.hpp"
#include "C_viewport.hpp"
#include "C_vertex.hpp"
#include "C_vertexBuffer.hpp"
#include "C_blendMode.hpp"

namespace fge::vulkan
{

class SwapChain;
class LogicalDevice;

class FGE_API GraphicPipeline
{
public:
    GraphicPipeline();
    GraphicPipeline(const GraphicPipeline& r) = delete;
    GraphicPipeline(GraphicPipeline&& r) noexcept;
    ~GraphicPipeline();

    GraphicPipeline& operator=(const GraphicPipeline& r) = delete;
    GraphicPipeline& operator=(GraphicPipeline&& r) noexcept = delete;

    void updateIfNeeded(const VkExtent2D& extent2D,
                        const LogicalDevice& logicalDevice,
                        const VkDescriptorSetLayout* descriptorSetLayouts,
                        std::size_t descriptorSetLayoutSize,
                        VkRenderPass renderPass,
                        bool force=false) const;

    void clearShader(Shader::Type type=Shader::Type::SHADER_NONE);
    void setShader(const Shader& shader);
    [[nodiscard]] const Shader* getShader(Shader::Type type=Shader::Type::SHADER_NONE) const;

    void setBlendMode(const BlendMode& blendMode);
    [[nodiscard]] const BlendMode& getBlendMode() const;

    void setPrimitiveTopology(VkPrimitiveTopology topology);

    void setViewport(const VkExtent2D& extent2D) const;
    void setViewport(const Viewport& viewport) const;
    [[nodiscard]] const Viewport& getViewport() const;

    void setVertexBuffer(const VertexBuffer* vertexBuffer) const;
    [[nodiscard]] const VertexBuffer* getVertexBuffer() const;

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t defaultVertexCount=3) const;
    void bindDescriptorSets(VkCommandBuffer commandBuffer, const VkDescriptorSet* descriptorSet, uint32_t descriptorCount) const;

    [[nodiscard]] VkPipelineLayout getPipelineLayout() const;
    [[nodiscard]] VkPipeline getPipeline() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice();

    void destroy();

private:
    void cleanPipeline() const;

    mutable bool g_needUpdate;

    const Shader* g_shaderCompute;
    const Shader* g_shaderVertex;
    const Shader* g_shaderFragment;
    const Shader* g_shaderGeometry;

    mutable const VertexBuffer* g_vertexBuffer;

    VkPipelineInputAssemblyStateCreateInfo g_inputAssembly;

    mutable Viewport g_viewport;
    mutable BlendMode g_blendMode;

    mutable VkPipelineLayout g_pipelineLayout;
    mutable VkPipeline g_graphicsPipeline;

    mutable const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
