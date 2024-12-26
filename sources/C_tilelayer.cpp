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

#ifndef FGE_DEF_SERVER
void TileLayer::draw(fge::RenderTarget& target, fge::RenderStates const& states) const
{
    auto statesCopy = states.copy();

    statesCopy._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));

    for (auto const& data: this->g_data)
    {
        if (data.g_tileSet)
        {
            statesCopy._resTextures.set(data.g_tileSet->getTexture().retrieve(), 1);
            statesCopy._vertexBuffer = &data.g_vertexBuffer;
            target.draw(statesCopy);
        }
    }
}
#endif //FGE_DEF_SERVER

void TileLayer::clear()
{
    this->g_data.clear();
}

void TileLayer::setId(GlobalTileId id)
{
    this->g_id = id;
}
GlobalTileId TileLayer::getId() const
{
    return this->g_id;
}

void TileLayer::setName(std::string name)
{
    this->g_name = std::move(name);
}
std::string const& TileLayer::getName() const
{
    return this->g_name;
}

fge::Matrix<TileLayer::Tile> const& TileLayer::getTiles() const
{
    return this->g_data;
}
void TileLayer::setGid(std::size_t x, std::size_t y, TileSetList const& tileSets, GlobalTileId gid)
{
    auto* data = this->g_data.getPtr(x, y);
    if (data != nullptr)
    {
        data->g_gid = gid;
        data->g_tileSet = TileLayer::retrieveAssociatedTileSet(tileSets, gid);
        if (data->g_tileSet)
        {
            data->g_position = {static_cast<float>(data->g_tileSet->getTileSize().x * x),
                                static_cast<float>(data->g_tileSet->getTileSize().y * y)};
        }
        data->updatePositions();
        data->updateTexCoords();
    }
}
void TileLayer::setGid(std::size_t x, std::size_t y, GlobalTileId gid)
{
    auto* data = this->g_data.getPtr(x, y);
    if (data != nullptr)
    {
        data->g_gid = gid;
    }
}
void TileLayer::setGridSize(std::size_t x, std::size_t y)
{
    this->g_data.clear();
    this->g_data.setSize(x, y);
}

void TileLayer::refreshTextures(TileSetList const& tileSets)
{
    for (std::size_t ix = 0; ix < this->g_data.getSizeX(); ++ix)
    {
        for (std::size_t iy = 0; iy < this->g_data.getSizeY(); ++iy)
        {
            auto& data = this->g_data[ix][iy];

            data.g_tileSet = TileLayer::retrieveAssociatedTileSet(tileSets, data.g_gid);
            if (data.g_tileSet)
            {
                data.g_position = {static_cast<float>(data.g_tileSet->getTileSize().x * ix),
                                   static_cast<float>(data.g_tileSet->getTileSize().y * iy)};
            }
            data.updatePositions();
            data.updateTexCoords();
        }
    }
}

fge::RectFloat TileLayer::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat TileLayer::getLocalBounds() const
{
    fge::RectFloat bounds{FGE_NUMERIC_LIMITS_VECTOR_MAX(fge::Vector2f), FGE_NUMERIC_LIMITS_VECTOR_MIN(fge::Vector2f)};
    for (auto const& tile: this->g_data)
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

std::shared_ptr<fge::TileSet> TileLayer::retrieveAssociatedTileSet(TileSetList const& tileSets, GlobalTileId gid)
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

void to_json(nlohmann::json& j, fge::TileLayer const& p)
{
    j = nlohmann::json{{"id", p.getId()},
                       {"name", p.getName()},
                       {"type", "tilelayer"},

                       {"width", p.getTiles().getSizeX()},
                       {"height", p.getTiles().getSizeY()},

                       {"offsetx", static_cast<int>(p.getPosition().x)},
                       {"offsety", static_cast<int>(p.getPosition().y)}};

    auto& dataArray = j["data"];
    dataArray = nlohmann::json::array();

    for (std::size_t ih = 0; ih < p.getTiles().getSizeY(); ++ih)
    {
        for (std::size_t iw = 0; iw < p.getTiles().getSizeX(); ++iw)
        {
            int gid = p.getTiles().get(iw, ih).getGid();
            dataArray.push_back(gid);
        }
    }
}
void from_json(nlohmann::json const& j, fge::TileLayer& p)
{
    p.setId(j.at("id").get<int>());

    p.setName(j.value<std::string>("name", {}));

    std::size_t w{0};
    std::size_t h{0};

    j.at("width").get_to(w);
    j.at("height").get_to(h);

    p.clear();
    p.setGridSize(w, h);

    auto const& dataArray = j.at("data");
    if (dataArray.is_array() && dataArray.size() == (w * h))
    {
        auto it = dataArray.begin();
        for (std::size_t ih = 0; ih < h; ++ih)
        {
            for (std::size_t iw = 0; iw < w; ++iw)
            {
                int gid = it->get<int>();
                p.setGid(iw, ih, gid);
                ++it;
            }
        }
    }

    p.setPosition({static_cast<float>(j.value<int>("offsetx", 0)), static_cast<float>(j.value<int>("offsety", 0))});
}

} // namespace fge
