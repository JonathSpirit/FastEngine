/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/graphic/C_renderTexture.hpp"
#include "FastEngine/graphic/C_transform.hpp"
#include "FastEngine/graphic/C_transformable.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace fge
{

RenderTexture::RenderTexture(glm::vec<2, int> const& size, fge::vulkan::Context const& context) :
        RenderTarget(context),
        g_textureImage(context),
        g_renderPass(VK_NULL_HANDLE),
        g_framebuffer(VK_NULL_HANDLE),
        g_commandBuffers({VK_NULL_HANDLE}),
        g_currentFrame(0),
        g_isCreated(false)
{
    this->init(size);
    this->initialize();
}
RenderTexture::RenderTexture(RenderTexture const& r) :
        RenderTarget(r),
        g_textureImage(r.getContext()),
        g_renderPass(VK_NULL_HANDLE),
        g_framebuffer(VK_NULL_HANDLE),
        g_currentFrame(0),
        g_isCreated(false)
{
    this->init(r.g_textureImage.getSize());
    this->initialize();
}
RenderTexture::RenderTexture(RenderTexture&& r) noexcept :
        RenderTarget(static_cast<RenderTarget&&>(r)),
        g_textureImage(std::move(r.g_textureImage)),
        g_renderPass(r.g_renderPass),
        g_framebuffer(r.g_framebuffer),
        g_commandBuffers(std::move(r.g_commandBuffers)),
        g_currentFrame(r.g_currentFrame),
        g_isCreated(r.g_isCreated)
{
    r.g_renderPass = VK_NULL_HANDLE;
    r.g_framebuffer = VK_NULL_HANDLE;
    r.g_currentFrame = 0;
    r.g_isCreated = false;
}
RenderTexture::~RenderTexture()
{
    this->destroy();
}

RenderTexture& RenderTexture::operator=(RenderTexture const& r)
{
    this->verifyContext(r);
    this->destroy();
    this->init(r.g_textureImage.getSize());
    this->initialize();
    return *this;
}
RenderTexture& RenderTexture::operator=(RenderTexture&& r) noexcept
{
    this->verifyContext(r);
    this->destroy();
    this->g_textureImage = std::move(r.g_textureImage);
    this->g_renderPass = r.g_renderPass;
    this->g_framebuffer = r.g_framebuffer;
    this->g_commandBuffers = std::move(r.g_commandBuffers);
    this->g_currentFrame = r.g_currentFrame;
    this->g_isCreated = r.g_isCreated;

    r.g_renderPass = VK_NULL_HANDLE;
    r.g_framebuffer = VK_NULL_HANDLE;
    r.g_currentFrame = 0;
    r.g_isCreated = false;
    return *this;
}

void RenderTexture::resize(glm::vec<2, int> const& size)
{
    this->destroy();
    this->init(size);
    this->initialize();
}
void RenderTexture::destroy()
{
    if (this->g_isCreated)
    {
        this->clearGraphicPipelineCache();

        VkDevice logicalDevice = this->getContext().getLogicalDevice().getDevice();

        for (auto commandBuffer: this->g_commandBuffers)
        {
            this->getContext()._garbageCollector.push(fge::vulkan::GarbageCommandBuffer(
                    this->getContext().getGraphicsCommandPool(), commandBuffer, logicalDevice));
        }
        this->getContext()._garbageCollector.push(fge::vulkan::GarbageFramebuffer(this->g_framebuffer, logicalDevice));
        this->getContext()._garbageCollector.push(fge::vulkan::GarbageRenderPass(this->g_renderPass, logicalDevice));

        this->g_textureImage.destroy();

        this->g_renderPass = VK_NULL_HANDLE;
        this->g_framebuffer = VK_NULL_HANDLE;
        this->g_currentFrame = 0;

        this->g_isCreated = false;
    }
}

uint32_t RenderTexture::prepareNextFrame(VkCommandBufferInheritanceInfo const* inheritanceInfo)
{
    vkResetCommandBuffer(this->g_commandBuffers[this->g_currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = inheritanceInfo;

    if (vkBeginCommandBuffer(this->g_commandBuffers[this->g_currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw fge::Exception("failed to begin recording command buffer!");
    }

    return FGE_RENDERTARGET_BAD_IMAGE_INDEX;
}
void RenderTexture::beginRenderPass([[maybe_unused]] uint32_t imageIndex)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->g_renderPass;
    renderPassInfo.framebuffer = this->g_framebuffer;

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->g_textureImage.getExtent();

    VkClearValue const clearColor = {.color = this->_g_clearColor};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(this->g_commandBuffers[this->g_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void RenderTexture::endRenderPass()
{
    vkCmdEndRenderPass(this->g_commandBuffers[this->g_currentFrame]);

    if (vkEndCommandBuffer(this->g_commandBuffers[this->g_currentFrame]) != VK_SUCCESS)
    {
        throw fge::Exception("failed to record command buffer!");
    }
}
void RenderTexture::display([[maybe_unused]] uint32_t imageIndex)
{
    this->getContext().pushGraphicsCommandBuffer(this->g_commandBuffers[this->g_currentFrame]);

    this->g_currentFrame = (this->g_currentFrame + 1) % FGE_MAX_FRAMES_IN_FLIGHT;
}

Vector2u RenderTexture::getSize() const
{
    return static_cast<Vector2u>(this->g_textureImage.getSize());
}

bool RenderTexture::isSrgb() const
{
    return false; ///TODO
}

VkExtent2D RenderTexture::getExtent2D() const
{
    return this->g_textureImage.getExtent();
}
VkCommandBuffer RenderTexture::getCommandBuffer() const
{
    return this->g_commandBuffers[this->g_currentFrame];
}
VkRenderPass RenderTexture::getRenderPass() const
{
    return this->g_renderPass;
}

fge::vulkan::TextureImage const& RenderTexture::getTextureImage() const
{
    return this->g_textureImage;
}

uint32_t RenderTexture::getCurrentFrame() const
{
    return this->g_currentFrame;
}

void RenderTexture::init(glm::vec<2, int> const& size)
{
    if (this->g_isCreated)
    {
        this->destroy();
    }
    this->g_isCreated = true;

    this->g_textureImage.create(size);

    this->createRenderPass();

    this->createFramebuffer();

    //create command buffers
    this->getContext().allocateGraphicsCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, this->g_commandBuffers.data(),
                                                      this->g_commandBuffers.size());
}

void RenderTexture::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    std::array<VkSubpassDependency, 2> dependencies{};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(this->getContext().getLogicalDevice().getDevice(), &renderPassInfo, nullptr,
                           &this->g_renderPass) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create render pass!");
    }
}

void RenderTexture::createFramebuffer()
{
    VkImageView attachments[] = {this->g_textureImage.getTextureImageView()};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = this->g_renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = this->g_textureImage.getSize().x;
    framebufferInfo.height = this->g_textureImage.getSize().y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(this->getContext().getLogicalDevice().getDevice(), &framebufferInfo, nullptr,
                            &this->g_framebuffer) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create framebuffer!");
    }
}

} // namespace fge
