/*
 * Copyright 2025 Guillaume Guillet
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
#include "FastEngine/C_alloca.hpp"
#include "FastEngine/extra/extra_function.hpp"
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

RenderWindow::RenderWindow(fge::vulkan::Context const& context, fge::vulkan::SurfaceWindow& surfaceWindow) :
        RenderTarget(context),
        g_surfaceWindow(&surfaceWindow),
        g_commandBuffers({context}),
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
        this->_g_defaultFragmentShader.reset();
        this->_g_defaultVertexShader.reset();
        this->_g_defaultNoTextureFragmentShader.reset();

        VkDevice logicalDevice = this->getContext().getLogicalDevice().getDevice();

        for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(logicalDevice, this->g_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(logicalDevice, this->g_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(logicalDevice, this->g_inFlightFences[i], nullptr);
            this->g_commandBuffers[i].destroy();
        }

        for (auto framebuffer: this->g_swapChainFramebuffers)
        {
            vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
        }

        vkDestroyRenderPass(logicalDevice, this->g_renderPass, nullptr);

        this->g_swapChain.destroy();

        this->g_isCreated = false;
    }
}

uint32_t RenderWindow::prepareNextFrame([[maybe_unused]] VkCommandBufferInheritanceInfo const* inheritanceInfo,
                                        uint64_t timeout_ns)
{
    if (this->g_targetFrameRate != FGE_RENDER_FPS_NOT_LIMITED)
    {
        auto const duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -
                                                                                    this->g_lastFrameTime);
        auto const targetDuration =
                std::chrono::microseconds{std::chrono::microseconds::period::den / this->g_targetFrameRate};

        auto const timeout = std::chrono::microseconds{timeout_ns / 1000};

        if (duration < targetDuration)
        {
            if (duration + timeout >= targetDuration)
            {
                fge::Sleep(targetDuration - duration);
            }
            else
            {
                fge::Sleep(timeout);
                return FGE_RENDER_BAD_IMAGE_INDEX;
            }
        }

        this->g_lastFrameTime = std::chrono::steady_clock::now();
    }

    this->getContext().startMainRenderTarget(*this);
    VkResult result = vkWaitForFences(this->getContext().getLogicalDevice().getDevice(), 1,
                                      &this->g_inFlightFences[this->g_currentFrame], VK_TRUE,
                                      this->g_targetFrameRate != FGE_RENDER_FPS_NOT_LIMITED ? 0 : timeout_ns);
    if (result == VK_TIMEOUT)
    {
        return FGE_RENDER_BAD_IMAGE_INDEX;
    }

    uint32_t imageIndex;
    result = vkAcquireNextImageKHR(this->getContext().getLogicalDevice().getDevice(), this->g_swapChain.getSwapChain(),
                                   UINT64_MAX, this->g_imageAvailableSemaphores[this->g_currentFrame], VK_NULL_HANDLE,
                                   &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        this->recreateSwapChain();
        return FGE_RENDER_BAD_IMAGE_INDEX;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw fge::Exception("failed to acquire swap chain image!");
    }

    // Only reset the fence if we are submitting work
    vkResetFences(this->getContext().getLogicalDevice().getDevice(), 1, &this->g_inFlightFences[this->g_currentFrame]);

    this->g_commandBuffers[this->g_currentFrame].reset();
    this->g_commandBuffers[this->g_currentFrame].begin(0);

    return imageIndex;
}
void RenderWindow::beginRenderPass(uint32_t imageIndex)
{
    this->refreshShaderCache();

    VkClearValue const clearColor = {.color = this->_g_clearColor};

    this->g_commandBuffers[this->g_currentFrame].beginRenderPass(
            this->g_renderPass, this->g_swapChainFramebuffers[imageIndex], this->g_swapChain.getSwapChainExtent(),
            clearColor, VK_SUBPASS_CONTENTS_INLINE);
}
void RenderWindow::endRenderPass()
{
    this->g_commandBuffers[this->g_currentFrame].endRenderPass();

    if (this->_g_forceGraphicPipelineUpdate)
    {
        this->_g_forceGraphicPipelineUpdate = false;
    }

    this->g_commandBuffers[this->g_currentFrame].end();
}
void RenderWindow::display(uint32_t imageIndex)
{
    this->getContext().pushGraphicsCommandBuffer(this->g_commandBuffers[this->g_currentFrame].get());

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    auto contextSemaphore = this->getContext().getIndirectSemaphore();

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
    this->getContext().endMainRenderTarget(*this);
}

[[nodiscard]] Vector2u RenderWindow::getSize() const
{
    return static_cast<Vector2u>(this->g_surfaceWindow->getSize());
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

void RenderWindow::setTargetFrameRate(unsigned int frameRate)
{
    this->g_targetFrameRate = frameRate;
    this->g_lastFrameTime = std::chrono::steady_clock::now();
}
unsigned int RenderWindow::getTargetFrameRate() const
{
    return this->g_targetFrameRate;
}

VkExtent2D RenderWindow::getExtent2D() const
{
    return this->g_swapChain.getSwapChainExtent();
}
fge::vulkan::CommandBuffer& RenderWindow::getCommandBuffer() const
{
    return this->g_commandBuffers[this->g_currentFrame];
}
VkRenderPass RenderWindow::getRenderPass() const
{
    return this->g_renderPass;
}
fge::vulkan::SurfaceWindow& RenderWindow::getSurface() const
{
    return *this->g_surfaceWindow;
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
    this->resetDefaultView();
}

void RenderWindow::init()
{
    if (this->g_isCreated)
    {
        this->destroy();
    }
    this->g_isCreated = true;

    this->g_swapChain.create(this->g_surfaceWindow->getExtent(), this->getContext().getLogicalDevice(),
                             this->getContext().getPhysicalDevice(), this->getContext().getSurface(),
                             this->g_presentMode);

    this->createRenderPass();

    this->createFramebuffers();

    //create command buffers
    for (auto& commandBuffer: this->g_commandBuffers)
    {
        commandBuffer.create(VK_COMMAND_BUFFER_LEVEL_PRIMARY, this->getContext().getGraphicsCommandPool());
    }

    this->createSyncObjects();

    SDL_AddEventWatch(&ResizeCallback, this);
}

void RenderWindow::recreateSwapChain()
{
    auto windowSize = this->g_surfaceWindow->getSize();

    SDL_Event event;

    while (windowSize.x == 0 || windowSize.y == 0)
    {
        windowSize = this->g_surfaceWindow->getSize();
        SDL_WaitEvent(&event);
    }

    vkDeviceWaitIdle(this->getContext().getLogicalDevice().getDevice());

    for (auto framebuffer: this->g_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(this->getContext().getLogicalDevice().getDevice(), framebuffer, nullptr);
    }
    this->g_swapChainFramebuffers.clear();

    vkDestroyRenderPass(this->getContext().getLogicalDevice().getDevice(), this->g_renderPass, nullptr);

    this->g_swapChain.create(this->g_surfaceWindow->getExtent(), this->getContext().getLogicalDevice(),
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
