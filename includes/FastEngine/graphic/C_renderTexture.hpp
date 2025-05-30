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

#ifndef _FGE_GRAPHIC_C_RENDERTEXTURE_HPP_INCLUDED
#define _FGE_GRAPHIC_C_RENDERTEXTURE_HPP_INCLUDED

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

class FGE_API RenderTexture : public RenderTarget
{
public:
    explicit RenderTexture(glm::vec<2, int> const& size = {1, 1},
                           fge::vulkan::Context const& context = fge::vulkan::GetActiveContext());
    RenderTexture(RenderTexture const& r);
    RenderTexture(RenderTexture&& r) noexcept;
    ~RenderTexture() override;

    RenderTexture& operator=(RenderTexture const& r);
    RenderTexture& operator=(RenderTexture&& r) noexcept;

    void resize(glm::vec<2, int> const& size);
    void destroy() final;

    uint32_t prepareNextFrame(VkCommandBufferInheritanceInfo const* inheritanceInfo, uint64_t timeout_ns) override;
    void beginRenderPass(uint32_t imageIndex) override;
    void endRenderPass() override;
    void display(uint32_t imageIndex) override;

    Vector2u getSize() const override;

    [[nodiscard]] VkExtent2D getExtent2D() const override;
    [[nodiscard]] fge::vulkan::CommandBuffer& getCommandBuffer() const override;
    [[nodiscard]] VkRenderPass getRenderPass() const override;

    [[nodiscard]] fge::vulkan::TextureImage const& getTextureImage() const;

    [[nodiscard]] uint32_t getCurrentFrame() const;

private:
    void init(glm::vec<2, int> const& size);

    void createRenderPass();
    void createFramebuffer();

    fge::vulkan::TextureImage g_textureImage;

    VkRenderPass g_renderPass;

    VkFramebuffer g_framebuffer;

    mutable std::array<fge::vulkan::CommandBuffer, FGE_MAX_FRAMES_IN_FLIGHT> g_commandBuffers;

    uint32_t g_currentFrame;

    bool g_isCreated;
};

} // namespace fge


#endif // _FGE_GRAPHIC_C_RENDERTEXTURE_HPP_INCLUDED
