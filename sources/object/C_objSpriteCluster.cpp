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

#include "FastEngine/object/C_objSpriteCluster.hpp"

namespace fge
{

ObjSpriteCluster::ObjSpriteCluster() :
        g_needBuffersUpdate(true)
{
    this->g_instancesVertices.create(*fge::vulkan::GlobalContext, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                     fge::vulkan::BufferTypes::LOCAL);
}
ObjSpriteCluster::ObjSpriteCluster(const ObjSpriteCluster& r) :
        fge::Object(r),
        g_texture(r.g_texture),
        g_instancesData(r.g_instancesData),
        g_needBuffersUpdate(true)
{
    this->g_instancesVertices.create(*fge::vulkan::GlobalContext, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                     fge::vulkan::BufferTypes::LOCAL);
}
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
    this->g_needBuffersUpdate = true;
}
void ObjSpriteCluster::addSprite(const fge::RectInt& rectangle, const fge::Vector2f& offset)
{
    this->g_instancesData.emplace_back(rectangle, offset);
    this->g_instancesVertices.resize(this->g_instancesVertices.getCount() + 6);
    this->updatePositions(this->g_instancesData.size() - 1);
    this->updateTexCoords(this->g_instancesData.size() - 1);
}
void ObjSpriteCluster::resize(std::size_t size)
{
    this->g_instancesData.resize(size);
    this->g_needBuffersUpdate = true;
}
void ObjSpriteCluster::setTextureRect(std::size_t index, const fge::RectInt& rectangle)
{
    if (index < this->g_instancesData.size())
    {
        if (rectangle != this->g_instancesData[index]._textureRect)
        {
            this->g_instancesData[index]._textureRect = rectangle;
            if (!this->g_needBuffersUpdate)
            {
                this->updatePositions(index);
                this->updateTexCoords(index);
            }
        }
    }
}
void ObjSpriteCluster::setColor(std::size_t index, const fge::Color& color)
{
    if (index < this->g_instancesData.size() && !this->g_needBuffersUpdate)
    {
        const std::size_t startIndex = index * 6;

        this->g_instancesVertices[startIndex]._color = color;
        this->g_instancesVertices[startIndex + 1]._color = color;
        this->g_instancesVertices[startIndex + 2]._color = color;
        this->g_instancesVertices[startIndex + 3]._color = color;
        this->g_instancesVertices[startIndex + 4]._color = color;
        this->g_instancesVertices[startIndex + 5]._color = color;
    }
}
void ObjSpriteCluster::setOffset(std::size_t index, const fge::Vector2f& offset)
{
    if (index < this->g_instancesData.size())
    {
        if (offset != this->g_instancesData[index]._offset)
        {
            this->g_instancesData[index]._offset = offset;
            if (!this->g_needBuffersUpdate)
            {
                this->updatePositions(index);
                this->updateTexCoords(index);
            }
        }
    }
}

const fge::Texture& ObjSpriteCluster::getTexture() const
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
    if (index < this->g_instancesData.size() && !this->g_needBuffersUpdate)
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

    this->updateBuffers();

    auto copyStates = states.copy(this->_transform.start(*this, states._transform));
    copyStates._textureImage = static_cast<const fge::vulkan::TextureImage*>(this->g_texture);
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
void ObjSpriteCluster::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_texture;
}

const char* ObjSpriteCluster::getClassName() const
{
    return FGE_OBJSPRITECLUSTER_CLASSNAME;
}
const char* ObjSpriteCluster::getReadableClassName() const
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
        const auto width = static_cast<float>(this->g_instancesData[index]._textureRect._width);
        const auto height = static_cast<float>(this->g_instancesData[index]._textureRect._height);

        return fge::RectFloat{{0.f, 0.f}, {width, height}};
    }
    return std::nullopt;
}

void ObjSpriteCluster::updatePositions(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        const auto offset = this->g_instancesData[index]._offset;
        const fge::RectFloat bounds = this->getLocalBounds(index).value();
        const std::size_t startIndex = index * 6;

        this->g_instancesVertices[startIndex]._position = offset;
        this->g_instancesVertices[startIndex + 1]._position = fge::Vector2f(0, bounds._height) + offset;
        this->g_instancesVertices[startIndex + 2]._position = fge::Vector2f(bounds._width, 0) + offset;
        this->g_instancesVertices[startIndex + 3]._position = fge::Vector2f(bounds._width, bounds._height) + offset;
        this->g_instancesVertices[startIndex + 4]._position = fge::Vector2f(0, bounds._height) + offset;
        this->g_instancesVertices[startIndex + 5]._position = fge::Vector2f(bounds._width, 0) + offset;
    }
}

void ObjSpriteCluster::updateTexCoords(std::size_t index) const
{
    if (index < this->g_instancesData.size())
    {
        const auto rect =
                this->g_texture.getData()->_texture->normalizeTextureRect(this->g_instancesData[index]._textureRect);
        const std::size_t startIndex = index * 6;

        this->g_instancesVertices[startIndex]._texCoords = fge::Vector2f(rect._x, rect._y);
        this->g_instancesVertices[startIndex + 1]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
        this->g_instancesVertices[startIndex + 2]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
        this->g_instancesVertices[startIndex + 3]._texCoords =
                fge::Vector2f(rect._x + rect._width, rect._y + rect._height);
        this->g_instancesVertices[startIndex + 4]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
        this->g_instancesVertices[startIndex + 5]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
    }
}
void ObjSpriteCluster::updateBuffers() const
{
    if (this->g_needBuffersUpdate)
    {
        this->g_needBuffersUpdate = false;

        if (this->g_instancesVertices.getCount() != this->g_instancesData.size() * 6 && !this->g_instancesData.empty())
        {
            this->g_instancesVertices.resize(this->g_instancesData.size() * 6);

            for (std::size_t i = 0; i < this->g_instancesData.size(); ++i)
            {
                this->updatePositions(i);
                this->updateTexCoords(i);
            }
        }
    }
}

} // namespace fge
