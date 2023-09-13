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

#ifndef _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/graphic/C_renderTarget.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_descriptorSetLayout.hpp"
#include "FastEngine/vulkan/C_swapChain.hpp"
#include "FastEngine/vulkan/C_textureImage.hpp"
#include "FastEngine/vulkan/C_uniformBuffer.hpp"
#include <string>

namespace fge
{

namespace vulkan
{

class Context;

} // namespace vulkan

class FGE_API RenderWindow : public fge::RenderTarget
{
public:
    explicit RenderWindow(const fge::vulkan::Context& context);
    ~RenderWindow() override;

    void destroy() final;

    [[nodiscard]] uint32_t prepareNextFrame(const VkCommandBufferInheritanceInfo* inheritanceInfo) override;
    void beginRenderPass(uint32_t imageIndex) override;
    void endRenderPass() override;
    void display(uint32_t imageIndex) override;

    Vector2u getSize() const override;

    bool isSrgb() const override;

    void pushExtraCommandBuffer(VkCommandBuffer commandBuffer) const override;
    void pushExtraCommandBuffer(const std::vector<VkCommandBuffer>& commandBuffers) const override;

    void setPresentMode(VkPresentModeKHR presentMode);
    [[nodiscard]] VkPresentModeKHR getPresentMode() const;

    [[nodiscard]] VkExtent2D getExtent2D() const override;
    [[nodiscard]] VkCommandBuffer getCommandBuffer() const override;
    [[nodiscard]] VkRenderPass getRenderPass() const override;

    [[nodiscard]] VkCommandBufferInheritanceInfo getInheritanceInfo(uint32_t imageIndex) const;

    [[nodiscard]] uint32_t getCurrentFrame() const;

    void onResize();

private:
    void init();

    void recreateSwapChain();

    void createRenderPass();

    void createFramebuffers();

    void createCommandBuffers();
    void createCommandPool();

    void createSyncObjects();

    fge::vulkan::SwapChain g_swapChain;

    VkRenderPass g_renderPass = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> g_swapChainFramebuffers;

    VkCommandPool g_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> g_commandBuffers;

    mutable std::vector<VkCommandBuffer> g_extraCommandBuffers;
    mutable std::vector<VkFence> g_extraFences;

    std::vector<VkSemaphore> g_imageAvailableSemaphores;
    std::vector<VkSemaphore> g_renderFinishedSemaphores;
    std::vector<VkFence> g_inFlightFences;

    uint32_t g_currentFrame = 0;

    VkPresentModeKHR g_presentMode = VK_PRESENT_MODE_FIFO_KHR;

    bool g_framebufferResized = false;
    bool g_isCreated = false;
};

} // namespace fge


#endif // _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED
