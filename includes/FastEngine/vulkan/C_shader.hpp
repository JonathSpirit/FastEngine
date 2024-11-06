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

#ifndef _FGE_VULKAN_C_SHADER_HPP_INCLUDED
#define _FGE_VULKAN_C_SHADER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"

#include "C_descriptorSetLayout.hpp"
#include <filesystem>
#include <map>
#include <vector>

#define FGE_SHADER_MAX_BINDING_VARIABLE_DESCRIPTOR_COUNT 64

namespace fge::vulkan
{

class LogicalDevice;

class FGE_API Shader
{
public:
    enum class Type : uint32_t
    {
        SHADER_NONE = 0,
        SHADER_COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT,
        SHADER_VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
        SHADER_FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
        SHADER_GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT
    };

    Shader();
    Shader(Shader const& r) = delete;
    Shader(Shader&& r) noexcept;
    ~Shader();

    Shader& operator=(Shader const& r) = delete;
    Shader& operator=(Shader&& r) noexcept;

    bool loadFromSpirVBuffer(LogicalDevice const& logicalDevice, std::vector<uint32_t> const& buffer, Type type);
    bool loadFromFile(LogicalDevice const& logicalDevice, std::filesystem::path const& filepath, Type type);

    void destroy();

    [[nodiscard]] VkShaderModule getShaderModule() const;
    [[nodiscard]] VkPipelineShaderStageCreateInfo const& getPipelineShaderStageCreateInfo() const;
    [[nodiscard]] Type getType() const;

#ifndef FGE_DEF_SERVER
    using ReflectBindings = std::vector<DescriptorSetLayout::Binding>;
    using ReflectSets = std::map<uint32_t, ReflectBindings, std::less<>>;

    void retrieveBindings(ReflectSets& buffer) const;
    [[nodiscard]] std::vector<VkPushConstantRange> retrievePushConstantRanges() const;
#endif

private:
#ifndef FGE_DEF_SERVER
    void reflect() const;
#endif

    VkShaderModule g_shaderModule;
    VkPipelineShaderStageCreateInfo g_pipelineShaderStageCreateInfo;
    Type g_type{Type::SHADER_NONE};
    std::vector<uint32_t> g_spirvBuffer;

    LogicalDevice const* g_logicalDevice;

#ifndef FGE_DEF_SERVER
    mutable std::vector<std::vector<DescriptorSetLayout::Binding>> g_reflectBindings;
    mutable std::vector<VkPushConstantRange> g_reflectPushConstantRanges;
#endif
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_SHADER_HPP_INCLUDED
