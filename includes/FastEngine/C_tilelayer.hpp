/*
 * Copyright 2024 Guillaume Guillet
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

#ifndef _FGE_C_TILELAYER_HPP_INCLUDED
#define _FGE_C_TILELAYER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_matrix.hpp"
#include "FastEngine/C_tileset.hpp"
#include "FastEngine/graphic/C_color.hpp"
#include "FastEngine/graphic/C_drawable.hpp"
#include "FastEngine/graphic/C_transformable.hpp"
#include "FastEngine/vulkan/C_vertexBuffer.hpp"
#include "json.hpp"
#include <span>

namespace fge
{

using GlobalTileId = int32_t;
using LocalTileId = int32_t;

/**
 * \class TileLayer
 * \brief A tile layer contain a matrix of global tile id and a list of TileSet
 * \ingroup graphics
 *
 * This class is compatible with the "Tiled" map editor.
 */
#ifdef FGE_DEF_SERVER
class FGE_API TileLayer : public fge::Transformable
#else
class FGE_API TileLayer : public fge::Transformable, public fge::Drawable
#endif
{
public:
    /**
     * \class Tile
     * \brief A tile that contain drawing information and its global id
     * \ingroup graphics
     */
    class FGE_API Tile
    {
    public:
        Tile();

        /**
         * \brief Set the global id of the tile
         *
         * This will automatically update texture coordinates.
         *
         * \param gid The global id of the tile
         */
        void setGid(GlobalTileId gid);
        /**
         * \brief Get the global id of the tile
         *
         * \return The global id of the tile
         */
        [[nodiscard]] GlobalTileId getGid() const;

        /**
         * \brief Set the local position of the tile
         *
         * \param position The local position of the tile
         */
        void setPosition(fge::Vector2f const& position);
        /**
         * \brief Get the local position of the tile
         *
         * \return The local position of the tile
         */
        [[nodiscard]] fge::Vector2f const& getPosition() const;

        /**
         * \brief Set the color of the tile
         *
         * \param color The color of the tile
         */
        void setColor(fge::Color const& color);
        /**
         * \brief Get the color of the tile
         *
         * \return The color of the tile
         */
        [[nodiscard]] fge::Color getColor() const;

        /**
         * \brief Set the associated tileset pointer
         *
         * This will automatically update texture coordinates and positions.
         *
         * \param tileSet The associated tileset pointer
         */
        void setTileSet(std::shared_ptr<fge::TileSet> tileSet);
        /**
         * \brief Get the associated tileset pointer
         *
         * \return The associated tileset pointer
         */
        [[nodiscard]] std::shared_ptr<fge::TileSet> const& getTileSet() const;

        [[nodiscard]] TileData const* getTileData() const;

    private:
        void updatePositions();
        void updateTexCoords();

        GlobalTileId g_gid{0};
        std::shared_ptr<fge::TileSet> g_tileSet;
        fge::vulkan::VertexBuffer g_vertexBuffer;
        fge::Vector2f g_position;

        friend TileLayer;
    };

    TileLayer() = default;

#ifndef FGE_DEF_SERVER
    void draw(fge::RenderTarget& target, fge::RenderStates const& states) const override;
#endif

    /**
     * \brief Clear the matrix of tiles
     */
    void clear();

    /**
     * \brief Set the id of the layer (mostly for "Tiled" map editor compatibility)
     *
     * \param id The id of the layer
     */
    void setId(GlobalTileId id);
    /**
     * \brief Get the id of the layer
     *
     * \return The id of the layer
     */
    [[nodiscard]] GlobalTileId getId() const;

    /**
     * \brief Set the name of the layer
     *
     * \param name The name of the layer
     */
    void setName(std::string name);
    /**
     * \brief Get the name of the layer
     *
     * \return The name of the layer
     */
    [[nodiscard]] std::string const& getName() const;

    /**
     * \brief Get the matrix of tiles
     *
     * \return The matrix of tiles
     */
    [[nodiscard]] fge::Matrix<Tile> const& getTiles() const;
    /**
     * \brief Shortcut to set a global tile id and a new tileset
     *
     * \param position The position of the tile
     * \param tileSets The list of tilesets
     * \param gid The global tile id
     */
    void setGid(fge::Vector2size position, std::span<std::shared_ptr<TileSet>> tileSets, GlobalTileId gid);
    [[nodiscard]] GlobalTileId getGid(fge::Vector2size position);
    [[nodiscard]] GlobalTileId getGid(fge::Vector2f position);
    /**
     * \brief Shortcut to set a global tile id
     *
     * \param position The position of the tile
     * \param gid The global tile id
     */
    void setGid(fge::Vector2size position, GlobalTileId gid);
    /**
     * \brief Set the tiles matrix size
     *
     * \param size The size of the matrix
     */
    void setGridSize(fge::Vector2size size);

    /**
     * \brief Refresh all tiles with a list of tilesets
     *
     * \param tileSets The list of tilesets
     */
    void refreshTextures(std::span<std::shared_ptr<TileSet>> tileSets);
    [[nodiscard]] static std::shared_ptr<TileSet>
    retrieveAssociatedTileSet(std::span<std::shared_ptr<TileSet>> tileSets, GlobalTileId gid);

    [[nodiscard]] fge::RectFloat getGlobalBounds() const;
    [[nodiscard]] fge::RectFloat getLocalBounds() const;

private:
    GlobalTileId g_id{1};
    std::string g_name;
    fge::Matrix<Tile> g_tiles;
};

FGE_API void to_json(nlohmann::json& j, fge::TileLayer const& p);
FGE_API void from_json(nlohmann::json const& j, fge::TileLayer& p);

} // namespace fge

#endif // _FGE_C_TILELAYER_HPP_INCLUDED
