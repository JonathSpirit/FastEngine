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

#ifndef _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/graphic/C_renderTarget.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/C_descriptorSetLayout.hpp"
#include "FastEngine/vulkan/C_swapChain.hpp"
#include "FastEngine/vulkan/C_textureImage.hpp"
#include "FastEngine/vulkan/C_uniformBuffer.hpp"
#include <array>
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
    explicit RenderWindow(fge::vulkan::Context const& context);
    ~RenderWindow() override;

    void destroy() final;

    [[nodiscard]] uint32_t prepareNextFrame(VkCommandBufferInheritanceInfo const* inheritanceInfo) override;
    void beginRenderPass(uint32_t imageIndex) override;
    void endRenderPass() override;
    void display(uint32_t imageIndex) override;

    Vector2u getSize() const override;

    void setPresentMode(VkPresentModeKHR presentMode);
    [[nodiscard]] VkPresentModeKHR getPresentMode() const;

    [[nodiscard]] VkExtent2D getExtent2D() const override;
    [[nodiscard]] fge::vulkan::CommandBuffer& getCommandBuffer() const override;
    [[nodiscard]] VkRenderPass getRenderPass() const override;

    [[nodiscard]] VkCommandBufferInheritanceInfo getInheritanceInfo(uint32_t imageIndex) const;

    [[nodiscard]] uint32_t getCurrentFrame() const;

    void onResize();

private:
    void init();

    void recreateSwapChain();

    void createRenderPass();

    void createFramebuffers();

    void createSyncObjects();

    fge::vulkan::SwapChain g_swapChain;

    VkRenderPass g_renderPass = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> g_swapChainFramebuffers;

    mutable std::array<fge::vulkan::CommandBuffer, FGE_MAX_FRAMES_IN_FLIGHT> g_commandBuffers;

    std::array<VkSemaphore, FGE_MAX_FRAMES_IN_FLIGHT> g_imageAvailableSemaphores;
    std::array<VkSemaphore, FGE_MAX_FRAMES_IN_FLIGHT> g_renderFinishedSemaphores;
    std::array<VkFence, FGE_MAX_FRAMES_IN_FLIGHT> g_inFlightFences;

    uint32_t g_currentFrame = 0;

    VkPresentModeKHR g_presentMode = VK_PRESENT_MODE_FIFO_KHR;

    bool g_framebufferResized = false;
    bool g_isCreated = false;
};

} // namespace fge


#endif // _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED
