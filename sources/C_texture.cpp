#include "FastEngine/C_texture.hpp"

namespace fge
{

FGE_API Texture::Texture() :
    g_data(fge::texture::GetBadTexture()),
    g_name(FGE_TEXTURE_BAD)
{
}
FGE_API Texture::Texture( const std::string& name ) :
    g_data(fge::texture::GetTexture(name)),
    g_name(name)
{
}
FGE_API Texture::Texture( const char* name ) :
    g_data(fge::texture::GetTexture(std::string(name))),
    g_name(name)
{
}
FGE_API Texture::Texture( const fge::texture::TextureDataPtr& data ) :
    g_data(data),
    g_name(FGE_TEXTURE_BAD)
{
}

void FGE_API Texture::clear()
{
    this->g_data = fge::texture::GetBadTexture();
    this->g_name = FGE_TEXTURE_BAD;
}

bool FGE_API Texture::valid() const
{
    return this->g_data->_valid;
}

sf::Vector2u FGE_API Texture::getTextureSize() const
{
    return this->g_data->_texture->getSize();
}

const fge::texture::TextureDataPtr& FGE_API Texture::getData() const
{
    return this->g_data;
}
const std::string& FGE_API Texture::getName() const
{
    return this->g_name;
}

void FGE_API Texture::operator =( const std::string& name )
{
    this->g_name = name;
    this->g_data = fge::texture::GetTexture(name);
}
void FGE_API Texture::operator =( const char* name )
{
    this->g_name = std::string(name);
    this->g_data = fge::texture::GetTexture(this->g_name);
}
void FGE_API Texture::operator =( const fge::texture::TextureDataPtr& data )
{
    this->g_name = FGE_TEXTURE_BAD;
    this->g_data = data;
}

FGE_API Texture::operator sf::Texture*()
{
    return this->g_data->_texture.get();
}
FGE_API Texture::operator const sf::Texture*() const
{
    return this->g_data->_texture.get();
}

FGE_API Texture::operator sf::Texture&()
{
    return *this->g_data->_texture;
}
FGE_API Texture::operator const sf::Texture&() const
{
    return *this->g_data->_texture;
}

FGE_API Texture::operator std::string&()
{
    return this->g_name;
}
FGE_API Texture::operator const std::string&() const
{
    return this->g_name;
}

}//end fge
