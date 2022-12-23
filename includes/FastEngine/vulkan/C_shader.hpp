#ifndef _FGE_VULKAN_C_SHADER_HPP_INCLUDED
#define _FGE_VULKAN_C_SHADER_HPP_INCLUDED

#include <filesystem>
#include "volk.h"

namespace fge::vulkan
{

class LogicalDevice;

class Shader
{
public:
    enum class Type : uint32_t
    {
        SHADER_NONE     = 0,
        SHADER_COMPUTE  = VK_SHADER_STAGE_COMPUTE_BIT,
        SHADER_VERTEX   = VK_SHADER_STAGE_VERTEX_BIT,
        SHADER_FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
        SHADER_GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT
    };

    Shader();
    Shader(const Shader& r) = delete;
    Shader(Shader&& r) noexcept;
    ~Shader();

    Shader& operator=(const Shader& r) = delete;
    Shader& operator=(Shader&& r) noexcept = delete;

    bool loadFromFile(const LogicalDevice& logicalDevice, const std::filesystem::path& filepath, Shader::Type type);

    void destroy();

    [[nodiscard]] VkShaderModule getShaderModule() const;
    [[nodiscard]] const VkPipelineShaderStageCreateInfo& getPipelineShaderStageCreateInfo() const;
    [[nodiscard]] Shader::Type getType() const;

private:
    VkShaderModule g_shaderModule;
    VkPipelineShaderStageCreateInfo g_pipelineShaderStageCreateInfo;
    Shader::Type g_type{Shader::Type::SHADER_NONE};

    const LogicalDevice* g_logicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_SHADER_HPP_INCLUDED
