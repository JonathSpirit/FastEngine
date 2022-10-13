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
#include <FastEngine/C_propertyList.hpp>
#include <set>
#include <optional>
#include <json.hpp>

namespace fge
{

using TileId = int32_t;

struct Tile
{
    TileId _id{0};
    sf::IntRect _rect;
    mutable fge::PropertyList _properties;
};

inline bool operator<(const fge::Tile& l, int r) { return l._id < r; }
inline bool operator<(int l, const fge::Tile& r) { return l < r._id; }
inline bool operator<(const fge::Tile& l, const fge::Tile& r) { return l._id < r._id; }

class FGE_API TileSet
{
public:
    using TileListType = std::set<fge::Tile, std::less<>>;

    TileSet() = default;
    TileSet(fge::Texture texture);
    TileSet(fge::Texture texture, const sf::Vector2i& tileSize);
    TileSet(fge::Texture texture, const sf::Vector2i& tileSize, const sf::Vector2i& offset);

    void clearTiles();

    void setName(std::string name);
    [[nodiscard]] const std::string& getName() const;

    [[nodiscard]] bool valid() const;

    [[nodiscard]] const fge::Texture& getTexture() const;
    void setTexture(fge::Texture texture);

    [[nodiscard]] const sf::Vector2i& getTileSize() const;
    void setTileSize(const sf::Vector2i& tileSize);

    [[nodiscard]] const sf::Vector2i& getOffset() const;
    void setOffset(const sf::Vector2i& offset);

    [[nodiscard]] std::size_t getTileCount() const;

    [[nodiscard]] const fge::Tile* getTile(TileId id) const;

    [[nodiscard]] TileId getLocalId(TileId gid) const;
    [[nodiscard]] bool isGidContained(TileId gid) const;
    void setFirstGid(TileId gid);
    [[nodiscard]] TileId getFirstGid() const;

    [[nodiscard]] TileListType::const_iterator begin() const;
    [[nodiscard]] TileListType::const_iterator end() const;

    void slice();

    [[nodiscard]] std::optional<sf::IntRect> getTextureRect(TileId id) const;
    [[nodiscard]] sf::IntRect computeTextureRect(TileId id) const;

    fge::TileSet& operator =(fge::Texture texture);

private:
    void setTile(fge::Tile tile);
    void pushTile(fge::Tile tile);

    std::string g_name;
    fge::Texture g_texture;
    sf::Vector2i g_tileSize;
    sf::Vector2i g_offset{0,0};
    TileListType g_tiles;
    TileId g_firstGid{1};
};

FGE_API void to_json(nlohmann::json& j, const fge::TileSet& p);
FGE_API void from_json(const nlohmann::json& j, fge::TileSet& p);

FGE_API void to_json(nlohmann::json& j, const fge::Tile& p);
FGE_API void from_json(const nlohmann::json& j, fge::Tile& p);

}//end fge

#endif // _FGE_C_TILESET_HPP_INCLUDED
