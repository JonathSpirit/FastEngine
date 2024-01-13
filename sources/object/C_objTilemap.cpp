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

#include "FastEngine/object/C_objTilemap.hpp"

namespace fge
{

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTileMap)
{
    auto copyStates = states.copy(this->_transform.start(*this, states._resTransform.get()));
    for (std::size_t i = 0; i < this->g_layers.size(); ++i)
    {
        target.draw(*this->g_layers[i], copyStates);
    }
}
#endif

void ObjTileMap::clear()
{
    this->g_tileSets.clear();
    this->g_layers.clear();
}

TileSetList& ObjTileMap::getTileSets()
{
    return this->g_tileSets;
}
TileSetList const& ObjTileMap::getTileSets() const
{
    return this->g_tileSets;
}

TileLayerList& ObjTileMap::getTileLayers()
{
    return this->g_layers;
}
TileLayerList const& ObjTileMap::getTileLayers() const
{
    return this->g_layers;
}

void ObjTileMap::save(nlohmann::json& jsonObject, [[maybe_unused]] fge::Scene* scene)
{
    jsonObject = nlohmann::json{{"infinite", false},
                                {"orientation", "orthogonal"},
                                {"renderorder", "right-down"},
                                {"tiledversion", "1.9.2"},
                                {"version", "1.9"},
                                {"type", "map"}};

    auto& tileSetsArray = jsonObject["tilesets"];
    tileSetsArray = nlohmann::json::array();

    for (auto& tileSet: this->g_tileSets)
    {
        auto& obj = tileSetsArray.emplace_back(nlohmann::json::object());
        obj = *tileSet;
    }

    auto& layersArray = jsonObject["layers"];
    layersArray = nlohmann::json::array();

    for (auto& layer: this->g_layers)
    {
        auto& obj = layersArray.emplace_back(nlohmann::json::object());
        obj = *layer;
    }
}
void ObjTileMap::load(nlohmann::json& jsonObject, [[maybe_unused]] fge::Scene* scene)
{
    auto const& tileSetsArray = jsonObject.at("tilesets");
    if (tileSetsArray.is_array())
    {
        for (auto const& tileSet: tileSetsArray)
        {
            this->g_tileSets.emplace_back(std::make_shared<fge::TileSet>());
            tileSet.get_to(*this->g_tileSets.back());
        }
    }

    auto const& layersArray = jsonObject.at("layers");
    if (layersArray.is_array())
    {
        for (auto const& layer: layersArray)
        {
            this->g_layers.emplace_back(std::make_shared<fge::TileLayer>());
            layer.get_to(*this->g_layers.back());
            this->g_layers.back()->refreshTextures(this->g_tileSets);
        }
    }
}

void ObjTileMap::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);
}
void ObjTileMap::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);
}

char const* ObjTileMap::getClassName() const
{
    return FGE_OBJTILEMAP_CLASSNAME;
}
char const* ObjTileMap::getReadableClassName() const
{
    return "tileMap";
}

fge::RectFloat ObjTileMap::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjTileMap::getLocalBounds() const
{
    return {{0.f, 0.f}, {1.f, 1.f}};
}

} // namespace fge
