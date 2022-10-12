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

#ifndef _FGE_C_TILELAYER_HPP_INCLUDED
#define _FGE_C_TILELAYER_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_tileset.hpp>
#include <FastEngine/C_matrix.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <json.hpp>

namespace fge
{

using TileId = int32_t;
using TileSetList = std::vector<std::shared_ptr<fge::TileSet> >;

class FGE_API TileLayer : public sf::Transformable, public sf::Drawable
{
public:
    struct Data
    {
        TileId _gid{0};
        std::shared_ptr<fge::TileSet> _tileSet;
        sf::Vertex _vertex[4];
        sf::Vector2f _position;

        void updatePositions();
        void updateTexCoords();
    };

    TileLayer() = default;

#ifndef FGE_DEF_SERVER
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
#endif

    void clear();

    void setId(TileId id);
    [[nodiscard]] TileId getId() const;

    void setName(std::string name);
    [[nodiscard]] const std::string& getName() const;

    [[nodiscard]] const fge::Matrix<TileLayer::Data>& getData() const;
    void setGid(std::size_t x, std::size_t y, const TileSetList& tileSets, TileId gid);
    void setGid(std::size_t x, std::size_t y, TileId gid);
    void setGridSize(std::size_t x, std::size_t y);

    void refreshTextures(const TileSetList& tileSets);

private:
    static std::shared_ptr<fge::TileSet> retrieveAssociatedTileSet(const TileSetList& tileSets, TileId gid);

    TileId g_id{1};
    std::string g_name;
    fge::Matrix<TileLayer::Data> g_data;
};

FGE_API void to_json(nlohmann::json& j, const fge::TileLayer& p);
FGE_API void from_json(const nlohmann::json& j, fge::TileLayer& p);

}//end fge

#endif // _FGE_C_TILELAYER_HPP_INCLUDED
