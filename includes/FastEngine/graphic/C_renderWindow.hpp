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

#ifndef _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED

/*
 * Original from : https://github.com/SFML/SFML
 * Copyright (C) 2007-2022 Laurent Gomila
 *
 * Altered/Modified by Guillaume Guillet
 */

#include "FastEngine/fastengine_extern.hpp"
#include <FastEngine/graphic/C_renderTarget.hpp>
#include <FastEngine/vulkan/C_swapChain.hpp>
#include <FastEngine/vulkan/C_uniformBuffer.hpp>
#include <FastEngine/vulkan/C_textureImage.hpp>
#include <FastEngine/vulkan/C_descriptorSet.hpp>
#include <FastEngine/vulkan/C_descriptorSetLayout.hpp>
#include <string>

namespace fge
{

namespace vulkan
{

class Context;

}//end vulkan

class FGE_API RenderWindow : public fge::RenderTarget
{
public:
    RenderWindow();
    explicit RenderWindow(const fge::vulkan::Context& context);
    ~RenderWindow() override;

    void create(const fge::vulkan::Context& context);
    void destroy();

    [[nodiscard]] uint32_t prepareNextFrame(const VkCommandBufferInheritanceInfo* inheritanceInfo) override;
    void beginRenderPass(uint32_t imageIndex) override;
    void draw(const fge::vulkan::GraphicPipeline& graphicPipeline, const RenderStates& states) override;
    void endRenderPass() override;
    void display(uint32_t imageIndex, const VkCommandBuffer* extraCommandBuffer, std::size_t extraCommandBufferSize) override;

    Vector2u getSize() const override;

    bool isSrgb() const override;

    [[nodiscard]] const fge::vulkan::DescriptorSetLayout& getDescriptorSetLayout() const;
    [[nodiscard]] VkCommandBuffer getCommandBuffer() const;
    [[nodiscard]] VkCommandBufferInheritanceInfo getInheritanceInfo(uint32_t imageIndex) const;

    void onResize();

private:
    void init(const fge::vulkan::Context& context);

    void recreateSwapChain();

    void createRenderPass();

    void createFramebuffers();

    void createCommandBuffers();
    void createCommandPool();

    void createSyncObjects();

    const fge::vulkan::Context* g_context = nullptr;

    fge::vulkan::SwapChain g_swapChain;

    VkRenderPass g_renderPass = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> g_swapChainFramebuffers;

    fge::vulkan::DescriptorSetLayout g_descriptorSetLayout;

    VkCommandPool g_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> g_commandBuffers;

    std::vector<VkSemaphore> g_imageAvailableSemaphores;
    std::vector<VkSemaphore> g_renderFinishedSemaphores;
    std::vector<VkFence> g_inFlightFences;

    uint32_t g_currentFrame = 0;

    bool g_framebufferResized = false;
    bool g_forceGraphicPipelineUpdate = false;
    bool g_isCreated = false;
};

}// end fge


#endif // _FGE_GRAPHIC_C_RENDERWINDOW_HPP_INCLUDED
