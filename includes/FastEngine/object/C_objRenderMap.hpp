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
#include "C_lightObstacle.hpp"
#include "C_object.hpp"
#include "FastEngine/C_tunnel.hpp"
#include "FastEngine/graphic/C_renderTexture.hpp"

#define FGE_OBJRENDERMAP_CLASSNAME "FGE:OBJ:RENDERMAP"

namespace fge
{

class FGE_API ObjRenderMap : public fge::Object, public fge::Subscriber
{
public:
    ObjRenderMap();
    ObjRenderMap(const fge::ObjRenderMap& r);
    ObjRenderMap(fge::ObjRenderMap& r);

    fge::Object* copy() override { return new fge::ObjRenderMap(); }

    void onDraw(const fge::Scene* scene, fge::RenderTarget& target);

    void setClearColor(const fge::Color& color);
    const fge::Color& getClearColor() const;

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

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

    mutable fge::RenderTexture _renderTexture;

private:
    void updatePositions();
    void updateTexCoords();

    fge::Color g_colorClear;

    fge::vulkan::VertexBuffer g_vertexBuffer;
    fge::View g_windowView;
    fge::Vector2u g_windowSize;
};

} // namespace fge

#endif // _FGE_C_OBJRENDERMAP_HPP_INCLUDED