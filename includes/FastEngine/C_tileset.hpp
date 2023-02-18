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

#include "FastEngine/fastengine_extern.hpp"
#include "C_rect.hpp"
#include "FastEngine/C_propertyList.hpp"
#include "FastEngine/C_texture.hpp"
#include "json.hpp"
#include <optional>
#include <set>

namespace fge
{

using TileId = int32_t;

/**
 * \struct TileData
 * \brief A tile structure that contain mostly the texture rectangle and data
 * \ingroup graphics
 *
 * This structure is compatible with the "Tiled" map editor. The id is the local id of the tile in the tileset.
 */
struct TileData
{
    TileId _id{0};
    fge::RectInt _rect;
    mutable fge::PropertyList _properties{};
};

inline bool operator<(const fge::TileData& l, int r)
{
    return l._id < r;
}
inline bool operator<(int l, const fge::TileData& r)
{
    return l < r._id;
}
inline bool operator<(const fge::TileData& l, const fge::TileData& r)
{
    return l._id < r._id;
}

/**
 * \class TileSet
 * \brief A class that represent a set of tiles that can be used in a TileLayer
 * \ingroup graphics
 *
 * This class is compatible with the "Tiled" map editor.
 */
class FGE_API TileSet
{
public:
    using TileListType = std::set<fge::TileData, std::less<>>;

    TileSet() = default;
    TileSet(fge::Texture texture);
    TileSet(fge::Texture texture, const fge::Vector2i& tileSize);
    TileSet(fge::Texture texture, const fge::Vector2i& tileSize, const fge::Vector2i& offset);

    /**
     * \brief Clear the tiles
     */
    void clearTiles();

    /**
     * \brief Set the name of the TileSet
     *
     * \param name The name of the TileSet
     */
    void setName(std::string name);
    /**
     * \brief Get the name of the TileSet
     *
     * \return The name of the TileSet
     */
    [[nodiscard]] const std::string& getName() const;

    /**
     * \brief Check if the TileSet have a valid Texture
     *
     * \return \b true if the TileSet have a valid Texture, \b false otherwise
     */
    [[nodiscard]] bool valid() const;

    /**
     * \brief Get the texture of the TileSet
     *
     * \return The texture of the TileSet
     */
    [[nodiscard]] const fge::Texture& getTexture() const;
    /**
     * \brief Set the texture of the TileSet
     *
     * This function will automatically slice the texture into tiles.
     *
     * \param texture The texture of the TileSet
     */
    void setTexture(fge::Texture texture);

    /**
     * \brief Get the tile size of the TileSet
     *
     * \return The tile size of the TileSet
     */
    [[nodiscard]] const fge::Vector2i& getTileSize() const;
    /**
     * \brief Set the tile size of the TileSet
     *
     * This function will automatically slice the texture into tiles.
     *
     * \param tileSize The tile size of the TileSet
     */
    void setTileSize(const fge::Vector2i& tileSize);

    /**
     * \brief Get the offset in pixel of the TileSet
     *
     * \return The offset in pixel of the TileSet
     */
    [[nodiscard]] const fge::Vector2i& getOffset() const;
    /**
     * \brief Set the offset in pixel of the TileSet
     *
     * \param offset The offset in pixel of the TileSet
     */
    void setOffset(const fge::Vector2i& offset);

    /**
     * \brief Get the total number of tiles in the TileSet
     *
     * \return The total number of tiles in the TileSet
     */
    [[nodiscard]] std::size_t getTileCount() const;

    /**
     * \brief Retrieve a tile by its local id
     *
     * \param id The local id of the tile
     * \return The tile pointer if found, \b nullptr otherwise
     */
    [[nodiscard]] const fge::TileData* getTile(TileId id) const;

    /**
     * \brief Get the local id of a tile by its grid position
     *
     * \param position The grid position of the tile
     * \return The local id of the tile if found, \b -1 otherwise
     */
    [[nodiscard]] TileId getLocalId(const fge::Vector2i& position) const;
    /**
     * \brief Get the local id of a tile by its global id
     *
     * The global id is subtracted by the first global id of the tileset in
     * order to get the local id.
     *
     * \param gid The global id of the tile
     * \return The local id of the tile if found, \b -1 otherwise
     */
    [[nodiscard]] TileId getLocalId(TileId gid) const;
    /**
     * \brief Check if the global id is in the tileset
     *
     * \param gid The global id of the tile
     * \return \b true if the global id is in the tileset, \b false otherwise
     */
    [[nodiscard]] bool isGidContained(TileId gid) const;
    /**
     * \brief Set the first global id of the tileset
     *
     * The first global id correspond to the first tile id in the tileset.
     *
     * \param gid The first global id of the tileset
     */
    void setFirstGid(TileId gid);
    /**
     * \brief Get the first global id of the tileset
     *
     * \return The first global id of the tileset
     */
    [[nodiscard]] TileId getFirstGid() const;

    [[nodiscard]] TileListType::const_iterator begin() const;
    [[nodiscard]] TileListType::const_iterator end() const;

    /**
     * \brief Slice the texture into tiles
     *
     * The texture will be sliced into tiles of the size specified by the tile size.
     * Tiles is always sliced from the top left corner of the texture in a Z pattern.
     *
     * Previous tiles will be cleared.
     */
    void slice();

    /**
     * \brief Return the number of columns in the texture
     *
     * \return The number of columns in the texture
     */
    [[nodiscard]] int getColumns() const;
    /**
     * \brief Return the number of rows in the texture
     *
     * \return The number of rows in the texture
     */
    [[nodiscard]] int getRows() const;

    /**
     * \brief Get the texture rectangle of a tile by its local id
     *
     * \param id The local id of the tile
     * \return The texture rectangle of the tile if found, \b std::nullopt otherwise
     */
    [[nodiscard]] std::optional<fge::RectInt> getTextureRect(TileId id) const;
    /**
     * \brief Compute the supposed texture rectangle with a local id
     *
     * \param id The local id of the tile
     * \return The supposed texture rectangle of the tile
     */
    [[nodiscard]] fge::RectInt computeTextureRect(TileId id) const;

    fge::TileSet& operator=(fge::Texture texture);

private:
    void setTile(fge::TileData tile);
    void pushTile(fge::TileData tile);

    std::string g_name;
    fge::Texture g_texture;
    fge::Vector2i g_tileSize;
    fge::Vector2i g_offset{0, 0};
    TileListType g_tiles;
    TileId g_firstGid{1};
    int g_columns{0};
    int g_rows{0};
};

FGE_API void to_json(nlohmann::json& j, const fge::TileSet& p);
FGE_API void from_json(const nlohmann::json& j, fge::TileSet& p);

FGE_API void to_json(nlohmann::json& j, const fge::TileData& p);
FGE_API void from_json(const nlohmann::json& j, fge::TileData& p);

} // namespace fge

#endif // _FGE_C_TILESET_HPP_INCLUDED
