/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_C_TILEMAP_HPP_INCLUDED
#define _FGE_C_TILEMAP_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/C_tilelayer.hpp"
#include "FastEngine/C_tileset.hpp"

#define FGE_TILEMAP_PLAN_STEP 2

namespace fge
{

struct FGE_API TileMap : public std::enable_shared_from_this<TileMap>
{
private:
    TileMap() = default;

public:
    using TileSetList = std::vector<std::shared_ptr<TileSet>>;
    using TileLayerList = std::vector<std::shared_ptr<BaseLayer>>;

    TileLayerList _layers;
    TileSetList _tileSets;
    std::vector<ObjectDataWeak> _generatedObjects;

    void clear();

    [[nodiscard]] TileLayerList::value_type* findLayerName(std::string_view name);
    [[nodiscard]] TileLayerList::value_type const* findLayerName(std::string_view name) const;

    bool loadFromFile(std::filesystem::path const& path);
    void save(nlohmann::json& jsonObject) const;
    void load(nlohmann::json& jsonObject, std::filesystem::path const& filePath);
    void generateObjects(fge::Scene& scene, fge::ObjectPlan basePlan = FGE_SCENE_PLAN_DEFAULT);

    [[nodiscard]] ObjectDataShared retrieveGeneratedTilelayerObject(std::string_view layerName) const;

    inline static std::shared_ptr<TileMap> create() { return std::shared_ptr<TileMap>(new TileMap()); }
};

} // namespace fge

#endif // _FGE_C_TILEMAP_HPP_INCLUDED
