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

#include "FastEngine/C_texture.hpp"

namespace fge
{

Texture::Texture() :
    g_data(fge::texture::GetBadTexture()),
    g_name(FGE_TEXTURE_BAD)
{
}
Texture::Texture(std::string name) :
    g_data(fge::texture::GetTexture(name)),
    g_name(std::move(name))
{
}
Texture::Texture(const char* name) :
    g_data(fge::texture::GetTexture(name)),
    g_name(name)
{
}
Texture::Texture( fge::texture::TextureDataPtr data ) :
    g_data(std::move(data)),
    g_name(FGE_TEXTURE_BAD)
{
}

void Texture::clear()
{
    this->g_data = fge::texture::GetBadTexture();
    this->g_name = FGE_TEXTURE_BAD;
}
void Texture::refresh()
{
    this->g_data = fge::texture::GetTexture(this->g_name);
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

fge::Texture& Texture::operator =( const std::string& name )
{
    if (this->g_name == name)
    {
        return *this;
    }

    this->g_name = name;
    this->g_data = fge::texture::GetTexture(name);
    return *this;
}
fge::Texture& Texture::operator =( const char* name )
{
    if (this->g_name == name)
    {
        return *this;
    }

    this->g_name = std::move(std::string(name));
    this->g_data = fge::texture::GetTexture(this->g_name);
    return *this;
}
fge::Texture& Texture::operator =( fge::texture::TextureDataPtr data )
{
    this->g_name = FGE_TEXTURE_BAD;
    this->g_data = std::move(data);
    return *this;
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
