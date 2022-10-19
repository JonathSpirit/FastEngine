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

#include "FastEngine/manager/texture_manager.hpp"
#include "private/string_hash.hpp"

namespace fge::texture
{

namespace
{

fge::texture::TextureDataPtr _dataTextureBad;
std::unordered_map<std::string, fge::texture::TextureDataPtr, fge::priv::string_hash, std::equal_to<>> _dataTexture;
std::mutex _dataMutex;

}//end

void Init()
{
    if ( _dataTextureBad == nullptr )
    {
        sf::Image tmpImage;

        tmpImage.create(32,32, sf::Color::Black);
        for (unsigned int y=0; y<16; ++y)
        {
            for (unsigned int x=0; x<16; ++x)
            {
                tmpImage.setPixel(x,y, sf::Color::Magenta);
            }
        }
        for (unsigned int y=16; y<32; ++y)
        {
            for (unsigned int x=16; x<32; ++x)
            {
                tmpImage.setPixel(x,y, sf::Color::Magenta);
            }
        }

        _dataTextureBad = std::make_shared<fge::texture::TextureData>();
#ifdef FGE_DEF_SERVER
        _dataTextureBad->_texture = std::make_shared<fge::TextureType>(tmpImage);
#else
        _dataTextureBad->_texture = std::make_shared<fge::TextureType>();
        _dataTextureBad->_texture->loadFromImage(tmpImage);
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
}

std::size_t GetTextureSize()
{
    std::lock_guard<std::mutex> lck(_dataMutex);
    return _dataTexture.size();
}

std::unique_lock<std::mutex> AcquireLock()
{
    return std::unique_lock<std::mutex>(_dataMutex);
}
fge::texture::TextureDataType::const_iterator IteratorBegin(const std::unique_lock<std::mutex>& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &_dataMutex)
    {
        throw std::runtime_error("texture_manager::IteratorBegin : lock is not owned or not my mutex !");
    }
    return _dataTexture.begin();
}
fge::texture::TextureDataType::const_iterator IteratorEnd(const std::unique_lock<std::mutex>& lock)
{
    if (!lock.owns_lock() || lock.mutex() != &_dataMutex)
    {
        throw std::runtime_error("texture_manager::IteratorEnd : lock is not owned or not my mutex !");
    }
    return _dataTexture.end();
}

const fge::texture::TextureDataPtr& GetBadTexture()
{
    return _dataTextureBad;
}
fge::texture::TextureDataPtr GetTexture(std::string_view name)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return _dataTextureBad;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
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

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataTexture.find(name);

    return it != _dataTexture.end();
}

bool LoadFromImage(std::string_view name, const sf::Image& image)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataTexture.find(name);

    if (it != _dataTexture.end())
    {
        return false;
    }

#ifdef FGE_DEF_SERVER
    auto tmpTexture = std::make_shared<fge::TextureType>(image);
#else
    auto tmpTexture = std::make_shared<fge::TextureType>();

    if ( !tmpTexture->loadFromImage(image) )
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

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataTexture.find(name);

    if (it != _dataTexture.end())
    {
        return false;
    }

    auto tmpTexture = std::make_shared<fge::TextureType>();

    if ( !tmpTexture->loadFromFile(path.string()) )
    {
        return false;
    }

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

    std::lock_guard<std::mutex> lck(_dataMutex);
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
    std::lock_guard<std::mutex> lck(_dataMutex);

    for (auto& data : _dataTexture)
    {
        data.second->_valid = false;
        data.second->_texture = _dataTextureBad->_texture;
    }
    _dataTexture.clear();
}

bool Push(std::string_view name, const fge::texture::TextureDataPtr& data)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    if ( fge::texture::Check(name) )
    {
        return false;
    }

    _dataTexture.emplace(name, data);
    return true;
}

}//end fge::texture
