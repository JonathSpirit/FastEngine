#include "FastEngine/C_texture.hpp"

namespace fge
{

Texture::Texture() :
    g_data(fge::texture::GetBadTexture()),
    g_name(FGE_TEXTURE_BAD)
{
}
Texture::Texture( const std::string& name ) :
    g_data(fge::texture::GetTexture(name)),
    g_name(name)
{
}
Texture::Texture( const char* name ) :
    g_data(fge::texture::GetTexture(std::string(name))),
    g_name(name)
{
}
Texture::Texture( const fge::texture::TextureDataPtr& data ) :
    g_data(data),
    g_name(FGE_TEXTURE_BAD)
{
}

void Texture::clear()
{
    this->g_data = fge::texture::GetBadTexture();
    this->g_name = FGE_TEXTURE_BAD;
}

bool Texture::valid() const
{
    return this->g_data->_valid;
}

sf::Vector2u Texture::getTextureSize() const
{
    return this->g_data->_texture->getSize();
}

const fge::texture::TextureDataPtr& Texture::getData() const
{
    return this->g_data;
}
const std::string& Texture::getName() const
{
    return this->g_name;
}

void Texture::operator =( const std::string& name )
{
    this->g_name = name;
    this->g_data = fge::texture::GetTexture(name);
}
void Texture::operator =( const char* name )
{
    this->g_name = std::string(name);
    this->g_data = fge::texture::GetTexture(this->g_name);
}
void Texture::operator =( const fge::texture::TextureDataPtr& data )
{
    this->g_name = FGE_TEXTURE_BAD;
    this->g_data = data;
}

Texture::operator sf::Texture*()
{
    return this->g_data->_texture.get();
}
Texture::operator const sf::Texture*() const
{
    return this->g_data->_texture.get();
}

Texture::operator sf::Texture&()
{
    return *this->g_data->_texture;
}
Texture::operator const sf::Texture&() const
{
    return *this->g_data->_texture;
}

Texture::operator std::string&()
{
    return this->g_name;
}
Texture::operator const std::string&() const
{
    return this->g_name;
}

}//end fge
