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

#include "FastEngine/graphic/C_renderWindow.hpp"
#include "FastEngine/graphic/C_transform.hpp"
#include "FastEngine/graphic/C_transformable.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "SDL_events.h"
#include "glm/gtc/type_ptr.hpp"

namespace fge
{

namespace
{

int ResizeCallback(void* userdata, SDL_Event* event)
{
    if (event->type == SDL_WINDOWEVENT)
    {
        if (event->window.event == SDL_WINDOWEVENT_RESIZED)
        {
            reinterpret_cast<fge::RenderWindow*>(userdata)->onResize();
        }
    }
    return 0;
}

} // namespace

RenderWindow::RenderWindow(const fge::vulkan::Context& context) :
        RenderTarget(context),
        g_commandBuffers({VK_NULL_HANDLE}),
        g_imageAvailableSemaphores({VK_NULL_HANDLE}),
        g_renderFinishedSemaphores({VK_NULL_HANDLE}),
        g_inFlightFences({VK_NULL_HANDLE})
{
    this->init();
    this->initialize();
}

RenderWindow::~RenderWindow()
{
    this->destroy();
}

void RenderWindow::destroy()
{
    if (this->g_isCreated)
    {
        SDL_DelEventWatch(&ResizeCallback, this);

        this->clearGraphicPipelineCache();

        VkDevice logicalDevice = this->getContext().getLogicalDevice().getDevice();

        for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(logicalDevice, this->g_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(logicalDevice, this->g_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(logicalDevice, this->g_inFlightFences[i], nullptr);
        }

        vkFreeCommandBuffers(logicalDevice, this->getContext().getGraphicsCommandPool(), FGE_MAX_FRAMES_IN_FLIGHT,
                             this->g_commandBuffers.data());
        for (auto framebuffer: this->g_swapChainFramebuffers)
        {
            vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
        }

        vkDestroyRenderPass(logicalDevice, this->g_renderPass, nullptr);

        this->g_swapChain.destroy();

        this->g_isCreated = false;
    }
}

uint32_t RenderWindow::prepareNextFrame([[maybe_unused]] const VkCommandBufferInheritanceInfo* inheritanceInfo)
{
    vkWaitForFences(this->getContext().getLogicalDevice().getDevice(), 1, &this->g_inFlightFences[this->g_currentFrame],
                    VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    const VkResult result = vkAcquireNextImageKHR(
            this->getContext().getLogicalDevice().getDevice(), this->g_swapChain.getSwapChain(), UINT64_MAX,
            this->g_imageAvailableSemaphores[this->g_currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        this->recreateSwapChain();
        return FGE_RENDERTARGET_BAD_IMAGE_INDEX;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw fge::Exception("failed to acquire swap chain image!");
    }

    // Only reset the fence if we are submitting work
    vkResetFences(this->getContext().getLogicalDevice().getDevice(), 1, &this->g_inFlightFences[this->g_currentFrame]);

    vkResetCommandBuffer(this->g_commandBuffers[this->g_currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(this->g_commandBuffers[this->g_currentFrame], &beginInfo) != VK_SUCCESS)
    {
        throw fge::Exception("failed to begin recording command buffer!");
    }

    return imageIndex;
}
void RenderWindow::beginRenderPass(uint32_t imageIndex)
{
    fge::RenderTarget::gLastTexture = nullptr;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->g_renderPass;
    renderPassInfo.framebuffer = this->g_swapChainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->g_swapChain.getSwapChainExtent();

    const VkClearValue clearColor = {.color = this->_g_clearColor};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(this->g_commandBuffers[this->g_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void RenderWindow::endRenderPass()
{
    vkCmdEndRenderPass(this->g_commandBuffers[this->g_currentFrame]);

    if (this->_g_forceGraphicPipelineUpdate)
    {
        this->_g_forceGraphicPipelineUpdate = false;
    }

    if (vkEndCommandBuffer(this->g_commandBuffers[this->g_currentFrame]) != VK_SUCCESS)
    {
        throw fge::Exception("failed to record command buffer!");
    }
}
void RenderWindow::display(uint32_t imageIndex)
{
    this->getContext().pushGraphicsCommandBuffer(this->g_commandBuffers[this->g_currentFrame]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    auto contextSemaphore = this->getContext().getOutsideRenderScopeSemaphore();

    VkSemaphore waitSemaphores[] = {this->g_imageAvailableSemaphores[this->g_currentFrame], contextSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                         FGE_CONTEXT_OUTSIDE_RENDER_SCOPE_COMMAND_WAITSTAGE};
    submitInfo.waitSemaphoreCount = 1 + (contextSemaphore == VK_NULL_HANDLE ? 0 : 1);
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = this->getContext().getGraphicsCommandBuffers().size();
    submitInfo.pCommandBuffers = this->getContext().getGraphicsCommandBuffers().data();

    VkSemaphore signalSemaphores[] = {this->g_renderFinishedSemaphores[this->g_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    this->getContext().submit();

    if (vkQueueSubmit(this->getContext().getLogicalDevice().getGraphicQueue(), 1, &submitInfo,
                      this->g_inFlightFences[this->g_currentFrame]) != VK_SUCCESS)
    {
        throw fge::Exception("failed to submit draw command buffer!");
    }

    this->getContext().clearGraphicsCommandBuffers();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {this->g_swapChain.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr; // Optional

    VkResult result = vkQueuePresentKHR(this->getContext().getLogicalDevice().getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->g_framebufferResized)
    {
        this->g_framebufferResized = false;
        this->recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw fge::Exception("failed to present swap chain image!");
    }

    this->g_currentFrame = (this->g_currentFrame + 1) % FGE_MAX_FRAMES_IN_FLIGHT;
}

Vector2u RenderWindow::getSize() const
{
    return static_cast<Vector2u>(this->getContext().getInstance().getWindowSize());
}

bool RenderWindow::isSrgb() const
{
    return false; ///TODO
}

void RenderWindow::setPresentMode(VkPresentModeKHR presentMode)
{
    this->g_presentMode = presentMode;
    this->recreateSwapChain();
}
VkPresentModeKHR RenderWindow::getPresentMode() const
{
    return this->g_presentMode;
}

VkExtent2D RenderWindow::getExtent2D() const
{
    return this->g_swapChain.getSwapChainExtent();
}
VkCommandBuffer RenderWindow::getCommandBuffer() const
{
    return this->g_commandBuffers[this->g_currentFrame];
}
VkRenderPass RenderWindow::getRenderPass() const
{
    return this->g_renderPass;
}

VkCommandBufferInheritanceInfo RenderWindow::getInheritanceInfo(uint32_t imageIndex) const
{
    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = this->g_renderPass;
    inheritanceInfo.subpass = 0;
    inheritanceInfo.framebuffer = this->g_swapChainFramebuffers[imageIndex];
    return inheritanceInfo;
}

uint32_t RenderWindow::getCurrentFrame() const
{
    return this->g_currentFrame;
}

void RenderWindow::onResize()
{
    this->g_framebufferResized = true;
}

void RenderWindow::init()
{
    if (this->g_isCreated)
    {
        this->destroy();
    }
    this->g_isCreated = true;

    this->g_swapChain.create(this->getContext().getInstance().getWindow(), this->getContext().getLogicalDevice(),
                             this->getContext().getPhysicalDevice(), this->getContext().getSurface(),
                             this->g_presentMode);

    this->createRenderPass();

    this->createFramebuffers();

    //create command buffers
    this->getContext().allocateGraphicsCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, this->g_commandBuffers.data(),
                                                      this->g_commandBuffers.size());

    this->createSyncObjects();

    SDL_AddEventWatch(&ResizeCallback, this);
}

void RenderWindow::recreateSwapChain()
{
    auto windowSize = this->getContext().getInstance().getWindowSize();

    SDL_Event event;

    while (windowSize.x == 0 || windowSize.y == 0)
    {
        windowSize = this->getContext().getInstance().getWindowSize();
        SDL_WaitEvent(&event);
    }

    vkDeviceWaitIdle(this->getContext().getLogicalDevice().getDevice());

    for (auto framebuffer: this->g_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(this->getContext().getLogicalDevice().getDevice(), framebuffer, nullptr);
    }
    this->g_swapChainFramebuffers.clear();

    vkDestroyRenderPass(this->getContext().getLogicalDevice().getDevice(), this->g_renderPass, nullptr);
    this->g_swapChain.destroy();

    this->g_swapChain.create(this->getContext().getInstance().getWindow(), this->getContext().getLogicalDevice(),
                             this->getContext().getPhysicalDevice(), this->getContext().getSurface(),
                             this->g_presentMode);
    this->createRenderPass();
    this->createFramebuffers();

    this->_g_forceGraphicPipelineUpdate = true;
}

void RenderWindow::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = this->g_swapChain.getSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

    if (vkCreateRenderPass(this->getContext().getLogicalDevice().getDevice(), &renderPassInfo, nullptr,
                           &this->g_renderPass) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create render pass!");
    }
}

void RenderWindow::createFramebuffers()
{
    this->g_swapChainFramebuffers.resize(this->g_swapChain.getSwapChainImageViews().size());

    for (size_t i = 0; i < this->g_swapChain.getSwapChainImageViews().size(); i++)
    {
        VkImageView attachments[] = {this->g_swapChain.getSwapChainImageViews()[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = this->g_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = this->g_swapChain.getSwapChainExtent().width;
        framebufferInfo.height = this->g_swapChain.getSwapChainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(this->getContext().getLogicalDevice().getDevice(), &framebufferInfo, nullptr,
                                &this->g_swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw fge::Exception("failed to create framebuffer!");
        }
    }
}

void RenderWindow::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(this->getContext().getLogicalDevice().getDevice(), &semaphoreInfo, nullptr,
                              &this->g_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(this->getContext().getLogicalDevice().getDevice(), &semaphoreInfo, nullptr,
                              &this->g_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(this->getContext().getLogicalDevice().getDevice(), &fenceInfo, nullptr,
                          &this->g_inFlightFences[i]) != VK_SUCCESS)
        {
            throw fge::Exception("failed to create semaphores!");
        }
    }
}

} // namespace fge
