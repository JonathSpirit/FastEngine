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

#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/graphic/shaderResources.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"

#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"

#ifdef FGE_DEF_DEBUG
    #include <iostream>
#endif
#include <fstream>

namespace fge::shader
{

namespace
{

#ifdef FGE_DEF_DEBUG
bool IsInfoLogEmpty(char const* infoLog)
{
    if (infoLog == nullptr)
    {
        return true;
    }
    return *infoLog == '\0';
}
#endif

bool CompileAndLinkShader(EShLanguage stage,
                          char const* shaderIn,
                          int shaderInSize,
                          std::vector<uint32_t>& shaderOut,
                          char const* shaderName,
                          char const* entryPointName,
                          bool debugBuild,
                          bool isHlsl)
{
    if (shaderIn == nullptr || shaderInSize <= 0 || shaderName == nullptr || entryPointName == nullptr)
    {
        return false;
    }

    glslang::TShader shader(stage); //Must respect this order
    glslang::TProgram program;

    shader.setStringsWithLengthsAndNames(&shaderIn, &shaderInSize, &shaderName, 1);
    shader.setEntryPoint(entryPointName);

    int const defaultVersion = 110;

    if (isHlsl)
    {
        shader.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, defaultVersion);
    }
    else
    {
        shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, defaultVersion);
    }
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

    if (!shader.parse(GetResources(), defaultVersion, false, EShMsgDefault))
    {
#ifdef FGE_DEF_DEBUG
        std::cout << "Can't parse shader <" << shaderName << "> !" << std::endl;
        if (!IsInfoLogEmpty(shader.getInfoLog()))
        {
            std::cout << "\t[Info log]\n" << shader.getInfoLog() << std::endl;
        }
        if (!IsInfoLogEmpty(shader.getInfoDebugLog()))
        {
            std::cout << "\t[Info debug log]\n" << shader.getInfoDebugLog() << std::endl;
        }
#endif
        return false;
    }

    program.addShader(&shader);

#ifdef FGE_DEF_DEBUG
    if (!IsInfoLogEmpty(shader.getInfoLog()))
    {
        std::cout << "\t[Info log] for <" << shaderName << ">\n" << shader.getInfoLog() << std::endl;
    }
    if (!IsInfoLogEmpty(shader.getInfoDebugLog()))
    {
        std::cout << "\t[Info debug log] for <" << shaderName << ">\n" << shader.getInfoDebugLog() << std::endl;
    }
#endif

    // Link
    if (!program.link(EShMsgDefault))
    {
#ifdef FGE_DEF_DEBUG
        std::cout << "Can't link shader <" << shaderName << "> !" << std::endl;
        if (!IsInfoLogEmpty(program.getInfoLog()))
        {
            std::cout << "\t[Program info log]\n" << program.getInfoLog() << std::endl;
        }
        if (!IsInfoLogEmpty(program.getInfoDebugLog()))
        {
            std::cout << "\t[Program info debug log]\n" << program.getInfoDebugLog() << std::endl;
        }
#endif
        return false;
    }

#ifdef FGE_DEF_DEBUG
    if (!IsInfoLogEmpty(program.getInfoLog()))
    {
        std::cout << "\t[Program info log] for <" << shaderName << ">\n" << program.getInfoLog() << std::endl;
    }
    if (!IsInfoLogEmpty(program.getInfoDebugLog()))
    {
        std::cout << "\t[Program info debug log] for <" << shaderName << ">\n"
                  << program.getInfoDebugLog() << std::endl;
    }
#endif

    // Dump SPIR-V
    auto* intermediate = program.getIntermediate(stage);
    if (intermediate == nullptr)
    {
        return false;
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;

    spvOptions.validate = true;

    if (debugBuild)
    {
        spvOptions.generateDebugInfo = true;
        spvOptions.disableOptimizer = true;
        spvOptions.stripDebugInfo = false;
    }
    else
    {
        spvOptions.generateDebugInfo = false;
        spvOptions.disableOptimizer = false;
        spvOptions.stripDebugInfo = true;
    }

    glslang::GlslangToSpv(*intermediate, shaderOut, &logger, &spvOptions);

#ifdef FGE_DEF_DEBUG
    auto loggerMessage = logger.getAllMessages();
    if (!IsInfoLogEmpty(loggerMessage.c_str()))
    {
        std::cout << "\t[Logger output] for <" << shaderName << ">\n" << loggerMessage << std::endl;
    }
#endif

    return true;
}

} // namespace

bool CompileAndLinkShader(fge::vulkan::Shader::Type type,
                          char const* shaderIn,
                          int shaderInSize,
                          std::vector<uint32_t>& shaderOut,
                          char const* shaderName,
                          char const* entryPointName,
                          bool debugBuild,
                          bool isHlsl)
{ //Public API of CompileAndLinkShader
    EShLanguage stage{};

    switch (type)
    {
    case vulkan::Shader::Type::SHADER_NONE:
        return false;
    case vulkan::Shader::Type::SHADER_COMPUTE:
        stage = EShLanguage::EShLangCompute;
        break;
    case vulkan::Shader::Type::SHADER_VERTEX:
        stage = EShLanguage::EShLangVertex;
        break;
    case vulkan::Shader::Type::SHADER_FRAGMENT:
        stage = EShLanguage::EShLangFragment;
        break;
    case vulkan::Shader::Type::SHADER_GEOMETRY:
        stage = EShLanguage::EShLangGeometry;
        break;
    }

    return CompileAndLinkShader(stage, shaderIn, shaderInSize, shaderOut, shaderName, entryPointName, debugBuild,
                                isHlsl);
}

bool SaveSpirVToBinaryFile(std::vector<uint32_t> const& spirv, std::filesystem::path const& path)
{
    std::ofstream file(path, std::ios::binary | std::ios::out);

    if (!file)
    {
        return false;
    }

    for (std::size_t i = 0; i < spirv.size(); ++i)
    {
        uint32_t word = spirv[i];
        file.write(reinterpret_cast<char const*>(&word), 4);
    }
    file.close();
    return true;
}

bool ShaderManager::initialize()
{
    if (this->isInitialized())
    {
        return true;
    }

    if (!glslang::InitializeProcess())
    {
        return false;
    }

    *GetResources() = *GetDefaultResources();

    this->_g_badElement = std::make_shared<DataBlockPointer::element_type>();
    this->_g_badElement->_valid = false;

    if (!this->loadFromMemory(FGE_SHADER_DEFAULT_VERTEX, fge::res::gDefaultVertexShader,
                              static_cast<int>(fge::res::gDefaultVertexShaderSize),
                              fge::vulkan::Shader::Type::SHADER_VERTEX, ShaderInputTypes::SHADER_GLSL))
    {
        this->unloadAll();
        this->_g_badElement.reset();
        return false;
    }
    if (!this->loadFromMemory(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT, fge::res::gDefaultFragmentShader,
                              static_cast<int>(fge::res::gDefaultFragmentShaderSize),
                              fge::vulkan::Shader::Type::SHADER_FRAGMENT, ShaderInputTypes::SHADER_GLSL))
    {
        this->unloadAll();
        this->_g_badElement.reset();
        return false;
    }
    if (!this->loadFromMemory(FGE_SHADER_DEFAULT_FRAGMENT, fge::res::gDefaultFragmentTextureShader,
                              static_cast<int>(fge::res::gDefaultFragmentTextureShaderSize),
                              fge::vulkan::Shader::Type::SHADER_FRAGMENT, ShaderInputTypes::SHADER_GLSL))
    {
        this->unloadAll();
        this->_g_badElement.reset();
        return false;
    }

    return true;
}
bool ShaderManager::isInitialized()
{
    return this->_g_badElement != nullptr;
}
void ShaderManager::uninitialize()
{
    if (!this->isInitialized())
    {
        return;
    }

    this->unloadAll();
    this->_g_badElement.reset();
}

bool ShaderManager::loadFromMemory(std::string_view name,
                                   void const* data,
                                   int size,
                                   vulkan::Shader::Type type,
                                   ShaderInputTypes input,
                                   bool debugBuild)
{
    if (name.empty() || size <= 0 || data == nullptr)
    {
        return false;
    }

    EShLanguage stage{};

    switch (type)
    {
    case vulkan::Shader::Type::SHADER_NONE:
        return false;
    case vulkan::Shader::Type::SHADER_COMPUTE:
        stage = EShLanguage::EShLangCompute;
        break;
    case vulkan::Shader::Type::SHADER_VERTEX:
        stage = EShLanguage::EShLangVertex;
        break;
    case vulkan::Shader::Type::SHADER_FRAGMENT:
        stage = EShLanguage::EShLangFragment;
        break;
    case vulkan::Shader::Type::SHADER_GEOMETRY:
        stage = EShLanguage::EShLangGeometry;
        break;
    }

    DataBlock::DataPointer tmpShader = std::make_shared<DataType>();
    std::string shaderName(name);
    bool isHlsl = false;

    switch (input)
    {
    case ShaderInputTypes::SHADER_HLSL:
        isHlsl = true;
        [[fallthrough]];
    case ShaderInputTypes::SHADER_GLSL:
    {
        std::vector<uint32_t> shaderOut;

        if (!CompileAndLinkShader(stage, reinterpret_cast<char const*>(data), size, shaderOut, shaderName.c_str(),
                                  "main", debugBuild, isHlsl))
        {
            return false;
        }

        if (!tmpShader->loadFromSpirVBuffer(fge::vulkan::GetActiveContext().getLogicalDevice(), shaderOut, type))
        {
            return false;
        }
    }
    break;
    case ShaderInputTypes::SHADER_SPIRV:
    {
        if (size % sizeof(uint32_t) != 0)
        {
            return false;
        }

        std::vector<uint32_t> const shader(reinterpret_cast<uint32_t const*>(data),
                                           reinterpret_cast<uint32_t const*>(data) + size / 4);

        if (!tmpShader->loadFromSpirVBuffer(fge::vulkan::GetActiveContext().getLogicalDevice(), shader, type))
        {
            return false;
        }
    }
    break;
    }

    DataBlockPointer block = std::make_shared<DataBlockPointer::element_type>();
    block->_ptr = std::move(tmpShader);
    block->_valid = true;

    return this->push(name, std::move(block));
}

bool ShaderManager::loadFromFile(std::string_view name,
                                 std::filesystem::path path,
                                 fge::vulkan::Shader::Type type,
                                 ShaderInputTypes input,
                                 bool debugBuild)
{
    if (name.empty())
    {
        return false;
    }

    EShLanguage stage{};

    switch (type)
    {
    case vulkan::Shader::Type::SHADER_NONE:
        return false;
    case vulkan::Shader::Type::SHADER_COMPUTE:
        stage = EShLanguage::EShLangCompute;
        break;
    case vulkan::Shader::Type::SHADER_VERTEX:
        stage = EShLanguage::EShLangVertex;
        break;
    case vulkan::Shader::Type::SHADER_FRAGMENT:
        stage = EShLanguage::EShLangFragment;
        break;
    case vulkan::Shader::Type::SHADER_GEOMETRY:
        stage = EShLanguage::EShLangGeometry;
        break;
    }

    DataBlock::DataPointer tmpShader = std::make_shared<DataType>();
    std::string shaderName(name);
    bool isHlsl = false;

    switch (input)
    {
    case ShaderInputTypes::SHADER_HLSL:
        isHlsl = true;
        [[fallthrough]];
    case ShaderInputTypes::SHADER_GLSL:
    {
        std::ifstream test(path, std::ios_base::binary);

        if (!test)
        {
            return false;
        }

        test.seekg(0, std::ios::end);
        int const size = static_cast<int>(test.tellg());

        std::vector<char> buffer(size);

        test.seekg(0);
        test.read(buffer.data(), static_cast<std::streamsize>(size));
        test.close();

        std::vector<uint32_t> shaderOut;

        if (!CompileAndLinkShader(stage, buffer.data(), size, shaderOut, shaderName.c_str(), "main", debugBuild,
                                  isHlsl))
        {
            return false;
        }

        if (!tmpShader->loadFromSpirVBuffer(fge::vulkan::GetActiveContext().getLogicalDevice(), shaderOut, type))
        {
            return false;
        }
    }
    break;
    case ShaderInputTypes::SHADER_SPIRV:
        if (!tmpShader->loadFromFile(fge::vulkan::GetActiveContext().getLogicalDevice(), path, type))
        {
            return false;
        }
        break;
    }

    DataBlockPointer block = std::make_shared<DataBlockPointer::element_type>();
    block->_ptr = std::move(tmpShader);
    block->_valid = true;
    block->_path = std::move(path);

    return this->push(name, std::move(block));
}

ShaderManager gManager;

} // namespace fge::shader
