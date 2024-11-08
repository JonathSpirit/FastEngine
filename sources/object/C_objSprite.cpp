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

#include "FastEngine/object/C_objSprite.hpp"

namespace fge
{

ObjSprite::ObjSprite() :
        g_vertices(fge::vulkan::GetActiveContext())
{
    this->g_vertices.create(4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, fge::vulkan::BufferTypes::LOCAL);
    this->setTextureRect(fge::RectInt({0, 0}, {FGE_TEXTURE_BAD_W, FGE_TEXTURE_BAD_H}));
}
ObjSprite::ObjSprite(fge::Texture const& texture, fge::Vector2f const& position) :
        ObjSprite()
{
    this->setTexture(texture);
    this->setPosition(position);
}
ObjSprite::ObjSprite(fge::Texture const& texture, fge::RectInt const& rectangle, fge::Vector2f const& position) :
        ObjSprite()
{
    this->setTexture(texture);
    this->setTextureRect(rectangle);
    this->setPosition(position);
}

void ObjSprite::setTexture(fge::Texture const& texture, bool resetRect)
{
    // Recompute the texture area if requested, or if there was no valid texture & rect before
    if (resetRect || !this->g_texture.valid())
    {
        this->g_texture = texture;
        this->setTextureRect(fge::RectInt({0, 0}, {texture.getTextureSize().x, texture.getTextureSize().y}));
    }
    else
    {
        this->g_texture = texture;
    }
}
void ObjSprite::setTextureRect(fge::RectInt const& rectangle)
{
    if (rectangle != this->g_textureRect)
    {
        this->g_textureRect = rectangle;
#ifndef FGE_DEF_SERVER
        this->updatePositions();
        this->updateTexCoords();
#endif
    }
}
void ObjSprite::flipHorizontal()
{
    this->g_textureRect = {{this->g_textureRect._x + this->g_textureRect._width, this->g_textureRect._y},
                           {-this->g_textureRect._width, this->g_textureRect._height}};
#ifndef FGE_DEF_SERVER
    this->updatePositions();
    this->updateTexCoords();
#endif
}
void ObjSprite::flipVertical()
{
    this->g_textureRect = {{this->g_textureRect._x, this->g_textureRect._y + this->g_textureRect._height},
                           {this->g_textureRect._width, -this->g_textureRect._height}};
#ifndef FGE_DEF_SERVER
    this->updatePositions();
    this->updateTexCoords();
#endif
}

void ObjSprite::setColor(fge::Color const& color)
{
    this->g_vertices.getVertices()[0]._color = color;
    this->g_vertices.getVertices()[1]._color = color;
    this->g_vertices.getVertices()[2]._color = color;
    this->g_vertices.getVertices()[3]._color = color;
}

fge::Texture const& ObjSprite::getTexture() const
{
    return this->g_texture;
}
fge::RectInt const& ObjSprite::getTextureRect() const
{
    return this->g_textureRect;
}

fge::Color ObjSprite::getColor() const
{
    return fge::Color(this->g_vertices.getVertices()[0]._color);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSprite)
{
    auto copyStates = states.copy();

    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));
    copyStates._vertexBuffer = &this->g_vertices;
    copyStates._resTextures.set(this->g_texture.retrieve(), 1);
    target.draw(copyStates);
}
#endif

void ObjSprite::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["color"] = fge::Color(this->g_vertices.getVertices()[0]._color).toInteger();
    jsonObject["texture"] = this->g_texture;
}
void ObjSprite::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->setColor(fge::Color(jsonObject.value<uint32_t>("color", 0)));
    this->g_texture = jsonObject.value<std::string>("texture", std::string{FGE_TEXTURE_BAD});
    this->setTexture(this->g_texture, true);
}

void ObjSprite::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << fge::Color(this->g_vertices.getVertices()[0]._color) << this->g_texture;
}
void ObjSprite::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    fge::Color color;
    pck >> color >> this->g_texture;
    this->setColor(color);
}

char const* ObjSprite::getClassName() const
{
    return FGE_OBJSPRITE_CLASSNAME;
}
char const* ObjSprite::getReadableClassName() const
{
    return "sprite";
}

fge::RectFloat ObjSprite::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjSprite::getLocalBounds() const
{
    auto const width = static_cast<float>(std::abs(this->g_textureRect._width));
    auto const height = static_cast<float>(std::abs(this->g_textureRect._height));

    return {{0.f, 0.f}, {width, height}};
}

void ObjSprite::updatePositions()
{
    fge::RectFloat const bounds = this->getLocalBounds();

    this->g_vertices[0]._position = fge::Vector2f(0, 0);
    this->g_vertices[1]._position = fge::Vector2f(0, bounds._height);
    this->g_vertices[2]._position = fge::Vector2f(bounds._width, 0);
    this->g_vertices[3]._position = fge::Vector2f(bounds._width, bounds._height);
}

void ObjSprite::updateTexCoords()
{
    auto const rect = this->g_texture.getSharedTexture()->normalizeTextureRect(this->g_textureRect);

    this->g_vertices[0]._texCoords = fge::Vector2f(rect._x, rect._y);
    this->g_vertices[1]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
    this->g_vertices[2]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
    this->g_vertices[3]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y + rect._height);
}

} // namespace fge
