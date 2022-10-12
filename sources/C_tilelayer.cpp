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

void TileLayer::Data::updatePositions()
{
    if (this->_tileSet)
    {
        auto size = static_cast<sf::Vector2f>(this->_tileSet->getTileSize());

        this->_vertex[0].position = sf::Vector2f(this->_position.x, this->_position.y);
        this->_vertex[1].position = sf::Vector2f(this->_position.x, this->_position.y+size.y);
        this->_vertex[2].position = sf::Vector2f(this->_position.x+size.x, this->_position.y);
        this->_vertex[3].position = sf::Vector2f(this->_position.x+size.x, this->_position.y+size.y);
    }
}

void TileLayer::Data::updateTexCoords()
{
    if (this->_tileSet)
    {
        const auto* tile = this->_tileSet->getTile(this->_tileSet->getLocalId(this->_gid));
        if (tile != nullptr)
        {
            auto rect = tile->_rect;

            float left   = static_cast<float>(rect.left);
            float right  = left + static_cast<float>(rect.width);
            float top    = static_cast<float>(rect.top);
            float bottom = top + static_cast<float>(rect.height);

            this->_vertex[0].texCoords = sf::Vector2f(left, top);
            this->_vertex[1].texCoords = sf::Vector2f(left, bottom);
            this->_vertex[2].texCoords = sf::Vector2f(right, top);
            this->_vertex[3].texCoords = sf::Vector2f(right, bottom);
        }
        else
        {
            std::terminate();
        }
    }
}

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

void TileLayer::clear()
{
    this->g_data.clear();
}

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
void TileLayer::setGid(std::size_t x, std::size_t y, const TileSetList& tileSets, TileId gid)
{
    auto* data = this->g_data.getPtr(x,y);
    if (data != nullptr)
    {
        data->_gid = gid;
        data->_tileSet = TileLayer::retrieveAssociatedTileSet(tileSets, gid);
        if (data->_tileSet)
        {
            data->_position = {static_cast<float>(data->_tileSet->getTileSize().x*x),
                               static_cast<float>(data->_tileSet->getTileSize().y*y)};
        }
        data->updatePositions();
        data->updateTexCoords();
    }
}
void TileLayer::setGid(std::size_t x, std::size_t y, TileId gid)
{
    auto* data = this->g_data.getPtr(x,y);
    if (data != nullptr)
    {
        data->_gid = gid;
    }
}
void TileLayer::setGridSize(std::size_t x, std::size_t y)
{
    this->g_data.clear();
    this->g_data.setSize(x, y);
}

void TileLayer::refreshTextures(const TileSetList& tileSets)
{
    for (std::size_t ix=0; ix<this->g_data.getSizeX(); ++ix)
    {
        for (std::size_t iy = 0; iy<this->g_data.getSizeY(); ++iy)
        {
            auto& data = this->g_data[ix][iy];

            data._tileSet = TileLayer::retrieveAssociatedTileSet(tileSets, data._gid);
            if (data._tileSet)
            {
                data._position = {static_cast<float>(data._tileSet->getTileSize().x*ix),
                                  static_cast<float>(data._tileSet->getTileSize().y*iy)};
            }
            data.updatePositions();
            data.updateTexCoords();
        }
    }
}

std::shared_ptr<fge::TileSet> TileLayer::retrieveAssociatedTileSet(const TileSetList& tileSets, TileId gid)
{
    for (const auto& tileSet : tileSets)
    {
        if ( tileSet->isGidContained(gid) )
        {
            return tileSet;
        }
    }
    return nullptr;
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
    p.setId( j.at("id").get<int>() );

    p.setName( j.value<std::string>("name", {}) );

    std::size_t w{0};
    std::size_t h{0};

    j.at("width").get_to(w);
    j.at("height").get_to(h);

    p.clear();
    p.setGridSize(w,h);

    const auto& dataArray = j.at("data");
    if (dataArray.is_array() && dataArray.size() == (w*h))
    {
        auto it = dataArray.begin();
        for (std::size_t ih=0; ih<h; ++ih)
        {
            for (std::size_t iw=0; iw<w; ++iw)
            {
                int gid = it->get<int>();
                p.setGid(iw, ih, gid);
                ++it;
            }
        }
    }

    p.setPosition({static_cast<float>(j.value<int>("offsetx", 0)),
                   static_cast<float>(j.value<int>("offsety", 0))});
}

}//end fge
