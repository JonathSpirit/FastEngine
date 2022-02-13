#include "FastEngine/texture_manager.hpp"

namespace fge
{
namespace texture
{

namespace
{

fge::texture::TextureDataPtr _dataTextureBad;
fge::texture::TextureDataType _dataTexture;
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
        _dataTextureBad->_texture = std::make_shared<sf::Texture>();
        _dataTextureBad->_texture->loadFromImage(tmpImage);
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

std::mutex& GetMutex()
{
    return _dataMutex;
}

fge::texture::TextureDataType::const_iterator GetCBegin()
{
    return _dataTexture.cbegin();
}
fge::texture::TextureDataType::const_iterator GetCEnd()
{
    return _dataTexture.cend();
}

const fge::texture::TextureDataPtr& GetBadTexture()
{
    return _dataTextureBad;
}
fge::texture::TextureDataPtr GetTexture(const std::string& name)
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

bool Check(const std::string& name)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(_dataMutex);
    auto it = _dataTexture.find(name);

    if (it != _dataTexture.end())
    {
        return true;
    }
    return false;
}

bool LoadFromImage(const std::string& name, const sf::Image& image)
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

    sf::Texture* tmpTexture = new sf::Texture();

    if ( !tmpTexture->loadFromImage(image) )
    {
        delete tmpTexture;
        return false;
    }

    fge::texture::TextureDataPtr buff = std::make_shared<fge::texture::TextureData>();
    buff->_texture = std::move( std::shared_ptr<sf::Texture>(tmpTexture) );
    buff->_valid = true;

    _dataTexture[name] = std::move(buff);
    return true;
}
bool LoadFromFile(const std::string& name, const std::string& path)
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

    sf::Texture* tmpTexture = new sf::Texture();

    if ( !tmpTexture->loadFromFile(path) )
    {
        delete tmpTexture;
        return false;
    }

    fge::texture::TextureDataPtr buff = std::make_shared<fge::texture::TextureData>();
    buff->_texture = std::move( std::shared_ptr<sf::Texture>(tmpTexture) );
    buff->_valid = true;
    buff->_path = path;

    _dataTexture[name] = std::move(buff);
    return true;
}

bool Unload(const std::string& name)
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

bool Push(const std::string& name, const fge::texture::TextureDataPtr& data)
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

}//end texture
}//end fge
