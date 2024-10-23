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

#include "FastEngine/manager/texture_manager.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "SDL_image.h"
#include "private/string_hash.hpp"

namespace fge::texture
{

namespace
{

fge::texture::TextureDataPtr _dataTextureBad;
std::unordered_map<std::string, fge::texture::TextureDataPtr, fge::priv::string_hash, std::equal_to<>> _dataTexture;
std::mutex _dataMutex;

} // namespace

void Init()
{
    if (_dataTextureBad == nullptr)
    {
        IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF);

        fge::Surface tmpSurface;

        tmpSurface.create(FGE_TEXTURE_BAD_W, FGE_TEXTURE_BAD_H, FGE_TEXTURE_BAD_COLOR_1);
        for (int y = 0; y < FGE_TEXTURE_BAD_H / 2; ++y)
        {
            for (int x = 0; x < FGE_TEXTURE_BAD_W / 2; ++x)
            {
                tmpSurface.setPixel(x, y, FGE_TEXTURE_BAD_COLOR_2);
            }
        }
        for (int y = FGE_TEXTURE_BAD_H / 2; y < FGE_TEXTURE_BAD_H; ++y)
        {
            for (int x = FGE_TEXTURE_BAD_W / 2; x < FGE_TEXTURE_BAD_W; ++x)
            {
                tmpSurface.setPixel(x, y, FGE_TEXTURE_BAD_COLOR_2);
            }
        }

        _dataTextureBad = std::make_shared<fge::texture::TextureData>();
#ifdef FGE_DEF_SERVER
        _dataTextureBad->_texture = std::make_shared<fge::TextureType>(tmpSurface);
#else
        _dataTextureBad->_texture = std::make_shared<fge::TextureType>(vulkan::GetActiveContext());
        _dataTextureBad->_texture->create(tmpSurface.get());
#endif //FGE_DEF_SERVER
        _dataTextureBad->_valid = false;
    }
}
bool IsInit()
{
    return _dataTextureBad != nullptr;
}
void Uninit()
{
    _dataTexture.clear();
    _dataTextureBad = nullptr;

    IMG_Quit();
}

std::size_t GetTextureSize()
{
    std::scoped_lock<std::mutex> const lck(_dataMutex);
    return _dataTexture.size();
}

std::unique_lock<std::mutex> AcquireLock()
{
    return std::unique_lock<std::mutex>(_dataMutex);
}
fge::texture::TextureDataType::const_iterator IteratorBegin(std::unique_lock<std::mutex> const& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &_dataMutex)
    {
        throw fge::Exception("texture_manager::IteratorBegin : lock is not owned or not my mutex !");
    }
    return _dataTexture.begin();
}
fge::texture::TextureDataType::const_iterator IteratorEnd(std::unique_lock<std::mutex> const& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &_dataMutex)
    {
        throw fge::Exception("texture_manager::IteratorEnd : lock is not owned or not my mutex !");
    }
    return _dataTexture.end();
}

fge::texture::TextureDataPtr const& GetBadTexture()
{
    return _dataTextureBad;
}
fge::texture::TextureDataPtr GetTexture(std::string_view name)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return _dataTextureBad;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataTexture.find(name);

    if (it != _dataTexture.end())
    {
        return it->second;
    }
    return _dataTextureBad;
}

bool Check(std::string_view name)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataTexture.find(name);

    return it != _dataTexture.end();
}

bool LoadFromSurface(std::string_view name, fge::Surface const& surface)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataTexture.find(name);

    if (it != _dataTexture.end())
    {
        return false;
    }

#ifdef FGE_DEF_SERVER
    auto tmpTexture = std::make_shared<fge::TextureType>(surface);
#else
    auto tmpTexture = std::make_shared<fge::TextureType>(vulkan::GetActiveContext());

    if (!tmpTexture->create(surface.get()))
    {
        return false;
    }
#endif //FGE_DEF_SERVER

    fge::texture::TextureDataPtr buff = std::make_shared<fge::texture::TextureData>();
    buff->_texture = std::move(tmpTexture);
    buff->_valid = true;

    _dataTexture[std::string{name}] = std::move(buff);
    return true;
}
bool LoadFromFile(std::string_view name, std::filesystem::path path)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataTexture.find(name);

    if (it != _dataTexture.end())
    {
        return false;
    }

    fge::Surface tmpSurface;

    if (!tmpSurface.loadFromFile(path))
    {
        return false;
    }

    auto tmpTexture = std::make_shared<fge::TextureType>(vulkan::GetActiveContext());

#ifdef FGE_DEF_SERVER
    *tmpTexture = std::move(tmpSurface);
#else
    if (!tmpTexture->create(tmpSurface.get()))
    {
        return false;
    }
#endif //FGE_DEF_SERVER

    fge::texture::TextureDataPtr buff = std::make_shared<fge::texture::TextureData>();
    buff->_texture = std::move(tmpTexture);
    buff->_valid = true;
    buff->_path = std::move(path);

    _dataTexture[std::string{name}] = std::move(buff);
    return true;
}

bool Unload(std::string_view name)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    auto it = _dataTexture.find(name);

    if (it != _dataTexture.end())
    {
        it->second->_valid = false;
        it->second->_texture = _dataTextureBad->_texture;
        _dataTexture.erase(it);
        return true;
    }
    return false;
}
void UnloadAll()
{
    std::scoped_lock<std::mutex> const lck(_dataMutex);

    for (auto& data: _dataTexture)
    {
        data.second->_valid = false;
        data.second->_texture = _dataTextureBad->_texture;
    }
    _dataTexture.clear();
}

bool Push(std::string_view name, fge::texture::TextureDataPtr const& data)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::scoped_lock<std::mutex> const lck(_dataMutex);
    if (fge::texture::Check(name))
    {
        return false;
    }

    _dataTexture.emplace(name, data);
    return true;
}

} // namespace fge::texture
