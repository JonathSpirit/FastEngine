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
        RenderTarget(context)
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

        VkDevice logicalDevice = this->_g_context->getLogicalDevice().getDevice();

        for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(logicalDevice, this->g_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(logicalDevice, this->g_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(logicalDevice, this->g_inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(logicalDevice, this->g_commandPool, nullptr);
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
    vkWaitForFences(this->_g_context->getLogicalDevice().getDevice(), 1, &this->g_inFlightFences[this->g_currentFrame],
                    VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    const VkResult result = vkAcquireNextImageKHR(
            this->_g_context->getLogicalDevice().getDevice(), this->g_swapChain.getSwapChain(), UINT64_MAX,
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
    vkResetFences(this->_g_context->getLogicalDevice().getDevice(), 1, &this->g_inFlightFences[this->g_currentFrame]);

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
}
void RenderWindow::display(uint32_t imageIndex)
{
    if (vkEndCommandBuffer(this->g_commandBuffers[this->g_currentFrame]) != VK_SUCCESS)
    {
        throw fge::Exception("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {this->g_imageAvailableSemaphores[this->g_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    this->g_extraCommandBuffers.push_back(this->g_commandBuffers[this->g_currentFrame]);

    submitInfo.commandBufferCount = this->g_extraCommandBuffers.size();
    submitInfo.pCommandBuffers = this->g_extraCommandBuffers.data();

    VkSemaphore signalSemaphores[] = {this->g_renderFinishedSemaphores[this->g_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(this->_g_context->getLogicalDevice().getGraphicQueue(), 1, &submitInfo,
                      this->g_inFlightFences[this->g_currentFrame]) != VK_SUCCESS)
    {
        throw fge::Exception("failed to submit draw command buffer!");
    }

    this->g_extraCommandBuffers.clear();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {this->g_swapChain.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr; // Optional

    VkResult result = vkQueuePresentKHR(this->_g_context->getLogicalDevice().getPresentQueue(), &presentInfo);

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
    return static_cast<Vector2u>(this->_g_context->getInstance().getWindowSize());
}

bool RenderWindow::isSrgb() const
{
    return false; ///TODO
}

void RenderWindow::pushExtraCommandBuffer(VkCommandBuffer commandBuffer) const
{
    this->g_extraCommandBuffers.push_back(commandBuffer);
}
void RenderWindow::pushExtraCommandBuffer(const std::vector<VkCommandBuffer>& commandBuffers) const
{
    this->g_extraCommandBuffers.insert(this->g_extraCommandBuffers.end(), commandBuffers.begin(), commandBuffers.end());
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

    this->g_swapChain.create(this->_g_context->getInstance().getWindow(), this->_g_context->getLogicalDevice(),
                             this->_g_context->getPhysicalDevice(), this->_g_context->getSurface(),
                             this->g_presentMode);

    this->createRenderPass();

    this->createFramebuffers();

    this->createCommandPool();
    this->createCommandBuffers();

    this->createSyncObjects();

    SDL_AddEventWatch(&ResizeCallback, this);
}

void RenderWindow::recreateSwapChain()
{
    auto windowSize = this->_g_context->getInstance().getWindowSize();

    SDL_Event event;

    while (windowSize.x == 0 || windowSize.y == 0)
    {
        windowSize = this->_g_context->getInstance().getWindowSize();
        SDL_WaitEvent(&event);
    }

    vkDeviceWaitIdle(this->_g_context->getLogicalDevice().getDevice());

    for (auto framebuffer: this->g_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(this->_g_context->getLogicalDevice().getDevice(), framebuffer, nullptr);
    }
    this->g_swapChainFramebuffers.clear();

    vkDestroyRenderPass(this->_g_context->getLogicalDevice().getDevice(), this->g_renderPass, nullptr);
    this->g_swapChain.destroy();

    this->g_swapChain.create(this->_g_context->getInstance().getWindow(), this->_g_context->getLogicalDevice(),
                             this->_g_context->getPhysicalDevice(), this->_g_context->getSurface(),
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

    if (vkCreateRenderPass(this->_g_context->getLogicalDevice().getDevice(), &renderPassInfo, nullptr,
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

        if (vkCreateFramebuffer(this->_g_context->getLogicalDevice().getDevice(), &framebufferInfo, nullptr,
                                &this->g_swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw fge::Exception("failed to create framebuffer!");
        }
    }
}

void RenderWindow::createCommandBuffers()
{
    this->g_commandBuffers.resize(FGE_MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = this->g_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = FGE_MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(this->_g_context->getLogicalDevice().getDevice(), &allocInfo,
                                 this->g_commandBuffers.data()) != VK_SUCCESS)
    {
        throw fge::Exception("failed to allocate command buffers!");
    }
}
void RenderWindow::createCommandPool()
{
    auto queueFamilyIndices =
            this->_g_context->getPhysicalDevice().findQueueFamilies(this->_g_context->getSurface().getSurface());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices._graphicsFamily.value();

    if (vkCreateCommandPool(this->_g_context->getLogicalDevice().getDevice(), &poolInfo, nullptr,
                            &this->g_commandPool) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create command pool!");
    }
}

void RenderWindow::createSyncObjects()
{
    this->g_imageAvailableSemaphores.resize(FGE_MAX_FRAMES_IN_FLIGHT);
    this->g_renderFinishedSemaphores.resize(FGE_MAX_FRAMES_IN_FLIGHT);
    this->g_inFlightFences.resize(FGE_MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(this->_g_context->getLogicalDevice().getDevice(), &semaphoreInfo, nullptr,
                              &this->g_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(this->_g_context->getLogicalDevice().getDevice(), &semaphoreInfo, nullptr,
                              &this->g_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(this->_g_context->getLogicalDevice().getDevice(), &fenceInfo, nullptr,
                          &this->g_inFlightFences[i]) != VK_SUCCESS)
        {
            throw fge::Exception("failed to create semaphores!");
        }
    }
}

} // namespace fge
