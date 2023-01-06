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

#include "FastEngine/object/C_objLight.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/object/C_lightObstacle.hpp"
#include "FastEngine/object/C_lightSystem.hpp"

#include "FastEngine/object/C_objRenderMap.hpp"

namespace fge
{

ObjLight::ObjLight()
{
    this->g_vertexBuffer.create(*fge::vulkan::GlobalContext, 4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    this->g_blendMode = fge::vulkan::BlendAlpha;
}
ObjLight::ObjLight(const fge::Texture& texture, const fge::Vector2f& position) :
        fge::ObjLight()
{
    this->setTexture(texture);
    this->setPosition(position);
}
ObjLight::ObjLight(const fge::Texture& texture, const fge::RectInt& rectangle, const fge::Vector2f& position) :
        fge::ObjLight()
{
    this->setTexture(texture);
    this->setTextureRect(rectangle);
    this->setPosition(position);
}

void ObjLight::setBlendMode(const fge::vulkan::BlendMode& blendMode)
{
    this->g_blendMode = blendMode;
}
const fge::vulkan::BlendMode& ObjLight::getBlendMode() const
{
    return this->g_blendMode;
}

void ObjLight::setTexture(const fge::Texture& texture, bool resetRect)
{
    // Recompute the texture area if requested, or if there was no valid texture & rect before
    if (resetRect || !this->g_texture.valid())
    {
        this->g_texture = texture;
        this->setTextureRect(fge::RectInt({0, 0},
                                          {texture.getTextureSize().x, texture.getTextureSize().y}));
    }
    else
    {
        this->g_texture = texture;
    }

    this->setOrigin({static_cast<float>(this->g_textureRect._width) / 2.0f,
                    static_cast<float>(this->g_textureRect._height) / 2.0f});
}
void ObjLight::setTextureRect(const fge::RectInt& rectangle)
{
    if (rectangle != this->g_textureRect)
    {
        this->g_textureRect = rectangle;
        this->updatePositions();
        this->updateTexCoords();
    }
}

void ObjLight::setRenderObject(const fge::ObjectDataShared& obj)
{
    this->g_renderObject = obj;
}
const fge::ObjectDataShared& ObjLight::getRenderObject() const
{
    return this->g_renderObject;
}

void ObjLight::setColor(const fge::Color& color)
{
    this->g_vertexBuffer.getVertices()[0]._color = color;
    this->g_vertexBuffer.getVertices()[1]._color = color;
    this->g_vertexBuffer.getVertices()[2]._color = color;
    this->g_vertexBuffer.getVertices()[3]._color = color;
}

const fge::Texture& ObjLight::getTexture() const
{
    return this->g_texture;
}
const fge::RectInt& ObjLight::getTextureRect() const
{
    return this->g_textureRect;
}

fge::Color ObjLight::getColor() const
{
    return fge::Color(this->g_vertexBuffer.getVertices()[0]._color);
}

void ObjLight::first(fge::Scene* scene)
{
    if (scene != nullptr && !this->g_renderObject)
    {
        this->g_renderObject = scene->getFirstObj_ByClass(FGE_OBJLIGHTMAP_CLASSNAME);
    }
    if (!this->_g_lightSystemGate.isOpen())
    {
        this->setDefaultLightSystem(scene);
    }
}

FGE_OBJ_UPDATE_BODY(ObjLight)
{
#ifndef FGE_DEF_SERVER
    FGE_OBJ_UPDATE_CALL(this->g_renderMap);
#endif //FGE_DEF_SERVER
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjLight)
{
    this->g_renderMap._renderTexture.setClearColor(fge::Color(0,0,0,0));
    this->g_renderMap._renderTexture.beginRenderPass(this->g_renderMap._renderTexture.prepareNextFrame(nullptr));

    auto copyStates = states.copy(this);
    copyStates._modelTransform *= this->getTransform();
    copyStates._textureImage = static_cast<const fge::TextureType*>(this->g_texture);
    copyStates._blendMode =
            fge::vulkan::BlendMode{VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
                                   VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD};

    copyStates._vertexBuffer = &this->g_vertexBuffer;

    this->g_renderMap._renderTexture.RenderTarget::draw(copyStates);

    if (this->_g_lightSystemGate.isOpen())
    {
        fge::LightSystem* lightSystem = this->_g_lightSystemGate.getTunnel();

        const fge::vulkan::BlendMode noLightBlend =
                fge::vulkan::BlendMode(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
                                       VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

        const fge::RectFloat bounds = this->getGlobalBounds();
        const float range = (bounds._width > bounds._height) ? bounds._width : bounds._height;

        for (std::size_t i = 0; i < lightSystem->getGatesSize(); ++i)
        {
            fge::LightObstacle* obstacle = lightSystem->get(i);

            std::size_t passCount = 0;

            static std::vector<fge::Vector2f> tmpHull;
            tmpHull.resize(obstacle->_g_myPoints.size() * 2);
            for (std::size_t a = 0; a < obstacle->_g_myPoints.size(); ++a)
            {
                float distance = range - fge::GetDistanceBetween(obstacle->_g_myPoints[a], this->getPosition());
                if (distance < 0)
                {
                    ++passCount;
                    distance = std::abs(distance) + range;
                }

                fge::Vector2f direction = fge::NormalizeVector2(obstacle->_g_myPoints[a] - this->getPosition());
                tmpHull[a] = fge::Vector2f(obstacle->_g_myPoints[a].x + direction.x * distance,
                                           obstacle->_g_myPoints[a].y + direction.y * distance);
                tmpHull[a + obstacle->_g_myPoints.size()] = obstacle->_g_myPoints[a];
            }
            if (passCount >= obstacle->_g_myPoints.size())
            {
                continue;
            }
            fge::GetConvexHull(tmpHull, tmpHull);

            fge::vulkan::VertexBuffer polygon;
            fge::Transformable polygonTransform;
            polygon.create(*fge::vulkan::GlobalContext, tmpHull.size(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, fge::vulkan::VertexBuffer::Types::VERTEX_BUFFER);
            for (std::size_t a = 0; a < tmpHull.size(); ++a)
            {
                polygon.getVertices()[a]._position = tmpHull[a];
                polygon.getVertices()[a]._color = fge::Color(255, 255, 255, 255);
            }

            auto polygonStates = fge::RenderStates(&polygonTransform, &polygon);
            polygonStates._blendMode = noLightBlend;
            this->g_renderMap._renderTexture.RenderTarget::draw(polygonStates);
        }
    }

    fge::RenderTarget* theTarget;
    if (this->g_renderObject)
    {
        theTarget = &reinterpret_cast<fge::ObjRenderMap*>(this->g_renderObject->getObject())->_renderTexture;
    }
    else
    {
        theTarget = &target;
    }

    auto targetStates = fge::RenderStates(this);
    targetStates._blendMode = this->g_blendMode;
    theTarget->draw(this->g_renderMap, targetStates);
}
#endif

void ObjLight::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["color"] = this->g_vertexBuffer.getVertices()[0]._color;
    jsonObject["texture"] = this->g_texture;
}
void ObjLight::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->setColor(fge::Color(jsonObject.value<uint32_t>("color", 0)));
    this->g_texture = jsonObject.value<std::string>("texture", FGE_TEXTURE_BAD);
}

void ObjLight::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_vertexBuffer.getVertices()[0]._color << this->g_texture;
}
void ObjLight::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    fge::Color color;
    pck >> color >> this->g_texture;
    this->setColor(color);
}

const char* ObjLight::getClassName() const
{
    return FGE_OBJLIGHT_CLASSNAME;
}
const char* ObjLight::getReadableClassName() const
{
    return "light";
}

fge::RectFloat ObjLight::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjLight::getLocalBounds() const
{
    auto width = static_cast<float>(std::abs(this->g_textureRect._width));
    auto height = static_cast<float>(std::abs(this->g_textureRect._height));

    return {{0.f, 0.f}, {width, height}};
}

void ObjLight::updatePositions()
{
    const fge::RectFloat bounds = this->getLocalBounds();

    this->g_vertexBuffer.getVertices()[0]._position = fge::Vector2f(0, 0);
    this->g_vertexBuffer.getVertices()[1]._position = fge::Vector2f(0, bounds._height);
    this->g_vertexBuffer.getVertices()[2]._position = fge::Vector2f(bounds._width, 0);
    this->g_vertexBuffer.getVertices()[3]._position = fge::Vector2f(bounds._width, bounds._height);
}

void ObjLight::updateTexCoords()
{
    auto rect = this->g_texture.getData()->_texture->normalizeTextureRect(this->g_textureRect);

    this->g_vertexBuffer.getVertices()[0]._texCoords = fge::Vector2f(rect._x, rect._y);
    this->g_vertexBuffer.getVertices()[1]._texCoords = fge::Vector2f(rect._x, rect._height);
    this->g_vertexBuffer.getVertices()[2]._texCoords = fge::Vector2f(rect._width, rect._y);
    this->g_vertexBuffer.getVertices()[3]._texCoords = fge::Vector2f(rect._width, rect._height);
}

} // namespace fge
