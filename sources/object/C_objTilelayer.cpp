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

#include "FastEngine/object/C_objTilelayer.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjTileLayer::ObjTileLayer(std::shared_ptr<TileMap> tilemap, std::shared_ptr<TileLayer> tilelayer) :
        g_tileMap(std::move(tilemap)),
        g_tileLayer(std::move(tilelayer))
{
    this->_drawMode = DrawModes::DRAW_ALWAYS_DRAWN;
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTileLayer)
{
    if (this->g_tileLayer == nullptr)
    {
        return;
    }

    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));
    this->g_tileLayer->draw(target, copyStates);
}
#endif

void ObjTileLayer::clear()
{
    this->g_tileLayer.reset();
    this->g_tileMap.reset();
}

void ObjTileLayer::setLayerName(std::string_view name)
{
    this->g_layerName = name;
}
std::string const& ObjTileLayer::getLayerName() const
{
    return this->g_layerName;
}
void ObjTileLayer::setData(std::shared_ptr<TileMap> tilemap, std::shared_ptr<TileLayer> tilelayer)
{
    this->g_tileMap = std::move(tilemap);
    this->g_tileLayer = std::move(tilelayer);
}

std::shared_ptr<TileMap> ObjTileLayer::getTileMap() const
{
    return this->g_tileMap.lock();
}

char const* ObjTileLayer::getClassName() const
{
    return FGE_OBJTILELAYER_CLASSNAME;
}
char const* ObjTileLayer::getReadableClassName() const
{
    return "tileLayer";
}

fge::RectFloat ObjTileLayer::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjTileLayer::getLocalBounds() const
{
    return {{0.f, 0.f}, {1.f, 1.f}};
}

} // namespace fge
