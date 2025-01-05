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

#include "FastEngine/C_tilelayer.hpp"

namespace fge
{

//BaseLayer

void BaseLayer::clear()
{
    this->g_id = FGE_LAYER_BAD_ID;
    this->g_name.clear();
}

void BaseLayer::setId(GlobalTileId id)
{
    this->g_id = id;
}
GlobalTileId BaseLayer::getId() const
{
    return this->g_id;
}

void BaseLayer::setName(std::string_view name)
{
    this->g_name = name;
}
std::string const& BaseLayer::getName() const
{
    return this->g_name;
}

void BaseLayer::save(nlohmann::json& jsonObject)
{
    jsonObject["id"] = this->g_id;
    jsonObject["name"] = this->g_name;
    switch (this->getType())
    {
    case Types::TILE_LAYER:
        jsonObject["type"] = "tilelayer";
        break;
    case Types::OBJECT_GROUP:
        jsonObject["type"] = "objectgroup";
        break;
    }
}
void BaseLayer::load(nlohmann::json const& jsonObject, [[maybe_unused]] std::filesystem::path const& filePath)
{
    this->g_id = jsonObject.at("id").get<GlobalTileId>();
    this->g_name = jsonObject.at("name").get<std::string>();
}

std::shared_ptr<BaseLayer> BaseLayer::loadLayer(nlohmann::json const& jsonObject, std::filesystem::path const& filePath)
{
    std::shared_ptr<BaseLayer> layer;
    if (jsonObject.contains("type"))
    {
        auto const type = jsonObject.at("type").get<std::string>();
        if (type == "tilelayer")
        {
            layer = std::make_shared<TileLayer>();
        }
        else if (type == "objectgroup")
        {
            layer = std::make_shared<ObjectGroupLayer>();
        }

        if (layer)
        {
            layer->load(jsonObject, filePath);
        }
    }
    return layer;
}

//TileLayer::Tile

TileLayer::Tile::Tile() :
        g_vertexBuffer(fge::vulkan::GetActiveContext())
{
    this->g_vertexBuffer.create(4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, fge::vulkan::BufferTypes::DEVICE);
}

void TileLayer::Tile::setGid(GlobalTileId gid)
{
    this->g_gid = gid;
    this->updateTexCoords();
}
GlobalTileId TileLayer::Tile::getGid() const
{
    return this->g_gid;
}

void TileLayer::Tile::setPosition(fge::Vector2f const& position)
{
    this->g_position = position;
    this->updatePositions();
}
fge::Vector2f const& TileLayer::Tile::getPosition() const
{
    return this->g_position;
}

void TileLayer::Tile::setColor(fge::Color const& color)
{
    for (std::size_t i = 0; i < 4; ++i)
    {
        this->g_vertexBuffer.getVertices()[i]._color = color;
    }
}
fge::Color TileLayer::Tile::getColor() const
{
    return fge::Color(this->g_vertexBuffer.getVertices()[0]._color);
}

void TileLayer::Tile::setTileSet(std::shared_ptr<fge::TileSet> tileSet)
{
    this->g_tileSet = std::move(tileSet);
    this->updatePositions();
    this->updateTexCoords();
}
std::shared_ptr<fge::TileSet> const& TileLayer::Tile::getTileSet() const
{
    return this->g_tileSet;
}
TileData const* TileLayer::Tile::getTileData() const
{
    if (this->g_tileSet)
    {
        return this->g_tileSet->getTile(this->g_tileSet->getLocalId(this->g_gid));
    }
    return nullptr;
}

void TileLayer::Tile::updatePositions()
{
    if (this->g_tileSet)
    {
        auto size = static_cast<fge::Vector2f>(this->g_tileSet->getTileSize());

        this->g_vertexBuffer.getVertices()[0]._position = fge::Vector2f(this->g_position.x, this->g_position.y);
        this->g_vertexBuffer.getVertices()[1]._position =
                fge::Vector2f(this->g_position.x, this->g_position.y + size.y);
        this->g_vertexBuffer.getVertices()[2]._position =
                fge::Vector2f(this->g_position.x + size.x, this->g_position.y);
        this->g_vertexBuffer.getVertices()[3]._position =
                fge::Vector2f(this->g_position.x + size.x, this->g_position.y + size.y);
    }
}

void TileLayer::Tile::updateTexCoords()
{
    if (this->g_tileSet)
    {
        auto const* tile = this->g_tileSet->getTile(this->g_tileSet->getLocalId(this->g_gid));
        if (tile != nullptr)
        {
            auto const rect = this->g_tileSet->getTexture().getSharedData()->normalizeTextureRect(tile->_rect);

            this->g_vertexBuffer.getVertices()[0]._texCoords = fge::Vector2f(rect._x, rect._y);
            this->g_vertexBuffer.getVertices()[1]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
            this->g_vertexBuffer.getVertices()[2]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
            this->g_vertexBuffer.getVertices()[3]._texCoords =
                    fge::Vector2f(rect._x + rect._width, rect._y + rect._height);
        }
    }
}

//TileLayer

#ifndef FGE_DEF_SERVER
void TileLayer::draw(fge::RenderTarget& target, fge::RenderStates const& states) const
{
    auto statesCopy = states.copy();

    statesCopy._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));

    for (auto const& tile: this->g_tiles)
    {
        if (tile.g_tileSet)
        {
            statesCopy._resTextures.set(tile.g_tileSet->getTexture().retrieve(), 1);
            statesCopy._vertexBuffer = &tile.g_vertexBuffer;
            target.draw(statesCopy);
        }
    }
}
#endif //FGE_DEF_SERVER

TileLayer::Types TileLayer::getType() const
{
    return Types::TILE_LAYER;
}
void TileLayer::clear()
{
    BaseLayer::clear();
    this->g_tiles.clear();
}

fge::Matrix<TileLayer::Tile> const& TileLayer::getTiles() const
{
    return this->g_tiles;
}
void TileLayer::setGid(fge::Vector2size position, std::span<std::shared_ptr<TileSet>> tileSets, GlobalTileId gid)
{
    auto* tile = this->g_tiles.getPtr(position.x, position.y);
    if (tile != nullptr)
    {
        tile->g_gid = gid;
        tile->g_tileSet = TileLayer::retrieveAssociatedTileSet(tileSets, gid);
        if (tile->g_tileSet)
        {
            tile->g_position = {static_cast<float>(tile->g_tileSet->getTileSize().x * position.x),
                                static_cast<float>(tile->g_tileSet->getTileSize().y * position.y)};
        }
        tile->updatePositions();
        tile->updateTexCoords();
    }
}
GlobalTileId TileLayer::getGid(fge::Vector2size position) const
{
    auto* tile = this->g_tiles.getPtr(position.x, position.y);
    if (tile != nullptr)
    {
        return tile->g_gid;
    }
    return FGE_LAYER_BAD_ID;
}
std::optional<fge::Vector2size> TileLayer::getGridPosition(fge::Vector2f position) const
{
    auto const bounds = this->getLocalBounds();
    auto const tileLocalPosition = this->getInverseTransform() * position;

    auto const size = this->g_tiles.getSize();

    fge::Vector2size const positionIndex{
            static_cast<std::size_t>(static_cast<float>(size.x) * tileLocalPosition.x / bounds._width),
            static_cast<std::size_t>(static_cast<float>(size.y) * tileLocalPosition.y / bounds._height)};

    if (positionIndex.x < this->g_tiles.getSizeX() && positionIndex.y < this->g_tiles.getSizeY())
    {
        return positionIndex;
    }
    return std::nullopt;
}
void TileLayer::setGid(fge::Vector2size position, GlobalTileId gid)
{
    auto* data = this->g_tiles.getPtr(position.x, position.y);
    if (data != nullptr)
    {
        data->g_gid = gid;
    }
}
void TileLayer::setGridSize(fge::Vector2size size)
{
    this->g_tiles.clear();
    this->g_tiles.setSize(size.x, size.y);
}

void TileLayer::refreshTextures(std::span<std::shared_ptr<TileSet>> tileSets)
{
    for (std::size_t ix = 0; ix < this->g_tiles.getSizeX(); ++ix)
    {
        for (std::size_t iy = 0; iy < this->g_tiles.getSizeY(); ++iy)
        {
            auto& tile = this->g_tiles[ix][iy];

            tile.g_tileSet = TileLayer::retrieveAssociatedTileSet(tileSets, tile.g_gid);
            if (tile.g_tileSet)
            {
                tile.g_position = {static_cast<float>(tile.g_tileSet->getTileSize().x * ix),
                                   static_cast<float>(tile.g_tileSet->getTileSize().y * iy)};
            }
            tile.updatePositions();
            tile.updateTexCoords();
        }
    }
}
std::shared_ptr<fge::TileSet> TileLayer::retrieveAssociatedTileSet(std::span<std::shared_ptr<TileSet>> tileSets,
                                                                   GlobalTileId gid)
{
    for (auto const& tileSet: tileSets)
    {
        if (tileSet->containsGlobal(gid))
        {
            return tileSet;
        }
    }
    return nullptr;
}

fge::RectFloat TileLayer::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat TileLayer::getLocalBounds() const
{
    fge::RectFloat bounds{FGE_NUMERIC_LIMITS_VECTOR_MAX(fge::Vector2f), FGE_NUMERIC_LIMITS_VECTOR_MIN(fge::Vector2f)};
    for (auto const& tile: this->g_tiles)
    {
        if (tile.g_tileSet && tile.g_gid != 0)
        {
            auto const size = static_cast<fge::Vector2f>(tile.g_tileSet->getTileSize());
            auto const& position = tile.getPosition();

            bounds._x = std::min(bounds._x, position.x);
            bounds._y = std::min(bounds._y, position.y);
            bounds._width = std::max(bounds._width, position.x + size.x);
            bounds._height = std::max(bounds._height, position.y + size.y);
        }
    }
    return bounds;
}

void TileLayer::save(nlohmann::json& jsonObject)
{
    jsonObject = nlohmann::json{{"type", "tilelayer"},

                                {"width", this->g_tiles.getSizeX()},
                                {"height", this->g_tiles.getSizeY()},

                                {"offsetx", static_cast<int>(this->getPosition().x)},
                                {"offsety", static_cast<int>(this->getPosition().y)}};

    BaseLayer::save(jsonObject);

    auto& dataArray = jsonObject["data"];
    dataArray = nlohmann::json::array();

    for (std::size_t ih = 0; ih < this->g_tiles.getSizeY(); ++ih)
    {
        for (std::size_t iw = 0; iw < this->g_tiles.getSizeX(); ++iw)
        {
            auto const gid = this->g_tiles.get(iw, ih).getGid();
            dataArray.push_back(gid);
        }
    }
}
void TileLayer::load(nlohmann::json const& jsonObject, std::filesystem::path const& filePath)
{
    this->clear();
    BaseLayer::load(jsonObject, filePath);

    std::size_t w{0};
    std::size_t h{0};

    jsonObject.at("width").get_to(w);
    jsonObject.at("height").get_to(h);

    this->setGridSize({w, h});

    auto const& dataArray = jsonObject.at("data");
    if (dataArray.is_array() && dataArray.size() == w * h)
    {
        auto it = dataArray.begin();
        for (std::size_t ih = 0; ih < h; ++ih)
        {
            for (std::size_t iw = 0; iw < w; ++iw)
            {
                auto const gid = it->get<GlobalTileId>();
                this->setGid({iw, ih}, gid);
                ++it;
            }
        }
    }

    this->setPosition({static_cast<float>(jsonObject.value<int>("offsetx", 0)),
                       static_cast<float>(jsonObject.value<int>("offsety", 0))});
}

//ObjectGroupLayer

#ifndef FGE_DEF_SERVER
void ObjectGroupLayer::draw([[maybe_unused]] fge::RenderTarget& target,
                            [[maybe_unused]] fge::RenderStates const& states) const
{}
#endif //FGE_DEF_SERVER

BaseLayer::Types ObjectGroupLayer::getType() const
{
    return Types::OBJECT_GROUP;
}

void ObjectGroupLayer::clear()
{
    BaseLayer::clear();
    this->g_objects.clear();
}

std::vector<ObjectGroupLayer::Object> const& ObjectGroupLayer::getObjects() const
{
    return this->g_objects;
}
std::vector<ObjectGroupLayer::Object>& ObjectGroupLayer::getObjects()
{
    return this->g_objects;
}

ObjectGroupLayer::Object* ObjectGroupLayer::findObjectName(std::string_view name)
{
    for (auto& object: this->g_objects)
    {
        if (object._name == name)
        {
            return &object;
        }
    }
    return nullptr;
}
ObjectGroupLayer::Object const* ObjectGroupLayer::findObjectName(std::string_view name) const
{
    for (auto const& object: this->g_objects)
    {
        if (object._name == name)
        {
            return &object;
        }
    }
    return nullptr;
}

void ObjectGroupLayer::save(nlohmann::json& jsonObject)
{
    BaseLayer::save(jsonObject);
}

void ObjectGroupLayer::load(nlohmann::json const& jsonObject, std::filesystem::path const& filePath)
{
    this->clear();
    BaseLayer::load(jsonObject, filePath);

    auto const& objectsArray = jsonObject.at("objects");
    if (objectsArray.is_array())
    {
        this->g_objects.reserve(objectsArray.size());
        for (auto const& object: objectsArray)
        {
            ObjectGroupLayer::Object newObject;
            newObject._position.x = object.at("x").get<float>();
            newObject._position.y = object.at("y").get<float>();
            newObject._size.x = object.at("width").get<float>();
            newObject._size.y = object.at("height").get<float>();
            newObject._name = object.at("name").get<std::string>();
            newObject._id = object.at("id").get<LocalTileId>();
            newObject._rotation = object.at("rotation").get<float>();
            newObject._point = object.at("point").get<bool>();
            this->g_objects.push_back(newObject);
        }
    }
}

} // namespace fge
