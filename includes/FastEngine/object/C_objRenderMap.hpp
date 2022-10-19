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

#ifndef _FGE_C_OBJRENDERMAP_HPP_INCLUDED
#define _FGE_C_OBJRENDERMAP_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_tunnel.hpp"
#include "C_lightObstacle.hpp"

#define FGE_OBJLIGHTMAP_CLASSNAME "FGE:OBJ:RENDERMAP"

namespace fge
{

class FGE_API ObjRenderMap : public fge::Object, public fge::Subscriber
{
public:
    ObjRenderMap() = default;
    ObjRenderMap(const fge::ObjRenderMap& r) :
        fge::Object(r),
        fge::Subscriber(r),
        g_colorClear(r.g_colorClear)
    {};
    ObjRenderMap(fge::ObjRenderMap& r) :
        fge::Object(r),
        fge::Subscriber(r),
        g_colorClear(r.g_colorClear)
    {};

    fge::Object* copy() override
    {
        return new fge::ObjRenderMap();
    }

    void onClear(const fge::Scene* scene, sf::RenderTarget& target, const sf::Color& color);

    void setClearColor(const sf::Color& color);
    const sf::Color& getClearColor() const;

    void first(fge::Scene* scene) override;
    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE
    void removed(fge::Scene* scene) override;

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

    mutable sf::RenderTexture _renderTexture;

private:

    void updatePositions();
    void updateTexCoords();

    sf::Color g_colorClear{sf::Color::White};

    sf::Vertex g_vertices[4];
    sf::View g_windowView;
    sf::Vector2u g_windowSize;
};

}//end fge

#endif // _FGE_C_OBJRENDERMAP_HPP_INCLUDED
