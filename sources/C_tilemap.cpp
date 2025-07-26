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

#include "FastEngine/C_tilemap.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/object/C_objTilelayer.hpp"

namespace fge
{

void TileMap::clear()
{
    this->_tileSets.clear();
    this->_layers.clear();
    this->_generatedObjects.clear();
}

TileMap::TileLayerList::value_type* TileMap::findLayerName(std::string_view name)
{
    for (auto& layer: this->_layers)
    {
        if (layer->getName() == name)
        {
            return &layer;
        }
    }
    return nullptr;
}
TileMap::TileLayerList::value_type const* TileMap::findLayerName(std::string_view name) const
{
    for (auto const& layer: this->_layers)
    {
        if (layer->getName() == name)
        {
            return &layer;
        }
    }
    return nullptr;
}

bool TileMap::loadFromFile(std::filesystem::path const& path)
{
    nlohmann::json inputJson;
    if (!LoadJsonFromFile(path, inputJson))
    {
        return false;
    }

    if (!inputJson.is_object() || inputJson.size() == 0)
    {
        return false;
    }

    this->load(inputJson, path);
    return true;
}
void TileMap::save(nlohmann::json& jsonObject) const
{
    jsonObject = nlohmann::json{{"infinite", false},
                                {"orientation", "orthogonal"},
                                {"renderorder", "right-down"},
                                {"tiledversion", "1.9.2"},
                                {"version", "1.9"},
                                {"type", "map"}};

    auto& tileSetsArray = jsonObject["tilesets"];
    tileSetsArray = nlohmann::json::array();

    for (auto const& tileSet: this->_tileSets)
    {
        auto& obj = tileSetsArray.emplace_back(nlohmann::json::object());
        obj = *tileSet;
    }

    auto& layersArray = jsonObject["layers"];
    layersArray = nlohmann::json::array();

    for (auto const& layer: this->_layers)
    {
        auto& obj = layersArray.emplace_back(nlohmann::json::object());
        layer->save(obj);
    }
}
void TileMap::load(nlohmann::json& jsonObject, std::filesystem::path const& filePath)
{
    this->clear();

    auto const& tileSetsArray = jsonObject.at("tilesets");
    if (tileSetsArray.is_array())
    {
        for (auto const& tileSet: tileSetsArray)
        {
            if (tileSet.contains("source"))
            { //TileSet data is in another json file
                auto const externFilePath = filePath.parent_path() / tileSet.at("source").get<std::filesystem::path>();
                nlohmann::json externJson;
                if (!LoadJsonFromFile(externFilePath, externJson))
                {
                    throw fge::Exception("Failed to load tileset file: " + externFilePath.string());
                }

                auto newTileSet = std::make_shared<TileSet>();
                externJson.get_to(*newTileSet);
                this->_tileSets.emplace_back(std::move(newTileSet));
            }
            else
            {
                auto newTileSet = std::make_shared<TileSet>();
                tileSet.get_to(*newTileSet);
                this->_tileSets.emplace_back(std::move(newTileSet));
            }
        }
    }

    auto const& layersArray = jsonObject.at("layers");
    if (layersArray.is_array())
    {
        for (auto const& layer: layersArray)
        {
            auto newLayer = BaseLayer::loadLayer(layer, filePath);
            if (newLayer)
            {
                this->_layers.push_back(std::move(newLayer));
                if (this->_layers.back()->getType() == BaseLayer::Types::TILE_LAYER)
                {
                    this->_layers.back()->as<TileLayer>()->refreshTextures(this->_tileSets);
                }
            }
        }
    }
}
void TileMap::generateObjects(fge::Scene& scene, fge::ObjectPlan basePlan)
{
    this->_generatedObjects.clear();
    auto plan = basePlan;
    for (auto const& layer: this->_layers)
    {
        if (layer->getType() == BaseLayer::Types::TILE_LAYER)
        {
            auto* objLayer = scene.newObject<ObjTileLayer>({plan}, this->shared_from_this(),
                                                           std::static_pointer_cast<TileLayer>(layer));
            objLayer->setLayerName(layer->getName());
            this->_generatedObjects.emplace_back(objLayer->_myObjectData);
            plan += FGE_TILEMAP_PLAN_STEP;
        }
    }
}

ObjectDataShared TileMap::retrieveGeneratedTilelayerObject(std::string_view layerName) const
{
    for (auto const& objData: this->_generatedObjects)
    {
        if (objData.expired())
        {
            continue;
        }

        auto objLayerData = objData.lock();
        if (std::strcmp(objLayerData->getObject()->getClassName(), FGE_OBJTILELAYER_CLASSNAME) != 0)
        {
            continue;
        }

        auto* objLayer = objLayerData->getObject<ObjTileLayer>();
        if (objLayer->getLayerName() != layerName)
        {
            continue;
        }

        return objLayerData;
    }
    return nullptr;
}

} // namespace fge
