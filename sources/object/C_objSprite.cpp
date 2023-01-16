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

#include "FastEngine/object/C_objSprite.hpp"

namespace fge
{

ObjSprite::ObjSprite(const fge::Texture& texture, const fge::Vector2f& position)
{
    this->setTexture(texture);
    this->setPosition(position);

    this->g_vertices.create(*fge::vulkan::GlobalContext, 4, 0, false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
}
ObjSprite::ObjSprite(const fge::Texture& texture, const fge::RectInt& rectangle, const fge::Vector2f& position)
{
    this->setTexture(texture);
    this->setTextureRect(rectangle);
    this->setPosition(position);
}

void ObjSprite::setTexture(const fge::Texture& texture, bool resetRect)
{
    // Recompute the texture area if requested, or if there was no valid texture & rect before
    if (resetRect || !this->g_texture.valid())
    {
        this->setTextureRect(fge::RectInt({0, 0}, {texture.getTextureSize().x, texture.getTextureSize().y}));
    }

    // Assign the new texture
    this->g_texture = texture;
}
void ObjSprite::setTextureRect(const fge::RectInt& rectangle)
{
    if (rectangle != this->g_textureRect)
    {
        this->g_textureRect = rectangle;
        this->updatePositions();
        this->updateTexCoords();
    }
}

void ObjSprite::setColor(const fge::Color& color)
{
    this->g_vertices.getVertices()[0]._color = color;
    this->g_vertices.getVertices()[1]._color = color;
    this->g_vertices.getVertices()[2]._color = color;
    this->g_vertices.getVertices()[3]._color = color;
}

const fge::Texture& ObjSprite::getTexture() const
{
    return this->g_texture;
}
const fge::RectInt& ObjSprite::getTextureRect() const
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
    auto copyStates = states.copy(this->_transform.start(*this, states._transform));

    copyStates._vertexBuffer = &this->g_vertices;
    copyStates._textureImage = static_cast<const fge::vulkan::TextureImage*>(this->g_texture);
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
    this->g_texture = jsonObject.value<std::string>("texture", FGE_TEXTURE_BAD);
    this->setTexture(this->g_texture, true);
}

void ObjSprite::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << fge::Color(this->g_vertices.getVertices()[0]._color) << this->g_texture;
}
void ObjSprite::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    fge::Color color;
    pck >> color >> this->g_texture;
    this->setColor(color);
}

const char* ObjSprite::getClassName() const
{
    return FGE_OBJSPRITE_CLASSNAME;
}
const char* ObjSprite::getReadableClassName() const
{
    return "sprite";
}

fge::RectFloat ObjSprite::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjSprite::getLocalBounds() const
{
    const auto width = static_cast<float>(std::abs(this->g_textureRect._width));
    const auto height = static_cast<float>(std::abs(this->g_textureRect._height));

    return {{0.f, 0.f}, {width, height}};
}

void ObjSprite::updatePositions()
{
    const fge::RectFloat bounds = this->getLocalBounds();

    this->g_vertices.getVertices()[0]._position = fge::Vector2f(0, 0);
    this->g_vertices.getVertices()[1]._position = fge::Vector2f(0, bounds._height);
    this->g_vertices.getVertices()[2]._position = fge::Vector2f(bounds._width, 0);
    this->g_vertices.getVertices()[3]._position = fge::Vector2f(bounds._width, bounds._height);
}

void ObjSprite::updateTexCoords()
{
    const float left = static_cast<float>(this->g_textureRect._x);
    const float right = left + static_cast<float>(this->g_textureRect._width);
    const float top = static_cast<float>(this->g_textureRect._y);
    const float bottom = top + static_cast<float>(this->g_textureRect._height);

    this->g_vertices.getVertices()[0]._texCoords = fge::Vector2f(left, top);
    this->g_vertices.getVertices()[1]._texCoords = fge::Vector2f(left, bottom);
    this->g_vertices.getVertices()[2]._texCoords = fge::Vector2f(right, top);
    this->g_vertices.getVertices()[3]._texCoords = fge::Vector2f(right, bottom);
}

} // namespace fge
