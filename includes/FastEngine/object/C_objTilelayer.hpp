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

#ifndef _FGE_C_OBJTILELAYER_HPP_INCLUDED
#define _FGE_C_OBJTILELAYER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_tilelayer.hpp"
#include "FastEngine/C_tilemap.hpp"

#define FGE_OBJTILELAYER_CLASSNAME "FGE:OBJ:TILELAYER"

namespace fge
{

class FGE_API ObjTileLayer : public fge::Object
{
public:
    ObjTileLayer() = default;
    ObjTileLayer(std::shared_ptr<TileMap> tilemap, std::shared_ptr<TileLayer> tilelayer);
    ~ObjTileLayer() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTileLayer)

    FGE_OBJ_DRAW_DECLARE

    void clear();

    void setLayerName(std::string_view name);
    [[nodiscard]] std::string const& getLayerName() const;
    void setData(std::shared_ptr<TileMap> tilemap, std::shared_ptr<TileLayer> tilelayer);

    [[nodiscard]] std::shared_ptr<TileMap> getTileMap() const;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

private:
    std::weak_ptr<TileMap> g_tileMap;
    std::shared_ptr<TileLayer> g_tileLayer;
    std::string g_layerName;
};

} // namespace fge

#endif // _FGE_C_OBJTILELAYER_HPP_INCLUDED
