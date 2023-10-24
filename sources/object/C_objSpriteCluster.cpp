/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/object/C_objSpriteCluster.hpp"

namespace fge
{

ObjSpriteCluster::ObjSpriteCluster() :
        g_instancesVertices(fge::vulkan::GetActiveContext())
{
    this->g_instancesVertices.create(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, fge::vulkan::BufferTypes::LOCAL);
}
ObjSpriteCluster::ObjSpriteCluster(ObjSpriteCluster const& r) :
        fge::Object(r),
        g_texture(r.g_texture),
        g_instancesData(r.g_instancesData),
        g_instancesVertices(r.g_instancesVertices)
{}
ObjSpriteCluster::ObjSpriteCluster(fge::Texture texture) :
        ObjSpriteCluster()
{
    this->setTexture(std::move(texture));
}

void ObjSpriteCluster::setTexture(fge::Texture texture)
{
    this->g_texture = std::move(texture);
}

void ObjSpriteCluster::clear()
{
    this->g_instancesData.clear();
    this->g_instancesVertices.clear();
}
void ObjSpriteCluster::addSprite(fge::RectInt const& rectangle, fge::Vector2f const& offset)
{
    this->g_instancesData.emplace_back(rectangle, offset);
    this->g_instancesVertices.resize(this->g_instancesVertices.getCount() + 6);
    this->updatePositions(this->g_instancesData.size() - 1);
    this->updateTexCoords(this->g_instancesData.size() - 1);
}
void ObjSpriteCluster::resize(std::size_t size)
{
    std::size_t const oldSize = this->g_instancesData.size();

    this->g_instancesData.resize(size);
    this->g_instancesVertices.resize(size * 6);

    if (size > oldSize)
    {
        for (std::size_t i = oldSize; i < size; ++i)
        {
            this->updatePositions(i);
            this->updateTexCoords(i);
        }
    }
}
void ObjSpriteCluster::setTextureRect(std::size_t index, fge::RectInt const& rectangle)
{
    if (index < this->g_instancesData.size())
    {
        if (rectangle != this->g_instancesData[index]._textureRect)
        {
            this->g_instancesData[index]._textureRect = rectangle;
            this->updatePositions(index);
            this->updateTexCoords(index);
        }
    }
}
void ObjSpriteCluster::setColor(std::size_t index, fge::Color const& color)
{
    if (index < this->g_instancesData.size())
    {
        std::size_t const startIndex = index * 6;

        this->g_instancesVertices[startIndex]._color = color;
        this->g_instancesVertices[startIndex + 1]._color = color;
        this->g_instancesVertices[startIndex + 2]._color = color;
        this->g_instancesVertices[startIndex + 3]._color = color;
        this->g_instancesVertices[startIndex + 4]._color = color;
        this->g_instancesVertices[startIndex + 5]._color = color;
    }
}
void ObjSpriteCluster::setOffset(std::size_t index, fge::Vector2f const& offset)
{
    if (index < this->g_instancesData.size())
    {
        if (offset != this->g_instancesData[index]._offset)
        {
            this->g_instancesData[index]._offset = offset;
            this->updatePositions(index);
        }
    }
}

fge::Texture const& ObjSpriteCluster::getTexture() const
{
    return this->g_texture;
}

std::optional<fge::RectInt> ObjSpriteCluster::getTextureRect(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        return this->g_instancesData[index]._textureRect;
    }
    return std::nullopt;
}
std::optional<fge::Color> ObjSpriteCluster::getColor(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        return fge::Color(this->g_instancesVertices[index * 6]._color);
    }
    return std::nullopt;
}
std::optional<fge::Vector2f> ObjSpriteCluster::getOffset(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        return this->g_instancesData[index]._offset;
    }
    return std::nullopt;
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSpriteCluster)
{
    if (this->g_instancesData.empty())
    {
        return;
    }

    auto copyStates = states.copy(this->_transform.start(*this, states._resTransform.get()));
    copyStates._resTextures.set(this->g_texture.retrieve(), 1);
    copyStates._vertexBuffer = &this->g_instancesVertices;

    target.draw(copyStates);
}
#endif

void ObjSpriteCluster::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["texture"] = this->g_texture;
}
void ObjSpriteCluster::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->g_texture = jsonObject.value<std::string>("texture", FGE_TEXTURE_BAD);
}

void ObjSpriteCluster::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_texture;
}
void ObjSpriteCluster::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_texture;
}

char const* ObjSpriteCluster::getClassName() const
{
    return FGE_OBJSPRITECLUSTER_CLASSNAME;
}
char const* ObjSpriteCluster::getReadableClassName() const
{
    return "sprite cluster";
}

fge::RectFloat ObjSpriteCluster::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
std::optional<fge::RectFloat> ObjSpriteCluster::getGlobalBounds(std::size_t index) const
{
    auto localBounds = this->getLocalBounds(index);
    if (localBounds)
    {
        return this->getTransform() * localBounds.value();
    }
    return std::nullopt;
}
fge::RectFloat ObjSpriteCluster::getLocalBounds() const
{
    return fge::Object::getLocalBounds();
}
std::optional<fge::RectFloat> ObjSpriteCluster::getLocalBounds(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        auto const width = static_cast<float>(this->g_instancesData[index]._textureRect._width);
        auto const height = static_cast<float>(this->g_instancesData[index]._textureRect._height);

        return fge::RectFloat{{0.f, 0.f}, {width, height}};
    }
    return std::nullopt;
}

void ObjSpriteCluster::updatePositions(std::size_t index)
{
    if (index < this->g_instancesData.size())
    {
        auto const offset = this->g_instancesData[index]._offset;
        fge::RectFloat const bounds = this->getLocalBounds(index).value();
        std::size_t const startIndex = index * 6;

        this->g_instancesVertices[startIndex]._position = offset;
        this->g_instancesVertices[startIndex + 1]._position = fge::Vector2f(0, bounds._height) + offset;
        this->g_instancesVertices[startIndex + 2]._position = fge::Vector2f(bounds._width, 0) + offset;
        this->g_instancesVertices[startIndex + 3]._position = fge::Vector2f(bounds._width, bounds._height) + offset;
        this->g_instancesVertices[startIndex + 4]._position = fge::Vector2f(0, bounds._height) + offset;
        this->g_instancesVertices[startIndex + 5]._position = fge::Vector2f(bounds._width, 0) + offset;
    }
}

void ObjSpriteCluster::updateTexCoords(std::size_t index)
{
    if (index < this->g_instancesData.size())
    {
        auto const rect =
                this->g_texture.getData()->_texture->normalizeTextureRect(this->g_instancesData[index]._textureRect);
        std::size_t const startIndex = index * 6;

        this->g_instancesVertices[startIndex]._texCoords = fge::Vector2f(rect._x, rect._y);
        this->g_instancesVertices[startIndex + 1]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
        this->g_instancesVertices[startIndex + 2]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
        this->g_instancesVertices[startIndex + 3]._texCoords =
                fge::Vector2f(rect._x + rect._width, rect._y + rect._height);
        this->g_instancesVertices[startIndex + 4]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
        this->g_instancesVertices[startIndex + 5]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
    }
}

} // namespace fge
