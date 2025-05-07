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

#include "FastEngine/vulkan/C_shader.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#ifndef FGE_DEF_SERVER
    #include "private/spirv_reflect.h"
#endif
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
        g_spirvBuffer(std::move(r.g_spirvBuffer)),
        g_logicalDevice(r.g_logicalDevice)
#ifndef FGE_DEF_SERVER
        ,
        g_reflectBindings(std::move(r.g_reflectBindings)),
        g_reflectPushConstantRanges(std::move(r.g_reflectPushConstantRanges))
#endif
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
    this->g_spirvBuffer = std::move(r.g_spirvBuffer);
    this->g_logicalDevice = r.g_logicalDevice;

#ifndef FGE_DEF_SERVER
    this->g_reflectBindings = std::move(r.g_reflectBindings);
    this->g_reflectPushConstantRanges = std::move(r.g_reflectPushConstantRanges);
#endif

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
    this->g_spirvBuffer = buffer;

#ifndef FGE_DEF_SERVER
    this->reflect();
#endif

    return true;
}
bool Shader::loadFromFile(LogicalDevice const& logicalDevice, std::filesystem::path const& filepath, Type type)
{
    if (type == Type::SHADER_NONE)
    {
        return false;
    }

    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file)
    {
        return false;
    }

    std::size_t const fileSize = static_cast<std::size_t>(file.tellg());
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
        this->g_spirvBuffer.clear();
#ifndef FGE_DEF_SERVER
        this->g_reflectBindings.clear();
        this->g_reflectPushConstantRanges.clear();
#endif
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

#ifndef FGE_DEF_SERVER
void Shader::retrieveBindings(ReflectSets& buffer) const
{
    if (this->g_shaderModule == VK_NULL_HANDLE)
    {
        return;
    }

    SpvReflectShaderModule module;
    if (spvReflectCreateShaderModule(this->g_spirvBuffer.size() * sizeof(decltype(this->g_spirvBuffer)::value_type),
                                     this->g_spirvBuffer.data(), &module) != SPV_REFLECT_RESULT_SUCCESS)
    {
        return;
    }

    for (uint32_t i = 0; i < module.descriptor_set_count; ++i)
    {
        SpvReflectDescriptorSet const* set = &module.descriptor_sets[i];

        ReflectBindings& bindings = buffer[set->set];

        for (uint32_t j = 0; j < set->binding_count; ++j)
        {
            SpvReflectDescriptorBinding const* binding = set->bindings[j];

            VkDescriptorBindingFlagsEXT const flags = binding->array.dims_count == 1 && binding->array.dims[0] == 0
                                                              ? VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
                                                              : 0;
            uint32_t const count = binding->array.dims_count == 1 && binding->array.dims[0] == 0
                                           ? FGE_SHADER_MAX_BINDING_VARIABLE_DESCRIPTOR_COUNT
                                           : binding->count;

            bool found{false};
            for (auto& bufferBinding: bindings)
            {
                if (bufferBinding.getBinding() == binding->binding)
                { //TODO: for now, if the binding is already in the set, we just overwrite it
                    found = true;
                    bufferBinding = DescriptorSetLayout::Binding(
                            binding->binding, static_cast<VkDescriptorType>(binding->descriptor_type),
                            static_cast<VkShaderStageFlags>(module.shader_stage), count, flags);
                    break;
                }
            }
            if (!found)
            {
                bindings.emplace_back(binding->binding, static_cast<VkDescriptorType>(binding->descriptor_type),
                                      static_cast<VkShaderStageFlags>(module.shader_stage), count, flags);
            }
        }
    }

    spvReflectDestroyShaderModule(&module);
}
std::vector<VkPushConstantRange> Shader::retrievePushConstantRanges() const
{
    if (this->g_shaderModule == VK_NULL_HANDLE)
    {
        return {};
    }

    SpvReflectShaderModule module;
    if (spvReflectCreateShaderModule(this->g_spirvBuffer.size() * sizeof(decltype(this->g_spirvBuffer)::value_type),
                                     this->g_spirvBuffer.data(), &module) != SPV_REFLECT_RESULT_SUCCESS)
    {
        return {};
    }

    std::vector<VkPushConstantRange> result;
    result.reserve(module.push_constant_block_count);

    for (uint32_t i = 0; i < module.push_constant_block_count; ++i)
    {
        SpvReflectBlockVariable const* block = &module.push_constant_blocks[i];
        result.emplace_back(
                VkPushConstantRange{static_cast<VkShaderStageFlags>(module.shader_stage), block->offset, block->size});
    }

    spvReflectDestroyShaderModule(&module);

    return result;
}

void Shader::reflect() const
{
    this->g_reflectBindings.clear();
    this->g_reflectPushConstantRanges.clear();

    if (this->g_shaderModule == VK_NULL_HANDLE)
    {
        return;
    }

    SpvReflectShaderModule module;
    if (spvReflectCreateShaderModule(this->g_spirvBuffer.size() * sizeof(decltype(this->g_spirvBuffer)::value_type),
                                     this->g_spirvBuffer.data(), &module) != SPV_REFLECT_RESULT_SUCCESS)
    {
        return;
    }

    this->g_reflectBindings.reserve(module.descriptor_set_count);

    for (uint32_t i = 0; i < module.descriptor_set_count; ++i)
    {
        auto& bindingResults = this->g_reflectBindings.emplace_back();
        SpvReflectDescriptorSet const* set = &module.descriptor_sets[i];
        for (uint32_t j = 0; j < set->binding_count; ++j)
        {
            SpvReflectDescriptorBinding const* binding = set->bindings[j];

            VkDescriptorBindingFlagsEXT const flags = binding->array.dims_count == 1 && binding->array.dims[0] == 0
                                                              ? VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
                                                              : 0;
            uint32_t const count = binding->array.dims_count == 1 && binding->array.dims[0] == 0
                                           ? FGE_SHADER_MAX_BINDING_VARIABLE_DESCRIPTOR_COUNT
                                           : binding->count;

            bindingResults.emplace_back(binding->binding, static_cast<VkDescriptorType>(binding->descriptor_type),
                                        static_cast<VkShaderStageFlags>(module.shader_stage), count, flags);
        }
    }

    this->g_reflectPushConstantRanges.reserve(module.push_constant_block_count);

    for (uint32_t i = 0; i < module.push_constant_block_count; ++i)
    {
        SpvReflectBlockVariable const* block = &module.push_constant_blocks[i];
        this->g_reflectPushConstantRanges.emplace_back(
                VkPushConstantRange{static_cast<VkShaderStageFlags>(module.shader_stage), block->offset, block->size});
    }

    spvReflectDestroyShaderModule(&module);
}

#endif

} // namespace fge::vulkan