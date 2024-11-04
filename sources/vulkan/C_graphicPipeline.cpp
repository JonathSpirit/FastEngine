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

#include "FastEngine/vulkan/C_graphicPipeline.hpp"

#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/C_swapChain.hpp"
#include "FastEngine/vulkan/C_vertex.hpp"

namespace fge::vulkan
{

//LayoutPipeline

LayoutPipeline::LayoutPipeline(Context const& context) :
        ContextAware(context),
        g_needUpdate(true),
        g_pipeline(VK_NULL_HANDLE)
{}
LayoutPipeline::LayoutPipeline(LayoutPipeline const& r) :
        ContextAware(r),
        g_needUpdate(true),
        g_pipeline(VK_NULL_HANDLE),
        g_pushConstantRanges(r.g_pushConstantRanges),
        g_descriptorSetLayouts(r.g_descriptorSetLayouts)
{}
LayoutPipeline::LayoutPipeline(LayoutPipeline&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_needUpdate(r.g_needUpdate),
        g_pipeline(r.g_pipeline),
        g_pushConstantRanges(std::move(r.g_pushConstantRanges)),
        g_descriptorSetLayouts(std::move(r.g_descriptorSetLayouts))
{
    r.g_needUpdate = true;
    r.g_pipeline = VK_NULL_HANDLE;
}
LayoutPipeline::~LayoutPipeline()
{
    this->destroy();
}

bool LayoutPipeline::updateIfNeeded(bool force)
{
    if (this->g_needUpdate || force)
    {
        this->clean();
        this->g_needUpdate = false;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = this->g_descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = this->g_descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = this->g_pushConstantRanges.size();
        pipelineLayoutInfo.pPushConstantRanges = this->g_pushConstantRanges.data();

        if (vkCreatePipelineLayout(this->getContext().getLogicalDevice().getDevice(), &pipelineLayoutInfo, nullptr,
                                   &this->g_pipeline) != VK_SUCCESS)
        {
            throw fge::Exception("failed to create pipeline layout!");
        }
        return true;
    }
    return false;
}

void LayoutPipeline::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
    this->g_descriptorSetLayouts.push_back(descriptorSetLayout);
    this->g_needUpdate = true;
}
void LayoutPipeline::setDescriptorSetLayouts(std::initializer_list<VkDescriptorSetLayout> descriptorSetLayouts)
{
    this->g_descriptorSetLayouts = descriptorSetLayouts;
    this->g_needUpdate = true;
}
void LayoutPipeline::setDescriptorSetLayouts(std::vector<VkDescriptorSetLayout> const& descriptorSetLayouts)
{
    this->g_descriptorSetLayouts = descriptorSetLayouts;
    this->g_needUpdate = true;
}
std::vector<VkDescriptorSetLayout> const& LayoutPipeline::getDescriptorSetLayouts() const
{
    return this->g_descriptorSetLayouts;
}

void LayoutPipeline::addPushConstantRanges(std::vector<VkPushConstantRange> const& pushConstantRanges)
{
    this->g_pushConstantRanges.insert(this->g_pushConstantRanges.end(), pushConstantRanges.begin(),
                                      pushConstantRanges.end());
    this->g_needUpdate = true;
}
void LayoutPipeline::setPushConstantRanges(std::initializer_list<VkPushConstantRange> pushConstantRanges)
{
    this->g_pushConstantRanges = pushConstantRanges;
    this->g_needUpdate = true;
}
void LayoutPipeline::setPushConstantRanges(std::vector<VkPushConstantRange> const& pushConstantRanges)
{
    this->g_pushConstantRanges = pushConstantRanges;
    this->g_needUpdate = true;
}
std::vector<VkPushConstantRange> const& LayoutPipeline::getPushConstantRanges() const
{
    return this->g_pushConstantRanges;
}

VkPipelineLayout LayoutPipeline::get() const
{
    return this->g_pipeline;
}

void LayoutPipeline::clean()
{
    if (this->g_pipeline != VK_NULL_HANDLE)
    {
        this->getContext()._garbageCollector.push(
                GarbagePipelineLayout(this->g_pipeline, this->getContext().getLogicalDevice().getDevice()));

        this->g_pipeline = VK_NULL_HANDLE;
    }

    this->g_needUpdate = true;
}
void LayoutPipeline::destroy()
{
    if (this->g_pipeline != VK_NULL_HANDLE)
    {
        this->getContext()._garbageCollector.push(
                GarbagePipelineLayout(this->g_pipeline, this->getContext().getLogicalDevice().getDevice()));

        this->g_pipeline = VK_NULL_HANDLE;
    }

    this->g_needUpdate = true;

    this->g_descriptorSetLayouts.clear();
    this->g_pushConstantRanges.clear();
}

std::size_t LayoutPipeline::Key::Hash::operator()(Key const& r) const
{
    return fge::Hash(&r, sizeof(Key));
}

bool LayoutPipeline::Key::Compare::operator()(Key const& lhs, Key const& rhs) const
{
    return lhs._vertexShader == rhs._vertexShader && lhs._geometryShader == rhs._geometryShader &&
           lhs._fragmentShader == rhs._fragmentShader;
}

//GraphicPipeline

GraphicPipeline::GraphicPipeline(Context const& context) :
        ContextAware(context),
        g_needUpdate(true),

        g_shaderCompute(nullptr),
        g_shaderVertex(nullptr),
        g_shaderFragment(nullptr),
        g_shaderGeometry(nullptr),

        g_primitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
        g_defaultVertexCount(3),

        g_pipelineLayout(VK_NULL_HANDLE),
        g_graphicsPipeline(VK_NULL_HANDLE)
{}
GraphicPipeline::GraphicPipeline(GraphicPipeline const& r) :
        ContextAware(r),
        g_needUpdate(true),

        g_shaderCompute(r.g_shaderCompute),
        g_shaderVertex(r.g_shaderVertex),
        g_shaderFragment(r.g_shaderFragment),
        g_shaderGeometry(r.g_shaderGeometry),

        g_primitiveTopology(r.g_primitiveTopology),
        g_defaultVertexCount(r.g_defaultVertexCount),

        g_blendMode(r.g_blendMode),

        g_pipelineLayout(r.g_pipelineLayout),
        g_graphicsPipeline(VK_NULL_HANDLE)
{}
GraphicPipeline::GraphicPipeline(GraphicPipeline&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_needUpdate(r.g_needUpdate),

        g_shaderCompute(r.g_shaderCompute),
        g_shaderVertex(r.g_shaderVertex),
        g_shaderFragment(r.g_shaderFragment),
        g_shaderGeometry(r.g_shaderGeometry),

        g_primitiveTopology(r.g_primitiveTopology),
        g_defaultVertexCount(r.g_defaultVertexCount),

        g_blendMode(r.g_blendMode),

        g_pipelineLayout(r.g_pipelineLayout),
        g_graphicsPipeline(r.g_graphicsPipeline)
{
    r.g_needUpdate = true;

    r.g_shaderCompute = nullptr;
    r.g_shaderVertex = nullptr;
    r.g_shaderFragment = nullptr;
    r.g_shaderGeometry = nullptr;

    r.g_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    r.g_defaultVertexCount = 3;

    r.g_blendMode = {};

    r.g_pipelineLayout = VK_NULL_HANDLE;
    r.g_graphicsPipeline = VK_NULL_HANDLE;
}
GraphicPipeline::~GraphicPipeline()
{
    this->destroy();
}

bool GraphicPipeline::updateIfNeeded(VkRenderPass renderPass, bool force)
{
    if ((this->g_needUpdate || force) && this->g_pipelineLayout != VK_NULL_HANDLE)
    {
        this->clean();

        this->g_needUpdate = false;

        VkPipelineShaderStageCreateInfo shaderStages[4];
        std::size_t shaderStagesCount = 0;

        if (this->g_shaderCompute != nullptr)
        {
            shaderStages[shaderStagesCount++] = this->g_shaderCompute->getPipelineShaderStageCreateInfo();
        }
        if (this->g_shaderFragment != nullptr)
        {
            shaderStages[shaderStagesCount++] = this->g_shaderFragment->getPipelineShaderStageCreateInfo();
        }
        if (this->g_shaderGeometry != nullptr)
        {
            shaderStages[shaderStagesCount++] = this->g_shaderGeometry->getPipelineShaderStageCreateInfo();
        }
        if (this->g_shaderVertex != nullptr)
        {
            shaderStages[shaderStagesCount++] = this->g_shaderVertex->getPipelineShaderStageCreateInfo();
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = nullptr;
        viewportState.scissorCount = 1;
        viewportState.pScissors = nullptr;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        //rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f;          // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;          // Optional
        multisampling.pSampleMask = nullptr;            // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE;      // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = this->g_blendMode._srcColorBlendFactor;
        colorBlendAttachment.dstColorBlendFactor = this->g_blendMode._dstColorBlendFactor;
        colorBlendAttachment.colorBlendOp = this->g_blendMode._colorBlendOp;
        colorBlendAttachment.srcAlphaBlendFactor = this->g_blendMode._srcAlphaBlendFactor;
        colorBlendAttachment.dstAlphaBlendFactor = this->g_blendMode._dstAlphaBlendFactor;
        colorBlendAttachment.alphaBlendOp = this->g_blendMode._alphaBlendOp;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};

        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = this->g_primitiveTopology;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStagesCount;
        pipelineInfo.pStages = shaderStages;

        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        pipelineInfo.layout = this->g_pipelineLayout;

        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1;              // Optional

        if (vkCreateGraphicsPipelines(this->getContext().getLogicalDevice().getDevice(), VK_NULL_HANDLE, 1,
                                      &pipelineInfo, nullptr, &this->g_graphicsPipeline) != VK_SUCCESS)
        {
            throw fge::Exception("failed to create graphics pipeline!");
        }

        return true;
    }
    return false;
}

void GraphicPipeline::clearShader(Shader::Type type)
{
    switch (type)
    {
    case Shader::Type::SHADER_COMPUTE:
        this->g_shaderCompute = nullptr;
        break;
    case Shader::Type::SHADER_VERTEX:
        this->g_shaderVertex = nullptr;
        break;
    case Shader::Type::SHADER_FRAGMENT:
        this->g_shaderFragment = nullptr;
        break;
    case Shader::Type::SHADER_GEOMETRY:
        this->g_shaderGeometry = nullptr;
        break;
    default:
        this->g_shaderCompute = nullptr;
        this->g_shaderVertex = nullptr;
        this->g_shaderFragment = nullptr;
        this->g_shaderGeometry = nullptr;
        break;
    }
    this->g_needUpdate = true;
}
void GraphicPipeline::setShader(Shader const& shader)
{
    switch (shader.getType())
    {
    case Shader::Type::SHADER_COMPUTE:
        this->g_shaderCompute = &shader;
        this->g_needUpdate = true;
        break;
    case Shader::Type::SHADER_VERTEX:
        this->g_shaderVertex = &shader;
        this->g_needUpdate = true;
        break;
    case Shader::Type::SHADER_FRAGMENT:
        this->g_shaderFragment = &shader;
        this->g_needUpdate = true;
        break;
    case Shader::Type::SHADER_GEOMETRY:
        this->g_shaderGeometry = &shader;
        this->g_needUpdate = true;
        break;
    default:
        break;
    }
}
Shader const* GraphicPipeline::getShader(Shader::Type type) const
{
    switch (type)
    {
    case Shader::Type::SHADER_COMPUTE:
        return this->g_shaderCompute;
    case Shader::Type::SHADER_VERTEX:
        return this->g_shaderVertex;
    case Shader::Type::SHADER_FRAGMENT:
        return this->g_shaderFragment;
    case Shader::Type::SHADER_GEOMETRY:
        return this->g_shaderGeometry;
    default:
        return nullptr;
    }
}

void GraphicPipeline::setBlendMode(BlendMode const& blendMode)
{
    this->g_blendMode = blendMode;
    this->g_needUpdate = true;
}
BlendMode const& GraphicPipeline::getBlendMode() const
{
    return this->g_blendMode;
}

void GraphicPipeline::setPrimitiveTopology(VkPrimitiveTopology topology) const
{
    this->g_primitiveTopology = topology;
    this->g_needUpdate = true;
}
[[nodiscard]] VkPrimitiveTopology GraphicPipeline::getPrimitiveTopology() const
{
    return this->g_primitiveTopology;
}

void GraphicPipeline::setDefaultVertexCount(uint32_t count) const
{
    this->g_defaultVertexCount = count;
}
uint32_t GraphicPipeline::getDefaultVertexCount() const
{
    return this->g_defaultVertexCount;
}

void GraphicPipeline::recordCommandBuffer(CommandBuffer& commandBuffer,
                                          Viewport const& viewport,
                                          VkRect2D const& scissor,
                                          VertexBuffer const* vertexBuffer,
                                          IndexBuffer const* indexBuffer) const
{
    commandBuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, this->g_graphicsPipeline);

    commandBuffer.setViewport(0, 1, &viewport.getViewport());
    commandBuffer.setScissor(0, 1, &scissor);

    if (vertexBuffer != nullptr && vertexBuffer->getType() != BufferTypes::UNINITIALIZED)
    {
        vertexBuffer->bind(commandBuffer);
        if (indexBuffer != nullptr && indexBuffer->getType() != BufferTypes::UNINITIALIZED)
        {
            indexBuffer->bind(commandBuffer);
        }
    }
    else
    {
        VkDeviceSize const offset = 0;
        VkBuffer buffer = VK_NULL_HANDLE;
        commandBuffer.bindVertexBuffers(0, 1, &buffer, &offset);
    }
}

void GraphicPipeline::setPipelineLayout(LayoutPipeline const& layoutPipeline) const
{
    this->g_pipelineLayout = layoutPipeline.get();
    this->g_needUpdate = true;
}
VkPipelineLayout GraphicPipeline::getPipelineLayout() const
{
    return this->g_pipelineLayout;
}
VkPipeline GraphicPipeline::getPipeline() const
{
    return this->g_graphicsPipeline;
}

void GraphicPipeline::clean()
{
    if (this->g_graphicsPipeline != VK_NULL_HANDLE)
    {
        this->getContext()._garbageCollector.push(
                GarbageGraphicPipeline(this->g_graphicsPipeline, this->getContext().getLogicalDevice().getDevice()));

        this->g_graphicsPipeline = VK_NULL_HANDLE;
    }

    this->g_needUpdate = true;
}
void GraphicPipeline::destroy()
{
    if (this->g_graphicsPipeline != VK_NULL_HANDLE)
    {
        this->getContext()._garbageCollector.push(
                GarbageGraphicPipeline(this->g_graphicsPipeline, this->getContext().getLogicalDevice().getDevice()));

        this->g_graphicsPipeline = VK_NULL_HANDLE;
    }

    this->g_needUpdate = true;

    this->g_shaderCompute = nullptr;
    this->g_shaderVertex = nullptr;
    this->g_shaderFragment = nullptr;
    this->g_shaderGeometry = nullptr;

    this->g_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    this->g_defaultVertexCount = 3;

    this->g_blendMode = {};

    this->g_pipelineLayout = VK_NULL_HANDLE;
}

std::size_t GraphicPipeline::Key::Hash::operator()(Key const& r) const
{
    return fge::Hash(&r, sizeof(Key));
}

bool GraphicPipeline::Key::Compare::operator()(Key const& lhs, Key const& rhs) const
{
    return lhs._shaderCompute == rhs._shaderCompute && lhs._shaderVertex == rhs._shaderVertex &&
           lhs._shaderFragment == rhs._shaderFragment && lhs._shaderGeometry == rhs._shaderGeometry &&
           lhs._primitiveTopology == rhs._primitiveTopology && lhs._blendMode == rhs._blendMode &&
           lhs._pipelineLayout == rhs._pipelineLayout;
}

} // namespace fge::vulkan