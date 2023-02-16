/*
 * Copyright 2022 Guillaume Guillet
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
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "private/string_hash.hpp"

namespace fge::shader
{

namespace
{

fge::shader::ShaderDataPtr _dataShaderBad;
std::unordered_map<std::string, fge::shader::ShaderDataPtr, fge::priv::string_hash, std::equal_to<>> _dataShader;
std::mutex _dataMutex;

} // namespace

bool Init(std::filesystem::path vertexPath,
          std::filesystem::path NoTextureFragmentPath,
          std::filesystem::path fragmentPath)
{
    if (_dataShaderBad == nullptr)
    {
        _dataShaderBad = std::make_shared<fge::shader::ShaderData>();
        _dataShaderBad->_valid = false;

        if (!fge::shader::LoadFromFile(FGE_SHADER_DEFAULT_VERTEX, std::move(vertexPath),
                                       fge::vulkan::Shader::Type::SHADER_VERTEX))
        {
            return false;
        }
        if (!fge::shader::LoadFromFile(FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT, std::move(NoTextureFragmentPath),
                                       fge::vulkan::Shader::Type::SHADER_FRAGMENT))
        {
            return false;
        }
        if (!fge::shader::LoadFromFile(FGE_SHADER_DEFAULT_FRAGMENT, std::move(fragmentPath),
                                       fge::vulkan::Shader::Type::SHADER_FRAGMENT))
        {
            return false;
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
}

std::size_t GetShaderSize()
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    return _dataShader.size();
}

std::unique_lock<std::mutex> AcquireLock()
{
    return std::unique_lock<std::mutex>(_dataMutex);
}
fge::shader::ShaderDataType::const_iterator IteratorBegin(const std::unique_lock<std::mutex>& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &_dataMutex)
    {
        throw std::runtime_error("texture_manager::IteratorBegin : lock is not owned or not my mutex !");
    }
    return _dataShader.begin();
}
fge::shader::ShaderDataType::const_iterator IteratorEnd(const std::unique_lock<std::mutex>& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &_dataMutex)
    {
        throw std::runtime_error("texture_manager::IteratorEnd : lock is not owned or not my mutex !");
    }
    return _dataShader.end();
}

const fge::shader::ShaderDataPtr& GetBadShader()
{
    return _dataShaderBad;
}
fge::shader::ShaderDataPtr GetShader(std::string_view name)
{
    if (name == FGE_SHADER_BAD)
    {
        return _dataShaderBad;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
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

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataShader.find(name);

    return it != _dataShader.end();
}

bool LoadFromFile(std::string_view name, std::filesystem::path path, fge::vulkan::Shader::Type type)
{
    if (name == FGE_SHADER_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataShader.find(name);

    if (it != _dataShader.end())
    {
        return false;
    }

    fge::vulkan::Shader tmpShader;

    if (!tmpShader.loadFromFile(fge::vulkan::GlobalContext->getLogicalDevice(), path, type))
    {
        return false;
    }

    fge::shader::ShaderDataPtr buff = std::make_shared<fge::shader::ShaderData>();
    buff->_shader = std::move(tmpShader);
    buff->_valid = true;
    buff->_path = std::move(path);

    _dataShader[std::string{name}] = std::move(buff);
    return true;
}

bool Unload(std::string_view name)
{
    if (name == FGE_SHADER_BAD || name == FGE_SHADER_DEFAULT_FRAGMENT ||
        name == FGE_SHADER_DEFAULT_NOTEXTURE_FRAGMENT || name == FGE_SHADER_DEFAULT_VERTEX)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
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
    std::lock_guard<std::mutex> lck(_dataMutex);

    for (auto& data: _dataShader)
    {
        data.second->_valid = false;
        data.second->_shader.destroy();
    }
    _dataShader.clear();
}

bool Push(std::string_view name, const fge::shader::ShaderDataPtr& data)
{
    if (name == FGE_SHADER_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    if (fge::shader::Check(name))
    {
        return false;
    }

    _dataShader.emplace(name, data);
    return true;
}

} // namespace fge::shader
