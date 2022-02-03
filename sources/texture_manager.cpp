#include "FastEngine/texture_manager.hpp"

namespace fge
{
namespace texture
{

namespace
{

fge::texture::TextureDataPtr __dataTextureBad;
fge::texture::TextureDataType __dataTexture;
std::mutex __dataMutex;

}//end

void FGE_API Init()
{
    if ( __dataTextureBad == nullptr )
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

        __dataTextureBad = std::make_shared<fge::texture::TextureData>();
        __dataTextureBad->_texture = std::make_shared<sf::Texture>();
        __dataTextureBad->_texture->loadFromImage(tmpImage);
        __dataTextureBad->_valid = false;
    }
}
bool FGE_API IsInit()
{
    return __dataTextureBad != nullptr;
}
void FGE_API Uninit()
{
    __dataTexture.clear();
    __dataTextureBad = nullptr;
}

std::size_t FGE_API GetTextureSize()
{
    std::lock_guard<std::mutex> lck(__dataMutex);
    return __dataTexture.size();
}

std::mutex& FGE_API GetMutex()
{
    return __dataMutex;
}

fge::texture::TextureDataType::const_iterator FGE_API GetCBegin()
{
    return __dataTexture.cbegin();
}
fge::texture::TextureDataType::const_iterator FGE_API GetCEnd()
{
    return __dataTexture.cend();
}

const fge::texture::TextureDataPtr& FGE_API GetBadTexture()
{
    return __dataTextureBad;
}
fge::texture::TextureDataPtr FGE_API GetTexture(const std::string& name)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return __dataTextureBad;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::texture::TextureDataType::iterator it = __dataTexture.find(name);

    if (it != __dataTexture.end())
    {
        return it->second;
    }
    return __dataTextureBad;
}

bool FGE_API Check(const std::string& name)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::texture::TextureDataType::iterator it = __dataTexture.find(name);

    if (it != __dataTexture.end())
    {
        return true;
    }
    return false;
}

bool FGE_API LoadFromImage(const std::string& name, const sf::Image& image)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::texture::TextureDataType::iterator it = __dataTexture.find(name);

    if (it != __dataTexture.end())
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

    __dataTexture[name] = std::move(buff);
    return true;
}
bool FGE_API LoadFromFile(const std::string& name, const std::string& path)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::texture::TextureDataType::iterator it = __dataTexture.find(name);

    if (it != __dataTexture.end())
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

    __dataTexture[name] = std::move(buff);
    return true;
}

bool FGE_API Unload(const std::string& name)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    fge::texture::TextureDataType::iterator it = __dataTexture.find(name);

    if (it != __dataTexture.end())
    {
        it->second->_valid = false;
        it->second->_texture = __dataTextureBad->_texture;
        __dataTexture.erase(it);
        return true;
    }
    return false;
}
void FGE_API UnloadAll()
{
    std::lock_guard<std::mutex> lck(__dataMutex);

    for (fge::texture::TextureDataType::iterator it=__dataTexture.begin(); it!=__dataTexture.end(); ++it)
    {
        it->second->_valid = false;
        it->second->_texture = __dataTextureBad->_texture;
    }
    __dataTexture.clear();
}

bool FGE_API Push(const std::string& name, const fge::texture::TextureDataPtr& data)
{
    if (name == FGE_TEXTURE_BAD)
    {
        return false;
    }

    std::lock_guard<std::mutex> lck(__dataMutex);
    if ( fge::texture::Check(name) )
    {
        return false;
    }

    __dataTexture.emplace(name, data);
    return true;
}

}//end texture
}//end fge
