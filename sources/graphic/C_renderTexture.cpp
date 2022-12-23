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

#include <FastEngine/graphic/C_renderTexture.hpp>
#include <FastEngine/vulkan/C_context.hpp>
#include <FastEngine/graphic/C_transform.hpp>
#include <FastEngine/graphic/C_transformable.hpp>
#include <SDL_events.h>
#include <glm/gtc/type_ptr.hpp>

namespace fge
{

RenderTexture::RenderTexture() = default;

RenderTexture::RenderTexture(const fge::vulkan::Context& context, const glm::vec<2, int>& size)
{
    this->init(context, size);
    this->initialize();
}

RenderTexture::~RenderTexture()
{
    this->destroy();
}

void RenderTexture::create(const fge::vulkan::Context& context, const glm::vec<2, int>& size)
{
    this->init(context, size);
    this->initialize();
}
void RenderTexture::destroy()
{
    if (this->g_isCreated)
    {
        VkDevice logicalDevice = this->g_context->getLogicalDevice().getDevice();

        vkDestroyCommandPool(logicalDevice, this->g_commandPool, nullptr);

        vkDestroyFramebuffer(logicalDevice, this->g_framebuffer, nullptr);

        vkDestroyRenderPass(logicalDevice, this->g_renderPass, nullptr);

        this->g_descriptorSetLayout.destroy();

        this->g_textureImage.destroy();

        this->g_isCreated = false;
    }
}

uint32_t RenderTexture::prepareNextFrame(const VkCommandBufferInheritanceInfo* inheritanceInfo)
{
    vkResetCommandBuffer(this->g_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = inheritanceInfo;

    if (vkBeginCommandBuffer(this->g_commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    return BAD_IMAGE_INDEX;
}
void RenderTexture::beginRenderPass([[maybe_unused]] uint32_t imageIndex)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->g_renderPass;
    renderPassInfo.framebuffer = this->g_framebuffer;

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->g_textureImage.getExtent();

    const VkClearValue clearColor = {.color=this->_g_clearColor};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(this->g_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void RenderTexture::draw(fge::vulkan::GraphicPipeline& graphicPipeline, const RenderStates& states)
{
    states._transformable->setViewMatrix(this->getView().getTransform());
    states._transformable->updateUniformBuffer(*this->g_context);

    auto windowSize = static_cast<fge::Vector2f>(this->getSize());
    auto factorViewport = this->getView().getFactorViewport();

    const fge::vulkan::Viewport viewport(windowSize.x*factorViewport._x, windowSize.y*factorViewport._y,
                                         windowSize.x*factorViewport._width,windowSize.y*factorViewport._height);
    graphicPipeline.setViewport(viewport);

    VkDescriptorSetLayout layout[] = {this->g_descriptorSetLayout.getLayout(), this->g_context->getDescriptorSetLayout().getLayout()};
    graphicPipeline.updateIfNeeded(this->g_textureImage.getExtent(),
                                   this->g_context->getLogicalDevice(),
                                   layout, 2,
                                   this->g_renderPass,
                                   this->g_forceGraphicPipelineUpdate);

    VkDescriptorSet descriptorSets[] = {states._transformable->getDescriptorSet().getDescriptorSet(), states._textureImage->getDescriptorSet().getDescriptorSet()};
    graphicPipeline.bindDescriptorSets(this->g_commandBuffer, descriptorSets, 2);
    graphicPipeline.recordCommandBuffer(this->g_commandBuffer);
}
void RenderTexture::endRenderPass()
{
    vkCmdEndRenderPass(this->g_commandBuffer);
}
void RenderTexture::display([[maybe_unused]] uint32_t imageIndex,
                            [[maybe_unused]] const VkCommandBuffer* extraCommandBuffer,
                            [[maybe_unused]] std::size_t extraCommandBufferSize)
{
    if (vkEndCommandBuffer(this->g_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

Vector2u RenderTexture::getSize() const
{
    return static_cast<Vector2u>(this->g_textureImage.getSize());
}

bool RenderTexture::isSrgb() const
{
    return false; ///TODO
}

const fge::vulkan::DescriptorSetLayout& RenderTexture::getDescriptorSetLayout() const
{
    return this->g_descriptorSetLayout;
}
VkCommandBuffer RenderTexture::getCommandBuffer() const
{
    return this->g_commandBuffer;
}
const fge::vulkan::TextureImage& RenderTexture::getTextureImage() const
{
    return this->g_textureImage;
}

void RenderTexture::init(const fge::vulkan::Context& context, const glm::vec<2, int>& size)
{
    if (this->g_isCreated)
    {
        this->destroy();
    }
    this->g_isCreated = true;

    this->g_context = &context;

    this->g_textureImage.create(context, size);

    this->createRenderPass();

    this->g_descriptorSetLayout.create(this->g_context->getLogicalDevice(), {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}
    });

    this->createFramebuffer();

    this->createCommandPool();
    this->createCommandBuffer();
}

void RenderTexture::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(this->g_context->getLogicalDevice().getDevice(), &renderPassInfo, nullptr, &this->g_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderTexture::createFramebuffer()
{
    VkImageView attachments[] = {
            this->g_textureImage.getTextureImageView()
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = this->g_renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = this->g_textureImage.getSize().x;
    framebufferInfo.height = this->g_textureImage.getSize().y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(this->g_context->getLogicalDevice().getDevice(), &framebufferInfo, nullptr, &this->g_framebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void RenderTexture::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = this->g_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(this->g_context->getLogicalDevice().getDevice(), &allocInfo, &this->g_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
void RenderTexture::createCommandPool()
{
    auto queueFamilyIndices = this->g_context->getPhysicalDevice().findQueueFamilies(this->g_context->getSurface().getSurface());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices._graphicsFamily.value();

    if (vkCreateCommandPool(this->g_context->getLogicalDevice().getDevice(), &poolInfo, nullptr, &this->g_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

}//end fge
