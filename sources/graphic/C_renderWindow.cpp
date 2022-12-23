#include <FastEngine/graphic/C_renderWindow.hpp>
#include <FastEngine/vulkan/C_context.hpp>
#include <FastEngine/graphic/C_transform.hpp>
#include <FastEngine/graphic/C_transformable.hpp>
#include <SDL_events.h>
#include <glm/gtc/type_ptr.hpp>

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

}//end

RenderWindow::RenderWindow() = default;

RenderWindow::RenderWindow(const fge::vulkan::Context& context)
{
    this->init(context);
    this->initialize();
}

RenderWindow::~RenderWindow()
{
    this->destroy();
}

void RenderWindow::create(const fge::vulkan::Context& context)
{
    this->init(context);
    this->initialize();
}
void RenderWindow::destroy()
{
    if (this->g_isCreated)
    {
        SDL_DelEventWatch(&ResizeCallback, this);

        VkDevice logicalDevice = this->g_context->getLogicalDevice().getDevice();

        vkDestroySemaphore(logicalDevice, this->g_imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(logicalDevice, this->g_renderFinishedSemaphore, nullptr);
        vkDestroyFence(logicalDevice, this->g_inFlightFence, nullptr);
        vkDestroyCommandPool(logicalDevice, this->g_commandPool, nullptr);
        for (auto framebuffer : this->g_swapChainFramebuffers)
        {
            vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
        }

        vkDestroyRenderPass(logicalDevice, this->g_renderPass, nullptr);

        this->g_descriptorSetLayout.destroy();

        this->g_swapChain.destroy();

        this->g_isCreated = false;
    }
}

uint32_t RenderWindow::prepareNextFrame([[maybe_unused]] const VkCommandBufferInheritanceInfo* inheritanceInfo)
{
    vkWaitForFences(this->g_context->getLogicalDevice().getDevice(), 1, &this->g_inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(this->g_context->getLogicalDevice().getDevice(),
                                            this->g_swapChain.getSwapChain(), UINT64_MAX,
                                            this->g_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        this->recreateSwapChain();
        return BAD_IMAGE_INDEX;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Only reset the fence if we are submitting work
    vkResetFences(this->g_context->getLogicalDevice().getDevice(), 1, &this->g_inFlightFence);

    vkResetCommandBuffer(this->g_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(this->g_commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    return imageIndex;
}
void RenderWindow::beginRenderPass(uint32_t imageIndex)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->g_renderPass;
    renderPassInfo.framebuffer = this->g_swapChainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->g_swapChain.getSwapChainExtent();

    VkClearValue clearColor = {.color=this->_g_clearColor};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(this->g_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void RenderWindow::draw(fge::vulkan::GraphicPipeline& graphicPipeline, const RenderStates& states)
{
    states._transformable->setViewMatrix(this->getView().getTransform());
    states._transformable->updateUniformBuffer(*this->g_context);

    auto windowSize = static_cast<fge::Vector2f>(this->getSize());
    auto factorViewport = this->getView().getFactorViewport();

    const fge::vulkan::Viewport viewport(windowSize.x*factorViewport._x, windowSize.y*factorViewport._y,
                                         windowSize.x*factorViewport._width,windowSize.y*factorViewport._height);
    graphicPipeline.setViewport(viewport);

    VkDescriptorSetLayout layout[] = {this->g_descriptorSetLayout.getLayout(), this->g_context->getDescriptorSetLayout().getLayout()};
    graphicPipeline.updateIfNeeded(this->g_swapChain.getSwapChainExtent(),
                                   this->g_context->getLogicalDevice(),
                                   layout, 2,
                                   this->g_renderPass,
                                   this->g_forceGraphicPipelineUpdate);

    VkDescriptorSet descriptorSets[] = {states._transformable->getDescriptorSet().getDescriptorSet(), states._textureImage->getDescriptorSet().getDescriptorSet()};
    graphicPipeline.bindDescriptorSets(this->g_commandBuffer, descriptorSets, 2);
    graphicPipeline.recordCommandBuffer(this->g_commandBuffer);
}
void RenderWindow::endRenderPass()
{
    vkCmdEndRenderPass(this->g_commandBuffer);

    if (this->g_forceGraphicPipelineUpdate)
    {
        this->g_forceGraphicPipelineUpdate = false;
    }
}
void RenderWindow::display(uint32_t imageIndex, const VkCommandBuffer* extraCommandBuffer, std::size_t extraCommandBufferSize)
{
    if (vkEndCommandBuffer(this->g_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {this->g_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    std::vector<VkCommandBuffer> commandBuffers(1 + extraCommandBufferSize);
    for (std::size_t i=0; i<extraCommandBufferSize; ++i)
    {
        commandBuffers[i] = extraCommandBuffer[i];
    }
    commandBuffers.back() = this->g_commandBuffer;

    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();//&this->g_commandBuffer;

    VkSemaphore signalSemaphores[] = {this->g_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(this->g_context->getLogicalDevice().getGraphicQueue(), 1, &submitInfo, this->g_inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {this->g_swapChain.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr; // Optional

    VkResult result = vkQueuePresentKHR(this->g_context->getLogicalDevice().getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->g_framebufferResized)
    {
        this->g_framebufferResized = false;
        this->recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

Vector2u RenderWindow::getSize() const
{
    return static_cast<Vector2u>(this->g_context->getInstance().getWindowSize());
}

bool RenderWindow::isSrgb() const
{
    return false; ///TODO
}

const fge::vulkan::DescriptorSetLayout& RenderWindow::getDescriptorSetLayout() const
{
    return this->g_descriptorSetLayout;
}
VkCommandBuffer RenderWindow::getCommandBuffer() const
{
    return this->g_commandBuffer;
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

void RenderWindow::onResize()
{
    this->g_framebufferResized = true;
}

void RenderWindow::init(const fge::vulkan::Context& context)
{
    if (this->g_isCreated)
    {
        this->destroy();
    }
    this->g_isCreated = true;

    this->g_context = &context;

    this->g_swapChain.create(context.getInstance().getWindow(),
                             context.getLogicalDevice(),
                             context.getPhysicalDevice(),
                             context.getSurface());

    this->createRenderPass();

    this->g_descriptorSetLayout.create(this->g_context->getLogicalDevice(), {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}
    });

    this->createFramebuffers();

    this->createCommandPool();
    this->createCommandBuffer();

    this->createSyncObjects();

    SDL_AddEventWatch(&ResizeCallback, this);
}

void RenderWindow::recreateSwapChain()
{
    auto windowSize = this->g_context->getInstance().getWindowSize();

    SDL_Event event;

    while (windowSize.x == 0 || windowSize.y == 0)
    {
        windowSize = this->g_context->getInstance().getWindowSize();
        SDL_WaitEvent(&event);
    }

    vkDeviceWaitIdle(this->g_context->getLogicalDevice().getDevice());

    for (auto framebuffer : this->g_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(this->g_context->getLogicalDevice().getDevice(), framebuffer, nullptr);
    }
    this->g_swapChainFramebuffers.clear();

    vkDestroyRenderPass(this->g_context->getLogicalDevice().getDevice(), this->g_renderPass, nullptr);
    this->g_swapChain.destroy();

    this->g_swapChain.create(this->g_context->getInstance().getWindow(),
                             this->g_context->getLogicalDevice(),
                             this->g_context->getPhysicalDevice(),
                             this->g_context->getSurface());
    this->createRenderPass();
    this->createFramebuffers();

    this->g_forceGraphicPipelineUpdate = true;
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

    if (vkCreateRenderPass(this->g_context->getLogicalDevice().getDevice(), &renderPassInfo, nullptr, &this->g_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderWindow::createFramebuffers()
{
    this->g_swapChainFramebuffers.resize(this->g_swapChain.getSwapChainImageViews().size());

    for (size_t i = 0; i < this->g_swapChain.getSwapChainImageViews().size(); i++)
    {
        VkImageView attachments[] = {
                this->g_swapChain.getSwapChainImageViews()[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = this->g_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = this->g_swapChain.getSwapChainExtent().width;
        framebufferInfo.height = this->g_swapChain.getSwapChainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(this->g_context->getLogicalDevice().getDevice(), &framebufferInfo, nullptr, &this->g_swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void RenderWindow::createCommandBuffer()
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
void RenderWindow::createCommandPool()
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

void RenderWindow::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(this->g_context->getLogicalDevice().getDevice(), &semaphoreInfo, nullptr, &this->g_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(this->g_context->getLogicalDevice().getDevice(), &semaphoreInfo, nullptr, &this->g_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(this->g_context->getLogicalDevice().getDevice(), &fenceInfo, nullptr, &this->g_inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }
}

}//end fge
