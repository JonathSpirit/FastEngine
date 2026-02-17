/*
 * Copyright 2026 Guillaume Guillet
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
{}
TileSet::TileSet(fge::Texture texture, fge::Vector2i const& tileSize) :
        g_texture(std::move(texture)),
        g_tileSize(tileSize)
{
    this->slice();
}
TileSet::TileSet(fge::Texture texture, fge::Vector2i const& tileSize, fge::Vector2i const& offset) :
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
std::string const& TileSet::getName() const
{
    return this->g_name;
}

bool TileSet::valid() const
{
    return this->g_texture.valid();
}

fge::Texture const& TileSet::getTexture() const
{
    return this->g_texture;
}
void TileSet::setTexture(fge::Texture texture)
{
    this->g_texture = std::move(texture);
    this->slice();
}

fge::Vector2i const& TileSet::getTileSize() const
{
    return this->g_tileSize;
}
void TileSet::setTileSize(fge::Vector2i const& tileSize)
{
    this->g_tileSize = tileSize;
    this->slice();
}

fge::Vector2i const& TileSet::getOffset() const
{
    return this->g_offset;
}
void TileSet::setOffset(fge::Vector2i const& offset)
{
    this->g_offset = offset;
    this->slice();
}

std::size_t TileSet::getTileCount() const
{
    return this->g_tiles.size();
}

fge::TileData const* TileSet::getTile(LocalTileId id) const
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

LocalTileId TileSet::getLocalId(fge::Vector2i const& position) const
{
    if (position.x < 0 || position.y < 0)
    {
        return -1;
    }
    fge::LocalTileId localId = position.x + (this->g_columns * position.y);
    return localId < static_cast<fge::LocalTileId>(this->g_tiles.size()) ? localId : -1;
}
LocalTileId TileSet::getLocalId(GlobalTileId gid) const
{
    return this->containsGlobal(gid) ? (gid - this->g_firstGid) : -1;
}
GlobalTileId TileSet::getGlobalId(LocalTileId id) const
{
    return this->containsLocal(id) ? (id + this->g_firstGid) : -1;
}
bool TileSet::containsGlobal(GlobalTileId gid) const
{
    return gid >= this->g_firstGid && gid < (this->g_firstGid + static_cast<GlobalTileId>(this->g_tiles.size()));
}
bool TileSet::containsLocal(LocalTileId id) const
{
    return id >= 0 && id < static_cast<LocalTileId>(this->g_tiles.size());
}
void TileSet::setFirstGid(GlobalTileId gid)
{
    this->g_firstGid = gid;
}
GlobalTileId TileSet::getFirstGid() const
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
    LocalTileId id = 0;
    this->clearTiles();
    if (this->g_texture.valid() && this->g_tileSize.x > 0 && this->g_tileSize.y > 0)
    {
        fge::Vector2i const size = static_cast<fge::Vector2i>(this->g_texture.getTextureSize());
        this->g_columns = (static_cast<int>(size.x) - this->g_offset.x) / this->g_tileSize.x;
        this->g_rows = (static_cast<int>(size.y) - this->g_offset.y) / this->g_tileSize.y;

        for (int y = this->g_offset.y; y < size.y; y += this->g_tileSize.y)
        {
            for (int x = this->g_offset.x; x < size.x; x += this->g_tileSize.x)
            {
                this->pushTile(fge::TileData{id, fge::RectInt{{x, y}, this->g_tileSize}});
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

std::optional<fge::RectInt> TileSet::getTextureRect(LocalTileId id) const
{
    auto it = this->g_tiles.find(id);
    if (it != this->g_tiles.end())
    {
        return it->_rect;
    }
    return std::nullopt;
}
fge::RectInt TileSet::computeTextureRect(LocalTileId id) const
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

    fge::RectInt result;

    result._x = this->g_tileSize.x * (id % this->g_columns);
    result._y = this->g_tileSize.y * (id % this->g_rows);
    result._width = this->g_tileSize.x;
    result._height = this->g_tileSize.y;

    return result;
}

fge::TileSet& TileSet::operator=(fge::Texture texture)
{
    this->g_texture = std::move(texture);
    return *this;
}

void to_json(nlohmann::json& j, fge::TileSet const& p)
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
    for (auto const& tile: p)
    {
        tilesArray.push_back(tile);
    }
}
void from_json(nlohmann::json const& j, fge::TileSet& p)
{
    p.clearTiles();

    p.setFirstGid(j.value<GlobalTileId>("firstgid", 1));
    p.setName(j.value<std::string>("name", {}));

    std::filesystem::path path = j.at("image").get<std::filesystem::path>();
    //Retrieve the filename of the path
    std::string filename = path.stem().string();
    p.setTexture(std::move(filename));

    p.setTileSize({j.at("tileheight").get<int>(), j.at("tilewidth").get<int>()});

    auto itOffset = j.find("tileoffset");
    if (itOffset != j.end())
    {
        p.setOffset({itOffset->value<int>("x", 0), itOffset->value<int>("y", 0)});
    }

    p.slice();

    auto itTiles = j.find("tiles");
    if (itTiles != j.end() && itTiles->is_array())
    {
        for (auto const& tile: *itTiles)
        {
            fge::TileData newTile = tile.get<fge::TileData>();
            auto const* actualTile = p.getTile(newTile._id);
            if (actualTile != nullptr)
            {
                actualTile->_collisionRects = std::move(newTile._collisionRects);
                actualTile->_properties = std::move(newTile._properties);
            }
        }
    }
}

void to_json(nlohmann::json& j, fge::TileData const& p)
{
    j = nlohmann::json{{"id", p._id}};

    if (p._properties.count() != 0)
    {
        auto& propertiesArray = j["properties"];
        for (auto& property: p._properties)
        {
            switch (property.second.getType())
            { //TODO: add a bool type for property
            case fge::Property::Types::PTYPE_INTEGERS:
                propertiesArray.push_back({{"name", property.first},
                                           {"type", "int"},
                                           {"value", property.second.get<fge::PintType>().value_or(0)}});
                break;
            case fge::Property::Types::PTYPE_FLOAT:
            case fge::Property::Types::PTYPE_DOUBLE:
                propertiesArray.push_back({{"name", property.first},
                                           {"type", "float"},
                                           {"value", property.second.get<fge::PfloatType>().value_or(0.0f)}});
                break;
            case fge::Property::Types::PTYPE_STRING:
                propertiesArray.push_back({{"name", property.first},
                                           {"type", "string"},
                                           {"value", property.second.get<std::string>().value_or("")}});
                break;
            default:
                break;
            }
        }
    }
}
void from_json(nlohmann::json const& j, fge::TileData& p)
{
    j.at("id").get_to(p._id);

    //Collision rects
    p._collisionRects.clear();
    auto itObjectGroup = j.find("objectgroup");
    if (itObjectGroup != j.end() && itObjectGroup->is_object())
    {
        auto itObjects = itObjectGroup->find("objects");
        for (auto const& object: *itObjects)
        {
            if (object.is_object())
            {
                fge::RectInt rect;
                rect._x = object.value<int>("x", 0);
                rect._y = object.value<int>("y", 0);
                rect._width = object.value<int>("width", 0);
                rect._height = object.value<int>("height", 0);

                p._collisionRects.push_back(rect);
            }
        }
    }

    //Properties
    p._properties.delAllProperties();
    auto itProperties = j.find("properties");
    if (itProperties != j.end() && itProperties->is_array())
    {
        for (auto const& property: *itProperties)
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

} // namespace fge
