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

#include "FastEngine/C_tileset.hpp"
#include "FastEngine/arbitraryJsonTypes.hpp"

namespace fge
{

Tileset::Tileset(fge::Texture texture) :
        g_texture(std::move(texture))
{
}
Tileset::Tileset(fge::Texture texture, const sf::Vector2i& tileSize) :
        g_texture(std::move(texture)),
        g_tileSize(tileSize)
{
}
Tileset::Tileset(fge::Texture texture, const sf::Vector2i& tileSize, const sf::Vector2i& offset) :
        g_texture(std::move(texture)),
        g_tileSize(tileSize),
        g_offset(offset)
{
}

void Tileset::clearTiles()
{
    this->g_tiles.clear();
}

void Tileset::setName(std::string name)
{
    this->g_name = std::move(name);
}
const std::string& Tileset::getName() const
{
    return this->g_name;
}

bool Tileset::valid() const
{
    return this->g_texture.valid();
}

const fge::Texture& Tileset::getTexture() const
{
    return this->g_texture;
}
void Tileset::setTexture(fge::Texture texture)
{
    this->g_texture = std::move(texture);
}

const sf::Vector2i& Tileset::getTileSize() const
{
    return this->g_tileSize;
}
void Tileset::setTileSize(const sf::Vector2i& tileSize)
{
    this->g_tileSize = tileSize;
}

const sf::Vector2i& Tileset::getOffset() const
{
    return this->g_offset;
}
void Tileset::setOffset(const sf::Vector2i& offset)
{
    this->g_offset = offset;
}

const fge::Tile* Tileset::getTile(int id) const
{
    auto it = this->g_tiles.find(id);
    return it != this->g_tiles.end() ? &(*it) : nullptr;
}
fge::Tile* Tileset::getTile(int id)
{
    auto it = this->g_tiles.find(id);
    return it != this->g_tiles.end() ? const_cast<fge::Tile*>(&(*it)) : nullptr;
}
void Tileset::setTile(fge::Tile tile)
{
    auto it = this->g_tiles.find(tile);
    if (it != this->g_tiles.end())
    {
        this->g_tiles.erase(it);
    }
    this->g_tiles.insert(std::move(tile));
}

Tileset::TileListType::iterator Tileset::begin()
{
    return this->g_tiles.begin();
}
Tileset::TileListType::const_iterator Tileset::begin() const
{
    return this->g_tiles.begin();
}
Tileset::TileListType::iterator Tileset::end()
{
    return this->g_tiles.end();
}
Tileset::TileListType::const_iterator Tileset::end() const
{
    return this->g_tiles.end();
}

void Tileset::slice()
{
    int id = 0;
    this->g_tiles.clear();
    if (this->g_texture.valid())
    {
        sf::Vector2u size = this->g_texture.getTextureSize();
        for (int x=this->g_offset.x; x<size.x; x+=this->g_tileSize.x)
        {
            for (int y=this->g_offset.y; x<size.y; y+=this->g_tileSize.y)
            {
                this->g_tiles.emplace( fge::Tile{id, sf::IntRect{{x,y},this->g_tileSize}} );
                ++id;
            }
        }
    }
}

sf::IntRect Tileset::getTextureRect(const sf::Vector2i& pos) const
{
    return {pos.x*this->g_tileSize.x + this->g_offset.x,
            pos.y*this->g_tileSize.y + this->g_offset.y,
            this->g_tileSize.x,
            this->g_tileSize.y};
}

fge::Tileset& Tileset::operator =(fge::Texture texture)
{
    this->g_texture = std::move(texture);
    return *this;
}

void to_json(nlohmann::json& j, const fge::Tileset& p)
{
    j["name"] = p.getName();
    j["texture"] = p.getTexture();

    j["size"] = p.getTileSize();
    j["offset"] = p.getOffset();

    auto& tilesArray = j["tiles"];
    for (const auto& tile : p)
    {
        tilesArray.push_back(tile);
    }
}
void from_json(const nlohmann::json& j, fge::Tileset& p)
{
    p.clearTiles();

    if ( j.contains("image") )
    {//Is a "Tiled" json format
        p.setName(j.value<std::string>("name", ""));
        p.setTexture( j.at("image").get<fge::Texture>() );

        p.setTileSize({
            j.at("tileheight").get<int>(),
            j.at("tilewidth").get<int>()
        });

        auto itOffset = j.find("tileoffset");
        if (itOffset != j.end())
        {
            p.setOffset({
                itOffset->value<int>("x", 0),
                itOffset->value<int>("y", 0)
            });
        }

        auto itTiles = j.find("tiles");
        if (itTiles != j.end() && itTiles->is_array())
        {
            for (const auto& tile : *itTiles)
            {
                fge::Tile newTile = tile.get<fge::Tile>();
                p.setTile(newTile);
            }
        }
    }
    else
    {//Is a FGE format
        p.setName(j.value<std::string>("name", ""));
        p.setTexture( j.at("texture").get<fge::Texture>() );

        p.setTileSize(j.at("size").get<sf::Vector2i>());
        p.setOffset(j.value<sf::Vector2i>("offset", {0,0}));

        auto itTiles = j.find("tiles");
        if (itTiles != j.end() && itTiles->is_array())
        {
            for (const auto& tile : *itTiles)
            {
                fge::Tile newTile = tile.get<fge::Tile>();
                p.setTile(newTile);
            }
        }
    }
}

void to_json(nlohmann::json& j, const fge::Tile& p)
{
    j = nlohmann::json{{"id", p._id},
                       {"rect", p._rect}};
}
void from_json(const nlohmann::json& j, fge::Tile& p)
{
    if ( !j.contains("rect") )
    {//Is a "Tiled" json format
        j.at("id").get_to(p._id);

        j.at("width").get_to(p._rect.width);
        j.at("height").get_to(p._rect.height);
        j.at("x").get_to(p._rect.left);
        j.at("y").get_to(p._rect.top);
    }
    else
    {//Is a FGE format
        j.at("id").get_to(p._id);
        j.at("rect").get_to(p._rect);
    }
}

}//end fge
