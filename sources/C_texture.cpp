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

#include "FastEngine/C_texture.hpp"
#include "FastEngine/network/C_packet.hpp"

namespace fge
{

Texture::Texture() :
        g_data(fge::texture::GetBadTexture()),
        g_name(FGE_TEXTURE_BAD)
{}
Texture::Texture(std::string name) :
        g_data(fge::texture::GetTexture(name)),
        g_name(std::move(name))
{}
Texture::Texture(char const* name) :
        g_data(fge::texture::GetTexture(name)),
        g_name(name)
{}
Texture::Texture(fge::texture::TextureDataPtr data) :
        g_data(std::move(data)),
        g_name(FGE_TEXTURE_BAD)
{}

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

fge::Vector2u Texture::getTextureSize() const
{
    return this->g_data->_texture->getSize();
}

fge::texture::TextureDataPtr const& Texture::getData() const
{
    return this->g_data;
}
std::string const& Texture::getName() const
{
    return this->g_name;
}

fge::Texture& Texture::operator=(std::string name)
{
    if (this->g_name == name)
    {
        return *this;
    }

    this->g_name = std::move(name);
    this->g_data = fge::texture::GetTexture(this->g_name);
    return *this;
}
fge::Texture& Texture::operator=(char const* name)
{
    if (this->g_name == name)
    {
        return *this;
    }

    this->g_name = name;
    this->g_data = fge::texture::GetTexture(this->g_name);
    return *this;
}
fge::Texture& Texture::operator=(fge::texture::TextureDataPtr data)
{
    this->g_name = FGE_TEXTURE_BAD;
    this->g_data = std::move(data);
    return *this;
}

fge::TextureType* Texture::retrieve()
{
    return this->g_data->_texture.get();
}
fge::TextureType const* Texture::retrieve() const
{
    return this->g_data->_texture.get();
}

fge::net::Packet const& operator>>(fge::net::Packet const& pck, fge::Texture& data)
{
    std::string textureName;
    pck >> textureName;
    data = std::move(textureName);
    return pck;
}
fge::net::Packet& operator<<(fge::net::Packet& pck, fge::Texture const& data)
{
    return pck << data.getName();
}

void to_json(nlohmann::json& j, fge::Texture const& p)
{
    j = p.getName();
}
void from_json(nlohmann::json const& j, fge::Texture& p)
{
    std::string textureName;
    j.get_to(textureName);
    p = std::move(textureName);
}

} // namespace fge
