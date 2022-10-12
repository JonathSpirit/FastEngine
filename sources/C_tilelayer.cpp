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

#include "FastEngine/C_tilelayer.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

namespace fge
{

#ifndef FGE_DEF_SERVER
void TileLayer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= this->getTransform();

    for (const auto& data : this->g_data)
    {
        if (data._tileSet)
        {
            states.texture = static_cast<const sf::Texture*>(data._tileSet->getTexture());
            target.draw(data._vertex, 4, sf::TriangleStrip, states);
        }
    }
}
#endif //FGE_DEF_SERVER

void TileLayer::setId(TileId id)
{
    this->g_id = id;
}
TileId TileLayer::getId() const
{
    return this->g_id;
}

void TileLayer::setName(std::string name)
{
    this->g_name = std::move(name);
}
const std::string& TileLayer::getName() const
{
    return this->g_name;
}

const fge::Matrix<TileLayer::Data>& TileLayer::getData() const
{
    return this->g_data;
}
void TileLayer::setGid(std::size_t x, std::size_t y, TileId gid)
{
    auto* data = this->g_data.getPtr(x,y);
    if (data != nullptr)
    {
        data->_gid = gid;
        data->_tileSet = this->retrieveTileSet(gid);
        if (data->_tileSet)
        {
            TileLayer::updatePositions(*data);
            TileLayer::updateTexCoords(*data);
        }
    }
}

std::size_t TileLayer::getTileSetSize() const
{
    return this->g_tileSets.size();
}
void TileLayer::pushTileSet(std::shared_ptr<fge::TileSet> tileSet)
{
    this->g_tileSets.push_back(std::move(tileSet));
}
std::shared_ptr<fge::TileSet> TileLayer::getTileSet(std::size_t index) const
{
    return this->g_tileSets[index];
}
void TileLayer::clearTileSet()
{
    this->g_tileSets.clear();
}

void TileLayer::refreshTextures()
{
    for (auto& data : this->g_data)
    {
        data._tileSet = this->retrieveTileSet(data._gid);
        TileLayer::updatePositions(data);
        TileLayer::updateTexCoords(data);
    }
}

std::shared_ptr<fge::TileSet> TileLayer::retrieveTileSet(TileId gid) const
{
    for (const auto& tileSet : this->g_tileSets)
    {
        auto id = tileSet->getLocalId(gid);
        if (id >= 0)
        {
            const auto* tile = tileSet->getTile(id);
            if (tile != nullptr)
            {
                return tileSet;
            }
        }
    }
    return nullptr;
}

void TileLayer::updatePositions(TileLayer::Data& data)
{
    auto size = static_cast<sf::Vector2f>(data._tileSet->getTileSize());

    data._vertex[0].position = sf::Vector2f(0.0f, 0.0f);
    data._vertex[1].position = sf::Vector2f(0.0f, size.y);
    data._vertex[2].position = sf::Vector2f(size.x, 0.0f);
    data._vertex[3].position = sf::Vector2f(size.x, size.y);
}

void TileLayer::updateTexCoords(TileLayer::Data& data)
{
    auto rect = data._tileSet->getTile(data._gid)->_rect;

    float left   = static_cast<float>(rect.left);
    float right  = left + static_cast<float>(rect.width);
    float top    = static_cast<float>(rect.top);
    float bottom = top + static_cast<float>(rect.height);

    data._vertex[0].texCoords = sf::Vector2f(left, top);
    data._vertex[1].texCoords = sf::Vector2f(left, bottom);
    data._vertex[2].texCoords = sf::Vector2f(right, top);
    data._vertex[3].texCoords = sf::Vector2f(right, bottom);
}

void to_json(nlohmann::json& j, const fge::TileLayer& p)
{
    /*j = nlohmann::json{{"id", p._id},
                       {"width", p._rect.width},
                       {"height", p._rect.height},
                       {"x", p._rect.left},
                       {"y", p._rect.top}};*/
}
void from_json(const nlohmann::json& j, fge::TileLayer& p)
{
    /*j.at("id").get_to(p._id);

    p._rect.width = j.value<int>("width", -1);
    p._rect.height = j.value<int>("height", -1);
    p._rect.left = j.value<int>("x", -1);
    p._rect.top = j.value<int>("y", -1);*/
}

}//end fge
