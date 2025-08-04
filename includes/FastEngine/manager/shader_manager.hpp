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

#ifndef _FGE_SHADER_MANAGER_HPP_INCLUDED
#define _FGE_SHADER_MANAGER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/manager/C_baseManager.hpp"
#include "FastEngine/vulkan/C_shader.hpp"

#define FGE_SHADER_BAD FGE_MANAGER_BAD
#define FGE_SHADER_DEFAULT_VERTEX "FGE:VERTEX"
#define FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT "FGE:NT_FRAG"
#define FGE_SHADER_DEFAULT_FRAGMENT "FGE:FRAG"

namespace fge::shader
{

struct DataBlock final : manager::BaseDataBlock<vulkan::Shader>
{};

enum class ShaderInputTypes
{
    SHADER_HLSL,
    SHADER_GLSL,
    SHADER_SPIRV
};

/**
 * \class ShaderManager
 * \ingroup graphics
 * \brief Manage shaders
 *
 * \see TextureManager
 */
class FGE_API ShaderManager final : public manager::BaseManager<vulkan::Shader, DataBlock>
{
public:
    using BaseManager::BaseManager;

    /**
     * \brief Initialize the shader manager
     *
     * A bad shader is created with this function, it is used when a shader is not found.
     * You also have to provide a default vertex and fragments shaders.
     *
     * 3 default shaders are created :
     * FGE_SHADER_DEFAULT_VERTEX : The default vertex shader (in shaderResources.hpp gDefaultVertexShader)
     * FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT : The default fragment shader with no texture attached (in shaderResources.hpp gDefaultFragmentShader)
     * FGE_SHADER_DEFAULT_FRAGMENT : The default fragment shader (in shaderResources.hpp gDefaultFragmentTextureShader)
     *
     * \return \b true if the shader manager is correctly initialized, \b false otherwise
     */
    bool initialize() override;

    /**
     * \brief Load a shader from the memory
     *
     * \warning if you pass a SPIR-V binary data, the size must
     * be a multiple of 4.
     *
     * \param name The name of the shader to load
     * \param data The data address to load
     * \param size The data size to load
     * \param type The shader type
     * \param input The input file type
     * \param debugBuild If the input must be compiled into SPIR-V, then it will be compiled with debug symbols
     * \return \b true if the shader was loaded, \b false otherwise
     */
    bool loadFromMemory(std::string_view name,
                        void const* data,
                        int size,
                        fge::vulkan::Shader::Type type,
                        ShaderInputTypes input,
                        bool debugBuild = false);
    /**
     * \brief Load a shader from a file
     *
     * \param name The name of the shader to load
     * \param path The path of the file to load
     * \param type The shader type
     * \param input The input file type
     * \param debugBuild If the input must be compiled into SPIR-V, then it will be compiled with debug symbols
     * \return \b true if the shader was loaded, \b false otherwise
     */
    bool loadFromFile(std::string_view name,
                      std::filesystem::path path,
                      fge::vulkan::Shader::Type type,
                      ShaderInputTypes input,
                      bool debugBuild = false);
};

/**
 * \ingroup managers
 * \brief The global shader manager
 */
FGE_API extern ShaderManager gManager;

/**
 * \ingroup graphics
 * @{
 */

/**
 * \brief Public API access to glslang
 *
 * \param type The shader type
 * \param shaderIn The shader input
 * \param shaderInSize The shader input size
 * \param shaderOut The shader output (SPIR-V)
 * \param shaderName The shader name
 * \param entryPointName The entry point name
 * \param debugBuild If the shader must be compiled with debug symbols
 * \param isHlsl If the shader is in HLSL language or \b false for GLSL
 * \return \b true if the shader was successfully compiled, \b false otherwise
 */
FGE_API bool CompileAndLinkShader(fge::vulkan::Shader::Type type,
                                  char const* shaderIn,
                                  int shaderInSize,
                                  std::vector<uint32_t>& shaderOut,
                                  char const* shaderName,
                                  char const* entryPointName,
                                  bool debugBuild,
                                  bool isHlsl);

FGE_API bool SaveSpirVToBinaryFile(std::vector<uint32_t> const& spirv, std::filesystem::path const& path);

/**
 * @}
 */

} // namespace fge::shader

#endif // _FGE_SHADER_MANAGER_HPP_INCLUDED
