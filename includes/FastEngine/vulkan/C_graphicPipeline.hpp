#ifndef _FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
#define _FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED

#include "C_shader.hpp"
#include "C_viewport.hpp"
#include "C_vertex.hpp"
#include "C_vertexBuffer.hpp"
#include "C_blendMode.hpp"

namespace fge::vulkan
{

class SwapChain;
class LogicalDevice;

class GraphicPipeline
{
public:
    GraphicPipeline();
    GraphicPipeline(const GraphicPipeline& r) = delete;
    GraphicPipeline(GraphicPipeline&& r) noexcept;
    ~GraphicPipeline();

    GraphicPipeline& operator=(const GraphicPipeline& r) = delete;
    GraphicPipeline& operator=(GraphicPipeline&& r) noexcept = delete;

    void updateIfNeeded(const VkExtent2D& extent2D,
                        const LogicalDevice& logicalDevice,
                        const VkDescriptorSetLayout* descriptorSetLayouts,
                        std::size_t descriptorSetLayoutSize,
                        VkRenderPass renderPass,
                        bool force=false);

    void clearShader(Shader::Type type=Shader::Type::SHADER_NONE);
    void setShader(const Shader& shader);
    [[nodiscard]] const Shader* getShader(Shader::Type type=Shader::Type::SHADER_NONE) const;

    void setBlendMode(const BlendMode& blendMode);
    [[nodiscard]] const BlendMode& getBlendMode() const;

    void setPrimitiveTopology(VkPrimitiveTopology topology);

    void setViewport(const VkExtent2D& extent2D);
    void setViewport(const Viewport& viewport);
    [[nodiscard]] const Viewport& getViewport() const;

    void setVertexBuffer(VertexBuffer* vertexBuffer);
    [[nodiscard]] const VertexBuffer* getVertexBuffer() const;
    [[nodiscard]] VertexBuffer* getVertexBuffer();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t defaultVertexCount=3) const;
    void bindDescriptorSets(VkCommandBuffer commandBuffer, const VkDescriptorSet* descriptorSet, uint32_t descriptorCount) const;

    [[nodiscard]] VkPipelineLayout getPipelineLayout() const;
    [[nodiscard]] VkPipeline getPipeline() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice();

    void destroy();

private:
    void cleanPipeline();

    bool g_needUpdate;

    const Shader* g_shaderCompute;
    const Shader* g_shaderVertex;
    const Shader* g_shaderFragment;
    const Shader* g_shaderGeometry;

    VertexBuffer* g_vertexBuffer;

    VkPipelineInputAssemblyStateCreateInfo g_inputAssembly;

    Viewport g_viewport;
    BlendMode g_blendMode;

    VkPipelineLayout g_pipelineLayout;
    VkPipeline g_graphicsPipeline;

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_GRAPHICPIPELINE_HPP_INCLUDED
