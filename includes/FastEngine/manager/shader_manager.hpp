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

#ifndef _FGE_SHADER_MANAGER_HPP_INCLUDED
#define _FGE_SHADER_MANAGER_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"

#include "FastEngine/vulkan/C_shader.hpp"
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#define FGE_SHADER_BAD ""
#define FGE_SHADER_DEFAULT_VERTEX "FGE:VERTEX"
#define FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT "FGE:NT_FRAG"
#define FGE_SHADER_DEFAULT_FRAGMENT "FGE:FRAG"

namespace fge::shader
{

/**
 * \struct ShaderData
 * \ingroup graphics
 * \brief Structure that safely contains the shader data with his path and validity
 */
struct ShaderData
{
    fge::vulkan::Shader _shader;
    bool _valid;
    std::filesystem::path _path;
};

using ShaderDataPtr = std::shared_ptr<fge::shader::ShaderData>;
using ShaderDataType = std::unordered_map<std::string, fge::shader::ShaderDataPtr>;

enum class ShaderInputTypes
{
    SHADER_HLSL,
    SHADER_GLSL,
    SHADER_SPIRV
};

/**
 * \ingroup graphics
 * @{
 */

/**
 * \brief Initialize the shader manager
 *
 * A bad shader is created with this function, it is used when a shader is not found.
 * You also have to provide a default vertex and fragments shaders.
 *
 * \param vertexPath The default vertex shader
 * \param NoTextureFragmentPath The default fragment shader with no texture attached
 * \param fragmentPath The default fragment shader
 */
FGE_API bool
Init(std::filesystem::path vertexPath, std::filesystem::path NoTextureFragmentPath, std::filesystem::path fragmentPath);
/**
 * \brief Check if the shader manager is initialized
 *
 * \return \b true if the shader manager is initialized, \b false otherwise
 */
FGE_API bool IsInit();
/**
 * \brief Un-initialize the shader manager
 */
FGE_API void Uninit();

/**
 * \brief Get the total number of loaded shaders
 *
 * \return The total number of loaded shaders
 */
FGE_API std::size_t GetShaderSize();

/**
 * \brief Acquire a unique lock, with the shader manager mutex
 *
 * In order to use iterators, you have to acquire a unique lock from this
 * function.
 * The lock is not differed and will lock the mutex.
 *
 * \return A unique lock bound to this mutex
 */
FGE_API std::unique_lock<std::mutex> AcquireLock();
/**
 * \brief Get the begin iterator of the shader manager
 *
 * You have to provide a valid reference to a unique lock acquire with
 * the function AcquireLock().
 * This function will throw if one of this is not respected :
 * - The lock does not owned the associated mutex.
 * - The mutex pointer of the lock does not correspond to this mutex.
 *
 * \param lock A unique lock bound to this mutex
 * \return The begin iterator of the shader manager
 */
FGE_API fge::shader::ShaderDataType::const_iterator IteratorBegin(const std::unique_lock<std::mutex>& lock);
/**
 * \brief Get the end iterator of the shader manager
 *
 * \see fge::texture::IteratorBegin()
 *
 * \param lock A unique lock bound to this mutex
 * \return The begin iterator of the shader manager
 */
FGE_API fge::shader::ShaderDataType::const_iterator IteratorEnd(const std::unique_lock<std::mutex>& lock);

/**
 * \brief Get the bad shader
 *
 * \return The bad shader
 */
FGE_API const fge::shader::ShaderDataPtr& GetBadShader();
/**
 * \brief Get the shader with the given name
 *
 * \param name The name of the shader to get
 * \return The shader with the given name or the bad shader if not found
 */
FGE_API fge::shader::ShaderDataPtr GetShader(std::string_view name);

/**
 * \brief Check if the shader with the given name exist
 *
 * \param name The name of the shader to check
 * \return \b true if the shader exist, \b false otherwise
 */
FGE_API bool Check(std::string_view name);

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
FGE_API bool LoadFromMemory(std::string_view name,
                            const void* data,
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
FGE_API bool LoadFromFile(std::string_view name,
                          std::filesystem::path path,
                          fge::vulkan::Shader::Type type,
                          ShaderInputTypes input,
                          bool debugBuild = false);
/**
 * \brief Unload the shader with the given name
 *
 * \param name The name of the shader to unload
 * \return \b true if the shader was unloaded, \b false otherwise
 */
FGE_API bool Unload(std::string_view name);
/**
 * \brief Unload all shaders
 */
FGE_API void UnloadAll();

/**
 * \brief Add a user handled shader
 *
 * \param name The name of the shader to add
 * \param data The shader data to add
 * \return \b true if the shader was added, \b false otherwise
 */
FGE_API bool Push(std::string_view name, const fge::shader::ShaderDataPtr& data);

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
                                  const char* shaderIn,
                                  int shaderInSize,
                                  std::vector<uint32_t>& shaderOut,
                                  const char* shaderName,
                                  const char* entryPointName,
                                  bool debugBuild,
                                  bool isHlsl);

FGE_API bool SaveSpirVToBinaryFile(const std::vector<uint32_t>& spirv, const std::filesystem::path& path);

/**
 * @}
 */

} // namespace fge::shader

#endif // _FGE_SHADER_MANAGER_HPP_INCLUDED
