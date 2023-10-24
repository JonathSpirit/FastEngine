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

#include "FastEngine/manager/shader_manager.hpp"
#include "FastEngine/graphic/shaderResources.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "private/string_hash.hpp"

#define GLSLANG_IS_SHARED_LIBRARY
#include "SPIRV/GlslangToSpv.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"

#ifdef FGE_DEF_DEBUG
    #include <iostream>
#endif
#include <fstream>

namespace fge::shader
{

namespace
{

fge::shader::ShaderDataPtr _dataShaderBad;
std::unordered_map<std::string, fge::shader::ShaderDataPtr, fge::priv::string_hash, std::equal_to<>> _dataShader;
std::mutex _dataMutex;

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
        file.write((char const*) &word, 4);
    }
    file.close();
    return true;
}

bool Init(bool dontLoadDefaultShaders)
{
    if (_dataShaderBad == nullptr)
    {
        if (!glslang::InitializeProcess())
        {
            return false;
        }

        *GetResources() = *GetDefaultResources();

        _dataShaderBad = std::make_shared<fge::shader::ShaderData>();
        _dataShaderBad->_valid = false;

        if (!dontLoadDefaultShaders)
        {
            if (!fge::shader::LoadFromMemory(FGE_SHADER_DEFAULT_VERTEX, fge::res::gDefaultVertexShader,
                                             static_cast<int>(fge::res::gDefaultVertexShaderSize),
                                             fge::vulkan::Shader::Type::SHADER_VERTEX, ShaderInputTypes::SHADER_GLSL))
            {
                Uninit();
                return false;
            }
            if (!fge::shader::LoadFromMemory(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT, fge::res::gDefaultFragmentShader,
                                             static_cast<int>(fge::res::gDefaultFragmentShaderSize),
                                             fge::vulkan::Shader::Type::SHADER_FRAGMENT, ShaderInputTypes::SHADER_GLSL))
            {
                Uninit();
                return false;
            }
            if (!fge::shader::LoadFromMemory(FGE_SHADER_DEFAULT_FRAGMENT, fge::res::gDefaultFragmentTextureShader,
                                             static_cast<int>(fge::res::gDefaultFragmentTextureShaderSize),
                                             fge::vulkan::Shader::Type::SHADER_FRAGMENT, ShaderInputTypes::SHADER_GLSL))
            {
                Uninit();
                return false;
            }
        }

        return true;
    }
    return false;
}
bool IsInit()
{
    return _dataShaderBad != nullptr;
}
void Uninit()
{
    _dataShader.clear();
    _dataShaderBad = nullptr;

    glslang::FinalizeProcess();
}

std::size_t GetShaderSize()
{
    std::scoped_lock<std::mutex> const lck(_dataMutex);
    return _dataShader.size();
}

std::unique_lock<std::mutex> AcquireLock()
{
    return std::unique_lock<std::mutex>(_dataMutex);
}
fge::shader::ShaderDataType::const_iterator IteratorBegin(std::unique_lock<std::mutex> const& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &_dataMutex)
    {
        throw fge::Exception("texture_manager::IteratorBegin : lock is not owned or not my mutex !");
    }
    return _dataShader.begin();
}
fge::shader::ShaderDataType::const_iterator IteratorEnd(std::unique_lock<std::mutex> const& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &_dataMutex)
    {
        throw fge::Exception("texture_manager::IteratorEnd : lock is not owned or not my mutex !");
    }
    return _dataShader.end();
}

fge::shader::ShaderDataPtr const& GetBadShader()
{
    return _dataShaderBad;
}
fge::shader::ShaderDataPtr GetShader(std::string_view name)
{
    if (name == FGE_SHADER_BAD)
    {
        return _dataShaderBad;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataShader.find(name);

    if (it != _dataShader.end())
    {
        return it->second;
    }
    return _dataShaderBad;
}

bool Check(std::string_view name)
{
    if (name == FGE_SHADER_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataShader.find(name);

    return it != _dataShader.end();
}

FGE_API bool LoadFromMemory(std::string_view name,
                            void const* data,
                            int size,
                            fge::vulkan::Shader::Type type,
                            ShaderInputTypes input,
                            bool debugBuild)
{
    if (name == FGE_SHADER_BAD || size <= 0 || data == nullptr)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataShader.find(name);

    if (it != _dataShader.end())
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

    fge::vulkan::Shader tmpShader;
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

        if (!tmpShader.loadFromSpirVBuffer(fge::vulkan::GetActiveContext().getLogicalDevice(), shaderOut, type))
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

        if (!tmpShader.loadFromSpirVBuffer(fge::vulkan::GetActiveContext().getLogicalDevice(), shader, type))
        {
            return false;
        }
    }
    break;
    }

    fge::shader::ShaderDataPtr buff = std::make_shared<fge::shader::ShaderData>();
    buff->_shader = std::move(tmpShader);
    buff->_valid = true;

    _dataShader[std::move(shaderName)] = std::move(buff);
    return true;
}
bool LoadFromFile(std::string_view name,
                  std::filesystem::path path,
                  fge::vulkan::Shader::Type type,
                  ShaderInputTypes input,
                  bool debugBuild)
{
    if (name == FGE_SHADER_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataShader.find(name);

    if (it != _dataShader.end())
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

    fge::vulkan::Shader tmpShader;
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

        if (!tmpShader.loadFromSpirVBuffer(fge::vulkan::GetActiveContext().getLogicalDevice(), shaderOut, type))
        {
            return false;
        }
    }
    break;
    case ShaderInputTypes::SHADER_SPIRV:
        if (!tmpShader.loadFromFile(fge::vulkan::GetActiveContext().getLogicalDevice(), path, type))
        {
            return false;
        }
        break;
    }

    fge::shader::ShaderDataPtr buff = std::make_shared<fge::shader::ShaderData>();
    buff->_shader = std::move(tmpShader);
    buff->_valid = true;
    buff->_path = std::move(path);

    _dataShader[std::move(shaderName)] = std::move(buff);
    return true;
}

bool Unload(std::string_view name)
{
    if (name == FGE_SHADER_BAD || name == FGE_SHADER_DEFAULT_FRAGMENT ||
        name == FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT || name == FGE_SHADER_DEFAULT_VERTEX)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataShader.find(name);

    if (it != _dataShader.end())
    {
        it->second->_valid = false;
        it->second->_shader.destroy();
        _dataShader.erase(it);
        return true;
    }
    return false;
}
void UnloadAll()
{
    std::scoped_lock<std::mutex> const lck(_dataMutex);

    for (auto& data: _dataShader)
    {
        data.second->_valid = false;
        data.second->_shader.destroy();
    }
    _dataShader.clear();
}

bool Push(std::string_view name, fge::shader::ShaderDataPtr const& data)
{
    if (name == FGE_SHADER_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    if (fge::shader::Check(name))
    {
        return false;
    }

    _dataShader.emplace(name, data);
    return true;
}

} // namespace fge::shader
