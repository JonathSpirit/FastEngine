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

#include "FastEngine/vulkan/C_shader.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include <fstream>
#include <vector>

namespace fge::vulkan
{

namespace
{

VkShaderModule CreateShaderModule(std::vector<uint32_t> const& code, VkDevice device)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

} // namespace

Shader::Shader() :
        g_shaderModule(VK_NULL_HANDLE),
        g_pipelineShaderStageCreateInfo(),
        g_type(Shader::Type::SHADER_NONE),
        g_logicalDevice(nullptr)
{
    this->g_pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
}
Shader::Shader(Shader&& r) noexcept :
        g_shaderModule(r.g_shaderModule),
        g_pipelineShaderStageCreateInfo(r.g_pipelineShaderStageCreateInfo),
        g_type(r.g_type),
        g_logicalDevice(r.g_logicalDevice)
{
    r.g_shaderModule = VK_NULL_HANDLE;
    r.g_pipelineShaderStageCreateInfo = {};
    r.g_pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    r.g_type = Shader::Type::SHADER_NONE;
    r.g_logicalDevice = nullptr;
}
Shader::~Shader()
{
    this->destroy();
}

Shader& Shader::operator=(Shader&& r) noexcept
{
    this->destroy();
    this->g_shaderModule = r.g_shaderModule;
    this->g_pipelineShaderStageCreateInfo = r.g_pipelineShaderStageCreateInfo;
    this->g_type = r.g_type;
    this->g_logicalDevice = r.g_logicalDevice;

    r.g_shaderModule = VK_NULL_HANDLE;
    r.g_pipelineShaderStageCreateInfo = {};
    r.g_pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    r.g_type = Shader::Type::SHADER_NONE;
    r.g_logicalDevice = nullptr;
    return *this;
}

bool Shader::loadFromSpirVBuffer(LogicalDevice const& logicalDevice,
                                 std::vector<uint32_t> const& buffer,
                                 Shader::Type type)
{
    this->destroy();

    if (type == Shader::Type::SHADER_NONE)
    {
        return false;
    }

    if (buffer.empty())
    {
        return false;
    }

    this->g_shaderModule = CreateShaderModule(buffer, logicalDevice.getDevice());
    if (this->g_shaderModule == VK_NULL_HANDLE)
    {
        return false;
    }

    this->g_pipelineShaderStageCreateInfo = {};
    this->g_pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    this->g_pipelineShaderStageCreateInfo.stage = static_cast<VkShaderStageFlagBits>(type);
    this->g_pipelineShaderStageCreateInfo.module = this->g_shaderModule;
    this->g_pipelineShaderStageCreateInfo.pName = "main";

    this->g_type = type;

    this->g_logicalDevice = &logicalDevice;

    return true;
}
bool Shader::loadFromFile(LogicalDevice const& logicalDevice, std::filesystem::path const& filepath, Shader::Type type)
{
    if (type == Shader::Type::SHADER_NONE)
    {
        return false;
    }

    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file)
    {
        return false;
    }

    const std::size_t fileSize = static_cast<std::size_t>(file.tellg());
    if (fileSize % sizeof(uint32_t) != 0)
    { //File should be a multiple of sizeof(uint32_t)
        file.close();
        return false;
    }

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
    file.close();

    return this->loadFromSpirVBuffer(logicalDevice, buffer, type);
}

void Shader::destroy()
{
    if (this->g_shaderModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(this->g_logicalDevice->getDevice(), this->g_shaderModule, nullptr);
        this->g_shaderModule = VK_NULL_HANDLE;
        this->g_pipelineShaderStageCreateInfo = {};
        this->g_pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        this->g_type = Shader::Type::SHADER_NONE;
        this->g_logicalDevice = nullptr;
    }
}

VkShaderModule Shader::getShaderModule() const
{
    return this->g_shaderModule;
}
VkPipelineShaderStageCreateInfo const& Shader::getPipelineShaderStageCreateInfo() const
{
    return this->g_pipelineShaderStageCreateInfo;
}
Shader::Type Shader::getType() const
{
    return this->g_type;
}

} // namespace fge::vulkan