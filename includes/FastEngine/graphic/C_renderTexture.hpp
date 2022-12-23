#ifndef _FGE_VULKAN_C_RENDERTEXTURE_HPP_INCLUDED
#define _FGE_VULKAN_C_RENDERTEXTURE_HPP_INCLUDED

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

class RenderTexture : public RenderTarget
{
public:
    RenderTexture();
    explicit RenderTexture(const fge::vulkan::Context& context, const glm::vec<2, int>& size);
    ~RenderTexture() override;

    void create(const fge::vulkan::Context& context, const glm::vec<2, int>& size);
    void destroy();

    [[nodiscard]] uint32_t prepareNextFrame(const VkCommandBufferInheritanceInfo* inheritanceInfo) override;
    void beginRenderPass(uint32_t imageIndex) override;
    void draw(fge::vulkan::GraphicPipeline& graphicPipeline, const RenderStates& states) override;
    void endRenderPass() override;
    void display(uint32_t imageIndex, const VkCommandBuffer* extraCommandBuffer, std::size_t extraCommandBufferSize) override;

    Vector2u getSize() const override;

    bool isSrgb() const override;

    [[nodiscard]] const fge::vulkan::DescriptorSetLayout& getDescriptorSetLayout() const;
    [[nodiscard]] VkCommandBuffer getCommandBuffer() const;
    [[nodiscard]] const fge::vulkan::TextureImage& getTextureImage() const;

private:
    void init(const fge::vulkan::Context& context, const glm::vec<2, int>& size);

    void createRenderPass();

    void createFramebuffer();

    void createCommandBuffer();
    void createCommandPool();

    const fge::vulkan::Context* g_context = nullptr;

    fge::vulkan::TextureImage g_textureImage;

    VkRenderPass g_renderPass = VK_NULL_HANDLE;

    VkFramebuffer g_framebuffer = VK_NULL_HANDLE;

    fge::vulkan::DescriptorSetLayout g_descriptorSetLayout;

    VkCommandPool g_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer g_commandBuffer = VK_NULL_HANDLE;

    bool g_forceGraphicPipelineUpdate = false;
    bool g_isCreated = false;
};

}// end fge


#endif // _FGE_VULKAN_C_RENDERTEXTURE_HPP_INCLUDED
