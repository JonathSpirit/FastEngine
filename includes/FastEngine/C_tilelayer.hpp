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

class FGE_API TileLayer : public sf::Transformable, public sf::Drawable
{
public:
    struct Data
    {
        TileId _gid;
        std::shared_ptr<fge::TileSet> _tileSet;
        sf::Vertex _vertex[4];
    };

    TileLayer() = default;

#ifndef FGE_DEF_SERVER
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
#endif

    void setId(TileId id);
    [[nodiscard]] TileId getId() const;

    void setName(std::string name);
    [[nodiscard]] const std::string& getName() const;

    [[nodiscard]] const fge::Matrix<TileLayer::Data>& getData() const;
    void setGid(std::size_t x, std::size_t y, TileId gid);

    [[nodiscard]] std::size_t getTileSetSize() const;
    void pushTileSet(std::shared_ptr<fge::TileSet> tileSet);
    [[nodiscard]] std::shared_ptr<fge::TileSet> getTileSet(std::size_t index) const;
    void clearTileSet();

    void refreshTextures();

private:
    static void updatePositions(TileLayer::Data& data);
    static void updateTexCoords(TileLayer::Data& data);
    std::shared_ptr<fge::TileSet> retrieveTileSet(TileId gid) const;

    TileId g_id{1};
    std::string g_name;
    fge::Matrix<TileLayer::Data> g_data;
    std::vector<std::shared_ptr<fge::TileSet> > g_tileSets;
};

FGE_API void to_json(nlohmann::json& j, const fge::TileLayer& p);
FGE_API void from_json(const nlohmann::json& j, fge::TileLayer& p);

}//end fge

#endif // _FGE_C_TILELAYER_HPP_INCLUDED
