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

#ifndef _FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
#define _FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include "C_blendMode.hpp"
#include "C_contextAware.hpp"
#include "C_shader.hpp"
#include "C_vertexBuffer.hpp"
#include "C_viewport.hpp"
#include <initializer_list>

namespace fge::vulkan
{

class CommandBuffer;

class FGE_API LayoutPipeline : public ContextAware
{
public:
    explicit LayoutPipeline(Context const& context);
    LayoutPipeline(LayoutPipeline const& r);
    LayoutPipeline(LayoutPipeline&& r) noexcept;
    ~LayoutPipeline() override;

    LayoutPipeline& operator=(LayoutPipeline const& r) = delete;
    LayoutPipeline& operator=(LayoutPipeline&& r) noexcept = delete;

    bool updateIfNeeded(bool force = false);

    void addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    void setDescriptorSetLayouts(std::initializer_list<VkDescriptorSetLayout> descriptorSetLayouts);
    void setDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> const& descriptorSetLayouts);
    [[nodiscard]] std::vector<VkDescriptorSetLayout> const& getDescriptorSetLayouts() const;

    void addPushConstantRanges(std::vector<VkPushConstantRange> const& pushConstantRanges);
    void setPushConstantRanges(std::initializer_list<VkPushConstantRange> pushConstantRanges);
    void setPushConstantRanges(std::vector<VkPushConstantRange> const& pushConstantRanges);
    [[nodiscard]] std::vector<VkPushConstantRange> const& getPushConstantRanges() const;

    [[nodiscard]] VkPipelineLayout get() const;

    void destroy() final;

    struct Key
    {
        VkShaderModule _vertexShader;
        VkShaderModule _geometryShader;
        VkShaderModule _fragmentShader;

        struct Hash
        {
            [[nodiscard]] std::size_t operator()(Key const& r) const;
        };

        struct Compare
        {
            [[nodiscard]] bool operator()(Key const& lhs, Key const& rhs) const;
        };
    };

private:
    void clean();

    mutable bool g_needUpdate;

    mutable VkPipelineLayout g_pipeline;

    std::vector<VkPushConstantRange> g_pushConstantRanges;
    std::vector<VkDescriptorSetLayout> g_descriptorSetLayouts;
};

class FGE_API GraphicPipeline : public ContextAware
{
public:
    explicit GraphicPipeline(Context const& context);
    GraphicPipeline(GraphicPipeline const& r);
    GraphicPipeline(GraphicPipeline&& r) noexcept;
    ~GraphicPipeline() override;

    GraphicPipeline& operator=(GraphicPipeline const& r) = delete;
    GraphicPipeline& operator=(GraphicPipeline&& r) noexcept = delete;

    bool updateIfNeeded(VkRenderPass renderPass, bool force = false);

    void clearShader(Shader::Type type = Shader::Type::SHADER_NONE);
    void setShader(Shader const& shader);
    [[nodiscard]] Shader const* getShader(Shader::Type type = Shader::Type::SHADER_NONE) const;

    void setBlendMode(BlendMode const& blendMode);
    [[nodiscard]] BlendMode const& getBlendMode() const;

    void setPrimitiveTopology(VkPrimitiveTopology topology) const;
    [[nodiscard]] VkPrimitiveTopology getPrimitiveTopology() const;

    void recordCommandBuffer(CommandBuffer& commandBuffer,
                             Viewport const& viewport,
                             VkRect2D const& scissor,
                             VertexBuffer const* vertexBuffer,
                             IndexBuffer const* indexBuffer) const;

    void setPipelineLayout(LayoutPipeline const& layoutPipeline) const;
    [[nodiscard]] VkPipelineLayout getPipelineLayout() const;
    [[nodiscard]] VkPipeline getPipeline() const;

    void destroy() final;

    struct Key
    {
        VkShaderModule _shaderCompute;
        VkShaderModule _shaderVertex;
        VkShaderModule _shaderFragment;
        VkShaderModule _shaderGeometry;

        VkPrimitiveTopology _primitiveTopology;

        BlendMode _blendMode;

        VkPipelineLayout _pipelineLayout;

        struct Hash
        {
            [[nodiscard]] std::size_t operator()(Key const& r) const;
        };

        struct Compare
        {
            [[nodiscard]] bool operator()(Key const& lhs, Key const& rhs) const;
        };
    };

private:
    void clean();

    mutable bool g_needUpdate;

    Shader const* g_shaderCompute;
    Shader const* g_shaderVertex;
    Shader const* g_shaderFragment;
    Shader const* g_shaderGeometry;

    mutable VkPrimitiveTopology g_primitiveTopology;

    mutable BlendMode g_blendMode;

    mutable VkPipelineLayout g_pipelineLayout;
    mutable VkPipeline g_graphicsPipeline;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
