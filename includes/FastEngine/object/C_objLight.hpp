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

#ifndef _FGE_C_OBJLIGHT_HPP_INCLUDED
#define _FGE_C_OBJLIGHT_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "C_lightObstacle.hpp"
#include "C_lightSystem.hpp"
#include "C_objRenderMap.hpp"
#include "C_object.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/C_texture.hpp"

#define FGE_OBJLIGHT_CLASSNAME "FGE:OBJ:LIGHT"

namespace fge
{

class FGE_API ObjLight : public fge::Object, public fge::LightComponent
{
public:
    ObjLight();
    explicit ObjLight(const fge::Texture& texture, const fge::Vector2f& position = fge::Vector2f());
    ObjLight(const fge::Texture& texture,
             const fge::RectInt& rectangle,
             const fge::Vector2f& position = fge::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjLight)

    void setBlendMode(const fge::vulkan::BlendMode& blendMode);
    const fge::vulkan::BlendMode& getBlendMode() const;

    void setTexture(const fge::Texture& texture, bool resetRect = false);
    void setTextureRect(const fge::RectInt& rectangle);

    void setRenderObject(const fge::ObjectDataShared& obj);
    const fge::ObjectDataShared& getRenderObject() const;

    void setColor(const fge::Color& color);

    const fge::Texture& getTexture() const;
    const fge::RectInt& getTextureRect() const;

    fge::Color getColor() const;

    void first(fge::Scene* scene) override;
    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

private:
    void updatePositions();
    void updateTexCoords();

    fge::vulkan::VertexBuffer g_vertexBuffer;
    fge::Texture g_texture;
    fge::RectInt g_textureRect;

    fge::ObjectDataShared g_renderObject;

    fge::Transform g_emptyTransform;
    mutable std::vector<fge::vulkan::VertexBuffer> g_obstacleHulls;

#ifndef FGE_DEF_SERVER
    fge::ObjRenderMap g_renderMap;
#endif //FGE_DEF_SERVER
    fge::vulkan::BlendMode g_blendMode;
};

} // namespace fge

#endif // _FGE_C_OBJLIGHT_HPP_INCLUDED