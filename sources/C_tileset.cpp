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

namespace fge
{

TileSet::TileSet(fge::Texture texture) :
        g_texture(std::move(texture))
{
}
TileSet::TileSet(fge::Texture texture, const sf::Vector2i& tileSize) :
        g_texture(std::move(texture)),
        g_tileSize(tileSize)
{
    this->slice();
}
TileSet::TileSet(fge::Texture texture, const sf::Vector2i& tileSize, const sf::Vector2i& offset) :
        g_texture(std::move(texture)),
        g_tileSize(tileSize),
        g_offset(offset)
{
    this->slice();
}

void TileSet::clearTiles()
{
    this->g_tiles.clear();
    this->g_columns = 0;
    this->g_rows = 0;
}

void TileSet::setName(std::string name)
{
    this->g_name = std::move(name);
}
const std::string& TileSet::getName() const
{
    return this->g_name;
}

bool TileSet::valid() const
{
    return this->g_texture.valid();
}

const fge::Texture& TileSet::getTexture() const
{
    return this->g_texture;
}
void TileSet::setTexture(fge::Texture texture)
{
    this->g_texture = std::move(texture);
    this->slice();
}

const sf::Vector2i& TileSet::getTileSize() const
{
    return this->g_tileSize;
}
void TileSet::setTileSize(const sf::Vector2i& tileSize)
{
    this->g_tileSize = tileSize;
    this->slice();
}

const sf::Vector2i& TileSet::getOffset() const
{
    return this->g_offset;
}
void TileSet::setOffset(const sf::Vector2i& offset)
{
    this->g_offset = offset;
    this->slice();
}

std::size_t TileSet::getTileCount() const
{
    return this->g_tiles.size();
}

const fge::TileData* TileSet::getTile(TileId id) const
{
    auto it = this->g_tiles.find(id);
    return it != this->g_tiles.end() ? &(*it) : nullptr;
}

void TileSet::setTile(fge::TileData tile)
{
    auto it = this->g_tiles.find(tile);
    if (it != this->g_tiles.end())
    {
        this->g_tiles.erase(it);
    }
    this->g_tiles.insert(std::move(tile));
}
void TileSet::pushTile(fge::TileData tile)
{
    auto it = this->g_tiles.find(tile);
    if (it == this->g_tiles.end())
    {
        this->g_tiles.insert(std::move(tile));
    }
}

TileId TileSet::getLocalId(const sf::Vector2i& position) const
{
    if (position.x<0 || position.y<0)
    {
        return -1;
    }
    fge::TileId localId = position.x + (this->g_columns*position.y);
    return localId<static_cast<fge::TileId>(this->g_tiles.size()) ? localId : -1;
}
TileId TileSet::getLocalId(TileId gid) const
{
    return this->isGidContained(gid) ? (gid - this->g_firstGid) : -1;
}
bool TileSet::isGidContained(TileId gid) const
{
    return gid >= this->g_firstGid && gid < (this->g_firstGid+static_cast<TileId>(this->g_tiles.size()));
}
void TileSet::setFirstGid(TileId gid)
{
    this->g_firstGid = gid;
}
TileId TileSet::getFirstGid() const
{
    return this->g_firstGid;
}

TileSet::TileListType::const_iterator TileSet::begin() const
{
    return this->g_tiles.begin();
}
TileSet::TileListType::const_iterator TileSet::end() const
{
    return this->g_tiles.end();
}

void TileSet::slice()
{
    TileId id = 0;
    this->clearTiles();
    if (this->g_texture.valid() && this->g_tileSize.x>0 && this->g_tileSize.y>0)
    {
        sf::Vector2i size = static_cast<sf::Vector2i>(this->g_texture.getTextureSize());
        this->g_columns = (static_cast<int>(size.x)-this->g_offset.x)/this->g_tileSize.x;
        this->g_rows = (static_cast<int>(size.y)-this->g_offset.y)/this->g_tileSize.y;

        for (int y=this->g_offset.y; y<size.y; y+=this->g_tileSize.y)
        {
            for (int x=this->g_offset.x; x<size.x; x+=this->g_tileSize.x)
            {
                this->pushTile( fge::TileData{id, sf::IntRect{{x,y}, this->g_tileSize}} );
                ++id;
            }
        }
    }
}

int TileSet::getColumns() const
{
    return this->g_columns;
}
int TileSet::getRows() const
{
    return this->g_rows;
}

std::optional<sf::IntRect> TileSet::getTextureRect(TileId id) const
{
    auto it = this->g_tiles.find(id);
    if (it != this->g_tiles.end())
    {
        return it->_rect;
    }
    return std::nullopt;
}
sf::IntRect TileSet::computeTextureRect(TileId id) const
{
    if (id < 0)
    {
        return {};
    }

    if (!this->g_texture.valid())
    {
        return {};
    }

    if (this->g_columns < 0 || this->g_rows < 0)
    {
        return {};
    }

    sf::IntRect result;

    result.left = this->g_tileSize.x * (id%this->g_columns);
    result.top = this->g_tileSize.y * (id%this->g_rows);
    result.width = this->g_tileSize.x;
    result.height = this->g_tileSize.y;

    return result;
}

fge::TileSet& TileSet::operator =(fge::Texture texture)
{
    this->g_texture = std::move(texture);
    return *this;
}

void to_json(nlohmann::json& j, const fge::TileSet& p)
{
    // Tiled info :
    j["type"] = "TileSet";
    j["version"] = "1.9";
    j["tiledversion"] = "1.9.2";

    j["name"] = p.getName();
    j["image"] = p.getTexture();
    j["imagewidth"] = p.getTexture().getTextureSize().y;
    j["imageheight"] = p.getTexture().getTextureSize().y;

    j["tilecount"] = p.getTileCount();
    j["columns"] = p.getColumns();

    j["margin"] = 0; //TODO: margin and spacing
    j["spacing"] = 0;

    j["tileheight"] = p.getTileSize().x;
    j["tilewidth"] = p.getTileSize().x;
    j["offset"] = {{"x", p.getOffset().x}, {"y", p.getOffset().y}};

    auto& tilesArray = j["tiles"];
    for (const auto& tile : p)
    {
        tilesArray.push_back(tile);
    }
}
void from_json(const nlohmann::json& j, fge::TileSet& p)
{
    p.clearTiles();

    p.setFirstGid(j.value<TileId>("firstgid", 1));
    p.setName(j.value<std::string>("name", {}));

    std::filesystem::path path =  j.at("image").get<std::filesystem::path>();
    //Retrieve the filename of the path
    std::string filename = path.stem().string();
    p.setTexture(std::move(filename));

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

    p.slice();

    auto itTiles = j.find("tiles");
    if (itTiles != j.end() && itTiles->is_array())
    {
        for (const auto& tile : *itTiles)
        {
            fge::TileData newTile = tile.get<fge::TileData>();
            const auto* actualTile = p.getTile(newTile._id);
            if (actualTile != nullptr)
            {
                actualTile->_properties = std::move(newTile._properties);
            }
        }
    }
}

void to_json(nlohmann::json& j, const fge::TileData& p)
{
    j = nlohmann::json{{"id", p._id}};

    if (p._properties.getPropertiesSize() != 0)
    {
        auto& propertiesArray = j["properties"];
        for (auto& property : p._properties)
        {
            switch (property.second.getType())
            {//TODO: add a bool type for property
            case Property::PTYPE_INTEGERS:
                propertiesArray.push_back({{"name", property.first},
                                           {"type", "int"},
                                           {"value", property.second.get<fge::PintType>()}});
                break;
            case Property::PTYPE_FLOAT:
            case Property::PTYPE_DOUBLE:
                propertiesArray.push_back({{"name", property.first},
                                           {"type", "float"},
                                           {"value", property.second.get<fge::PfloatType>()}});
                break;
            case Property::PTYPE_STRING:
                propertiesArray.push_back({{"name", property.first},
                                           {"type", "string"},
                                           {"value", property.second.get<std::string>()}});
                break;
            default:
                break;
            }
        }
    }
}
void from_json(const nlohmann::json& j, fge::TileData& p)
{
    j.at("id").get_to(p._id);

    auto itProperties = j.find("properties");
    if (itProperties != j.end() && itProperties->is_array())
    {
        for (const auto& property : *itProperties)
        {
            if (property.is_object())
            {
                auto name = property.value<std::string>("name", {});
                auto type = property.value<std::string>("type", "string");

                if (name.empty())
                {
                    continue;
                }

                if (type == "string")
                {
                    auto value = property.value<std::string>("value", {});
                    p._properties[name] = std::move(value);
                }
                else if (type == "int")
                {
                    auto value = property.value<int>("value", 0);
                    p._properties[name] = value;
                }
                else if (type == "float")
                {
                    auto value = property.value<float>("value", 0.0f);
                    p._properties[name] = value;
                }
                else if (type == "bool")
                {
                    auto value = property.value<bool>("value", false);
                    p._properties[name] = value;
                }
            }
        }
    }
}

}//end fge
