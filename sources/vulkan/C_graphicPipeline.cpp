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

#include "Fastengine/vulkan/C_graphicPipeline.hpp"
#include "Fastengine/vulkan/C_swapChain.hpp"
#include "Fastengine/vulkan/C_logicalDevice.hpp"

namespace fge::vulkan
{

GraphicPipeline::GraphicPipeline() :
        g_needUpdate(true),

        g_shaderCompute(nullptr),
        g_shaderVertex(GraphicPipeline::defaultShaderVertex),
        g_shaderFragment(GraphicPipeline::defaultShaderFragment),
        g_shaderGeometry(nullptr),

        g_vertexBuffer(nullptr),

        g_primitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN),

        g_viewport(),
        g_blendMode(),

        g_pipelineLayout(VK_NULL_HANDLE),
        g_graphicsPipeline(VK_NULL_HANDLE),

        g_logicalDevice(nullptr)
{
    this->setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
}
GraphicPipeline::GraphicPipeline(const GraphicPipeline& r) :
        g_needUpdate(true),

        g_shaderCompute(r.g_shaderCompute),
        g_shaderVertex(r.g_shaderVertex),
        g_shaderFragment(r.g_shaderFragment),
        g_shaderGeometry(r.g_shaderGeometry),

        g_vertexBuffer(r.g_vertexBuffer),

        g_primitiveTopology(r.g_primitiveTopology),

        g_viewport(r.g_viewport),
        g_blendMode(r.g_blendMode),

        g_pipelineLayout(VK_NULL_HANDLE),
        g_graphicsPipeline(VK_NULL_HANDLE),

        g_logicalDevice(r.g_logicalDevice)
{}
GraphicPipeline::GraphicPipeline(GraphicPipeline&& r) noexcept :
        g_needUpdate(r.g_needUpdate),

        g_shaderCompute(r.g_shaderCompute),
        g_shaderVertex(r.g_shaderVertex),
        g_shaderFragment(r.g_shaderFragment),
        g_shaderGeometry(r.g_shaderGeometry),

        g_vertexBuffer(r.g_vertexBuffer),

        g_primitiveTopology(r.g_primitiveTopology),

        g_viewport(r.g_viewport),
        g_blendMode(r.g_blendMode),

        g_pipelineLayout(r.g_pipelineLayout),
        g_graphicsPipeline(r.g_graphicsPipeline),

        g_logicalDevice(r.g_logicalDevice)
{
    r.g_needUpdate = true;

    r.g_shaderCompute = nullptr;
    r.g_shaderVertex = GraphicPipeline::defaultShaderVertex;
    r.g_shaderFragment = GraphicPipeline::defaultShaderFragment;
    r.g_shaderGeometry = nullptr;

    r.g_vertexBuffer = nullptr;

    r.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    r.g_viewport = {};
    r.g_blendMode = {};

    r.g_pipelineLayout = VK_NULL_HANDLE;
    r.g_graphicsPipeline = VK_NULL_HANDLE;

    r.g_logicalDevice = nullptr;
}
GraphicPipeline::~GraphicPipeline()
{
    this->destroy();
}

bool GraphicPipeline::updateIfNeeded(const LogicalDevice& logicalDevice,
                                     const VkDescriptorSetLayout* descriptorSetLayouts,
                                     std::size_t descriptorSetLayoutSize,
                                     VkRenderPass renderPass,
                                     bool force) const
{
    if (this->g_needUpdate || force)
    {
        this->cleanPipeline();

        this->g_needUpdate = false;

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(4);

        if (this->g_shaderCompute != nullptr)
        {
            shaderStages.push_back(this->g_shaderCompute->getPipelineShaderStageCreateInfo());
        }
        if (this->g_shaderFragment != nullptr)
        {
            shaderStages.push_back(this->g_shaderFragment->getPipelineShaderStageCreateInfo());
        }
        if (this->g_shaderGeometry != nullptr)
        {
            shaderStages.push_back(this->g_shaderGeometry->getPipelineShaderStageCreateInfo());
        }
        if (this->g_shaderVertex != nullptr)
        {
            shaderStages.push_back(this->g_shaderVertex->getPipelineShaderStageCreateInfo());
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        if (this->g_vertexBuffer != nullptr && this->g_vertexBuffer->getType() != VertexBuffer::Types::UNINITIALIZED)
        {
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        }
        else
        {
            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.pVertexBindingDescriptions = nullptr;
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            vertexInputInfo.pVertexAttributeDescriptions = nullptr;
        }

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
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
                VK_DYNAMIC_STATE_SCISSOR
                /*,
                VK_DYNAMIC_STATE_LINE_WIDTH,
                VK_DYNAMIC_STATE_BLEND_CONSTANTS*/
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutSize;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(logicalDevice.getDevice(), &pipelineLayoutInfo, nullptr, &this->g_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};

        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = this->g_primitiveTopology;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();

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
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(logicalDevice.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->g_graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        this->g_logicalDevice = &logicalDevice;
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
void GraphicPipeline::setShader(const Shader& shader)
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
const Shader* GraphicPipeline::getShader(Shader::Type type) const
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

void GraphicPipeline::setBlendMode(const BlendMode& blendMode)
{
    this->g_blendMode = blendMode;
    this->g_needUpdate = true;
}
const BlendMode& GraphicPipeline::getBlendMode() const
{
    return this->g_blendMode;
}

void GraphicPipeline::setPrimitiveTopology(VkPrimitiveTopology topology) const
{
    this->g_primitiveTopology = topology;
}
VkPrimitiveTopology GraphicPipeline::getPrimitiveTopology() const
{
    return this->g_primitiveTopology;
}

void GraphicPipeline::setViewport(const VkExtent2D& extent2D) const
{
    this->g_viewport.setPosition(0.0f, 0.0f);
    this->g_viewport.setSize(static_cast<float>(extent2D.width), static_cast<float>(extent2D.height));
}
void GraphicPipeline::setViewport(const Viewport& viewport) const
{
    this->g_viewport = viewport;
}
const Viewport& GraphicPipeline::getViewport() const
{
    return this->g_viewport;
}

void GraphicPipeline::setScissor(const VkRect2D& scissor) const
{
    this->g_scissor = scissor;
}
const VkRect2D& GraphicPipeline::getScissor() const
{
    return this->g_scissor;
}

void GraphicPipeline::setVertexBuffer(const VertexBuffer* vertexBuffer) const
{
    this->g_vertexBuffer = vertexBuffer;
}
const VertexBuffer* GraphicPipeline::getVertexBuffer() const
{
    return this->g_vertexBuffer;
}

void GraphicPipeline::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t defaultVertexCount) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->g_graphicsPipeline);

    vkCmdSetViewport(commandBuffer, 0, 1, &this->g_viewport.getViewport());
    vkCmdSetPrimitiveTopologyEXT(commandBuffer, this->g_primitiveTopology);
    vkCmdSetScissor(commandBuffer, 0, 1, &this->g_scissor);

    if (this->g_vertexBuffer != nullptr && this->g_vertexBuffer->getType() != VertexBuffer::Types::UNINITIALIZED)
    {
        this->g_vertexBuffer->bind(commandBuffer);
        if (this->g_vertexBuffer->isUsingIndexBuffer())
        {
            vkCmdDrawIndexed(commandBuffer, this->g_vertexBuffer->getIndexCount(), 1, 0, 0, 0);
        }
        else
        {
            vkCmdDraw(commandBuffer, this->g_vertexBuffer->getVertexCount(), 1, 0, 0);
        }
    }
    else
    {
        vkCmdDraw(commandBuffer, defaultVertexCount, 1, 0, 0);
    }
}
void GraphicPipeline::bindDescriptorSets(VkCommandBuffer commandBuffer, const VkDescriptorSet* descriptorSet, uint32_t descriptorCount) const
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            this->g_pipelineLayout, 0, descriptorCount, descriptorSet, 0, nullptr);
}

VkPipelineLayout GraphicPipeline::getPipelineLayout() const
{
    return this->g_pipelineLayout;
}
VkPipeline GraphicPipeline::getPipeline() const
{
    return this->g_graphicsPipeline;
}
const LogicalDevice* GraphicPipeline::getLogicalDevice()
{
    return this->g_logicalDevice;
}

void GraphicPipeline::cleanPipeline() const
{
    if (this->g_graphicsPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(this->g_logicalDevice->getDevice(), this->g_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(this->g_logicalDevice->getDevice(), this->g_pipelineLayout, nullptr);

        this->g_pipelineLayout = VK_NULL_HANDLE;
        this->g_graphicsPipeline = VK_NULL_HANDLE;
    }
}

void GraphicPipeline::destroy()
{
    if (this->g_graphicsPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(this->g_logicalDevice->getDevice(), this->g_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(this->g_logicalDevice->getDevice(), this->g_pipelineLayout, nullptr);

        this->g_needUpdate = true;

        this->g_shaderCompute = nullptr;
        this->g_shaderVertex = GraphicPipeline::defaultShaderVertex;
        this->g_shaderFragment = GraphicPipeline::defaultShaderFragment;
        this->g_shaderGeometry = nullptr;

        this->g_vertexBuffer = nullptr;

        this->setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        this->g_viewport = {};

        this->g_pipelineLayout = VK_NULL_HANDLE;
        this->g_graphicsPipeline = VK_NULL_HANDLE;

        this->g_logicalDevice = nullptr;
    }
}

const Shader* GraphicPipeline::defaultShaderVertex;
const Shader* GraphicPipeline::defaultShaderFragment;
const Shader* GraphicPipeline::defaultShaderFragmentNoTexture;

}//end fge::vulkan