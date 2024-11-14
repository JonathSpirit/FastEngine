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

#include "FastEngine/object/C_objLight.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/object/C_lightSystem.hpp"

#include "FastEngine/object/C_objRenderMap.hpp"

namespace fge
{

ObjLight::ObjLight() :
        g_vertexBuffer(fge::vulkan::GetActiveContext())
{
    this->g_vertexBuffer.create(4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    this->g_blendMode = fge::vulkan::BlendAlpha;
}
ObjLight::ObjLight(fge::Texture const& texture, fge::Vector2f const& position) :
        fge::ObjLight()
{
    this->setTexture(texture);
    this->setPosition(position);
}
ObjLight::ObjLight(fge::Texture const& texture, fge::RectInt const& rectangle, fge::Vector2f const& position) :
        fge::ObjLight()
{
    this->setTexture(texture);
    this->setTextureRect(rectangle);
    this->setPosition(position);
}

void ObjLight::setBlendMode(fge::vulkan::BlendMode const& blendMode)
{
    this->g_blendMode = blendMode;
}
fge::vulkan::BlendMode const& ObjLight::getBlendMode() const
{
    return this->g_blendMode;
}

void ObjLight::setTexture(fge::Texture const& texture, bool resetRect)
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

    this->setOrigin({static_cast<float>(this->g_textureRect._width) / 2.0f,
                     static_cast<float>(this->g_textureRect._height) / 2.0f});
}
void ObjLight::setTextureRect(fge::RectInt const& rectangle)
{
    if (rectangle != this->g_textureRect)
    {
        this->g_textureRect = rectangle;
        this->updatePositions();
        this->updateTexCoords();
    }
}

void ObjLight::setRenderObject(fge::ObjectDataShared const& obj)
{
    this->g_renderObject = obj;
}
fge::ObjectDataShared const& ObjLight::getRenderObject() const
{
    return this->g_renderObject;
}

void ObjLight::setColor(fge::Color const& color)
{
    this->g_vertexBuffer.getVertices()[0]._color = color;
    this->g_vertexBuffer.getVertices()[1]._color = color;
    this->g_vertexBuffer.getVertices()[2]._color = color;
    this->g_vertexBuffer.getVertices()[3]._color = color;
}

fge::Texture const& ObjLight::getTexture() const
{
    return this->g_texture;
}
fge::RectInt const& ObjLight::getTextureRect() const
{
    return this->g_textureRect;
}

fge::Color ObjLight::getColor() const
{
    return fge::Color(this->g_vertexBuffer.getVertices()[0]._color);
}

void ObjLight::first(fge::Scene& scene)
{
    if (!this->g_renderObject)
    {
        this->g_renderObject = scene.getFirstObj_ByClass(FGE_OBJRENDERMAP_CLASSNAME);
    }
    if (!this->_g_lightSystemGate.isOpen())
    {
        this->setDefaultLightSystem(scene);
    }

#ifndef FGE_DEF_SERVER
    this->g_renderMap._renderTexture.setClearColor(fge::Color(0, 0, 0, 0));
#endif //FGE_DEF_SERVER
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
    this->g_renderMap._renderTexture.beginRenderPass(
            this->g_renderMap._renderTexture.prepareNextFrame(nullptr, FGE_RENDER_TIMEOUT_BLOCKING));

    auto emptyTransform = target.getContext().requestGlobalTransform();
    emptyTransform.second->_modelTransform = glm::mat4{1.0f};
    emptyTransform.second->_viewTransform = target.getView().getProjection() * target.getView().getTransform();

    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));
    copyStates._resTextures.set(this->g_texture.retrieve(), 1);
    copyStates._blendMode = fge::vulkan::BlendNone;

    copyStates._vertexBuffer = &this->g_vertexBuffer;

    this->g_renderMap._renderTexture.draw(copyStates);

    if (this->_g_lightSystemGate.isOpen())
    {
        fge::LightSystem* lightSystem = this->_g_lightSystemGate.getTunnel();

        constexpr fge::vulkan::BlendMode noLightBlend =
                fge::vulkan::BlendMode(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO,
                                       VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

        fge::RectFloat const bounds = this->getGlobalBounds();
        float const range = (bounds._width > bounds._height) ? bounds._width : bounds._height;
        fge::Vector2f const center = bounds.getPosition() + bounds.getSize() / 2.0f;

        this->g_obstacleHulls.resize(lightSystem->getGatesSize(),
                                     fge::vulkan::VertexBuffer{fge::vulkan::GetActiveContext()});

        for (std::size_t iComponent = 0; iComponent < lightSystem->getGatesSize(); ++iComponent)
        {
            auto* lightComponent = lightSystem->get(iComponent);
            if (!lightComponent->isObstacle())
            {
                continue;
            }
            auto* obstacle = reinterpret_cast<fge::LightObstacle*>(lightComponent);

            //Update the shape if needed
            if (obstacle->getShape().subPolygonCount() == 0)
            {
                obstacle->updateObstacleShape();
            }

            std::size_t passCount = 0;

            //Check if the objects is too far from the light, (if so do nothing with it)
            //TODO: for now, we just check the first point of every subPolygon
            for (std::size_t iShape = 0; iShape < obstacle->getShape().subPolygonCount(); ++iShape)
            {
                auto point = *obstacle->getShape().subPolygon(iShape).begin();
                point = obstacle->getTransformableParent().getTransform() * point;

                float const distance = range - fge::GetDistanceBetween(point, center);
                if (distance < 0.0f)
                {
                    ++passCount;
                }
            }
            if (passCount >= obstacle->getShape().subPolygonCount())
            {
                continue;
            }

            this->g_obstacleHulls[iComponent].create(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
                                                     fge::vulkan::BufferTypes::LOCAL);

            static std::vector<fge::Vector2f> tmpHull;
            for (std::size_t iShape = 0; iShape < obstacle->getShape().subPolygonCount(); ++iShape)
            {
                auto const& shape = obstacle->getShape().subPolygon(iShape);

                tmpHull.resize(shape.size() * 2);
                for (std::size_t iVertex = 0; iVertex < shape.size(); ++iVertex)
                {
                    auto const vertex = obstacle->getTransformableParent().getTransform() * shape[iVertex];
                    float distance = range - fge::GetDistanceBetween(vertex, center);
                    if (distance < 0.0f)
                    {
                        distance = std::abs(distance) + range;
                    }

                    auto const direction = glm::normalize(vertex - center);
                    tmpHull[iVertex] =
                            fge::Vector2f(vertex.x + direction.x * distance, vertex.y + direction.y * distance);
                    tmpHull[iVertex + shape.size()] = vertex;
                }
                fge::GetConvexHull(tmpHull, tmpHull);

                auto const vertexOffset = this->g_obstacleHulls[iComponent].getCount();
                this->g_obstacleHulls[iComponent].resize(vertexOffset + tmpHull.size());
                for (std::size_t iVertex = 0; iVertex < tmpHull.size(); ++iVertex)
                {
                    this->g_obstacleHulls[iComponent][vertexOffset + iVertex]._position = tmpHull[iVertex];
                    this->g_obstacleHulls[iComponent][vertexOffset + iVertex]._color = fge::Color::White;
                }

                auto polygonStates = fge::RenderStates(&this->g_obstacleHulls[iComponent]);
                polygonStates._resTransform.set(emptyTransform.first);
                polygonStates._blendMode = noLightBlend;
                polygonStates._resInstances.setVertexCount(tmpHull.size());
                polygonStates._resInstances.setVertexOffset(vertexOffset);
                this->g_renderMap._renderTexture.draw(polygonStates);
            }
        }
    }

    fge::RenderTarget* finalTarget{&target};
    if (this->g_renderObject)
    {
        finalTarget = &reinterpret_cast<fge::ObjRenderMap*>(this->g_renderObject->getObject())->_renderTexture;
    }

    auto targetStates = fge::RenderStates();
    targetStates._resTransform.set(target.requestGlobalTransform(*this));
    targetStates._blendMode = this->g_blendMode;
    this->g_renderMap.draw(*finalTarget, targetStates);
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
    this->g_texture = jsonObject.value<std::string>("texture", std::string{FGE_TEXTURE_BAD});
}

void ObjLight::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_vertexBuffer.getVertices()[0]._color << this->g_texture;
}
void ObjLight::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    fge::Color color;
    pck >> color >> this->g_texture;
    this->setColor(color);
}

char const* ObjLight::getClassName() const
{
    return FGE_OBJLIGHT_CLASSNAME;
}
char const* ObjLight::getReadableClassName() const
{
    return "light";
}

fge::RectFloat ObjLight::getGlobalBounds() const
{
    return (this->getParentsTransform() * this->getTransform()) * this->getLocalBounds();
}
fge::RectFloat ObjLight::getLocalBounds() const
{
    auto width = static_cast<float>(std::abs(this->g_textureRect._width));
    auto height = static_cast<float>(std::abs(this->g_textureRect._height));

    return {{0.f, 0.f}, {width, height}};
}

void ObjLight::updatePositions()
{
    fge::RectFloat const bounds = this->getLocalBounds();

    this->g_vertexBuffer.getVertices()[0]._position = fge::Vector2f(0, 0);
    this->g_vertexBuffer.getVertices()[1]._position = fge::Vector2f(0, bounds._height);
    this->g_vertexBuffer.getVertices()[2]._position = fge::Vector2f(bounds._width, 0);
    this->g_vertexBuffer.getVertices()[3]._position = fge::Vector2f(bounds._width, bounds._height);
}

void ObjLight::updateTexCoords()
{
    auto rect = this->g_texture.getSharedData()->normalizeTextureRect(this->g_textureRect);

    this->g_vertexBuffer.getVertices()[0]._texCoords = fge::Vector2f(rect._x, rect._y);
    this->g_vertexBuffer.getVertices()[1]._texCoords = fge::Vector2f(rect._x, rect._y + rect._height);
    this->g_vertexBuffer.getVertices()[2]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y);
    this->g_vertexBuffer.getVertices()[3]._texCoords = fge::Vector2f(rect._x + rect._width, rect._y + rect._height);
}

} // namespace fge
