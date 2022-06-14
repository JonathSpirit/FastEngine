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

#ifndef _FGE_C_TILESET_HPP_INCLUDED
#define _FGE_C_TILESET_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_texture.hpp>

namespace fge
{

class Tileset
{
public:
    Tileset() = default;
    Tileset( const fge::Texture& texture ) :
        g_texture(texture)
    {
    }
    Tileset( const fge::Texture& texture, const sf::Vector2i& tileSize) :
        g_texture(texture),
        g_tileSize(tileSize)
    {
    }
    Tileset( const fge::Texture& texture, const sf::Vector2i& tileSize, const sf::Vector2i& offset) :
        g_texture(texture),
        g_tileSize(tileSize),
        g_offset(offset)
    {
    }

    inline void clear()
    {
        this->g_texture.clear();
        this->g_tileSize.x = 0;
        this->g_tileSize.y = 0;
        this->g_offset.x = 0;
        this->g_offset.y = 0;
    }

    [[nodiscard]] inline bool valid() const
    {
        return this->g_texture.valid();
    }

    inline fge::Texture& getTexture()
    {
        return this->g_texture;
    }
    inline const fge::Texture& getTexture() const
    {
        return this->g_texture;
    }
    inline void setTexture(const fge::Texture& texture)
    {
        this->g_texture = texture;
    }

    inline const sf::Vector2i& getTileSize() const
    {
        return this->g_tileSize;
    }
    inline void setTileSize(const sf::Vector2i& tileSize)
    {
        this->g_tileSize = tileSize;
    }

    inline const sf::Vector2i& getOffset() const
    {
        return this->g_offset;
    }
    inline void setOffset(const sf::Vector2i& offset)
    {
        this->g_offset = offset;
    }

    inline sf::IntRect getTextureRect(const sf::Vector2i& pos) const
    {
        return sf::IntRect(pos.x*this->g_tileSize.x + this->g_offset.x, pos.y*this->g_tileSize.y + this->g_offset.y, this->g_tileSize.x,this->g_tileSize.y);
    }
    inline sf::IntRect getTextureRect(int posX, int posY) const
    {
        return sf::IntRect(posX*this->g_tileSize.x + this->g_offset.x, posY*this->g_tileSize.y + this->g_offset.y, this->g_tileSize.x,this->g_tileSize.y);
    }

    inline void operator =( const fge::Texture& texture )
    {
        this->g_texture = texture;
    }

private:
    fge::Texture g_texture;
    sf::Vector2i g_tileSize;
    sf::Vector2i g_offset;
};

}//end fge

#endif // _FGE_C_TILESET_HPP_INCLUDED
