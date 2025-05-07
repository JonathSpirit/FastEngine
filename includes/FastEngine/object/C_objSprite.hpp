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

#ifndef _FGE_C_OBJSPRITE_HPP_INCLUDED
#define _FGE_C_OBJSPRITE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/accessor/C_texture.hpp"

#define FGE_OBJSPRITE_CLASSNAME "FGE:OBJ:SPRITE"

namespace fge
{

class FGE_API ObjSprite : public fge::Object
{
public:
    ObjSprite();
    explicit ObjSprite(fge::Texture const& texture, fge::Vector2f const& position = fge::Vector2f());
    ObjSprite(fge::Texture const& texture,
              fge::RectInt const& rectangle,
              fge::Vector2f const& position = fge::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSprite)

    void setTexture(fge::Texture const& texture, bool resetRect = false);
    void setTextureRect(fge::RectInt const& rectangle);
    void flipHorizontal();
    void flipVertical();

    void setColor(fge::Color const& color);

    fge::Texture const& getTexture() const;
    fge::RectInt const& getTextureRect() const;

    fge::Color getColor() const;

    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject) override;
    void load(nlohmann::json& jsonObject, std::filesystem::path const& filePath) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet const& pck) override;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

private:
    void updatePositions();
    void updateTexCoords();

    fge::vulkan::VertexBuffer g_vertices;
    fge::Texture g_texture;
    fge::RectInt g_textureRect;
};

} // namespace fge

#endif // _FGE_C_OBJSPRITE_HPP_INCLUDED
