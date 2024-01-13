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

#ifndef _FGE_C_OBJTILEMAP_HPP_INCLUDED
#define _FGE_C_OBJTILEMAP_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_texture.hpp"
#include "FastEngine/C_tilelayer.hpp"

#define FGE_OBJTILEMAP_CLASSNAME "FGE:OBJ:TILEMAP"

namespace fge
{

using TileSetList = std::vector<std::shared_ptr<fge::TileSet>>;
using TileLayerList = std::vector<std::shared_ptr<fge::TileLayer>>;

class FGE_API ObjTileMap : public fge::Object
{
public:
    ObjTileMap() = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTileMap)

    FGE_OBJ_DRAW_DECLARE

    void clear();

    TileSetList& getTileSets();
    TileSetList const& getTileSets() const;

    TileLayerList& getTileLayers();
    TileLayerList const& getTileLayers() const;

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet const& pck) override;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

private:
    TileLayerList g_layers;
    TileSetList g_tileSets;
};

} // namespace fge

#endif // _FGE_C_OBJTILEMAP_HPP_INCLUDED
