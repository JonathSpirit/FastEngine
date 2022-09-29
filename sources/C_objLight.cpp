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

#include "FastEngine/C_objLight.hpp"
#include "FastEngine/C_lightObstacle.hpp"
#include "FastEngine/C_lightSystem.hpp"
#include "FastEngine/extra_function.hpp"

#include "FastEngine/C_objRenderMap.hpp"

namespace fge
{

ObjLight::ObjLight()
{
    this->g_blendMode = sf::BlendAlpha;
}
ObjLight::ObjLight(const fge::Texture& texture, const sf::Vector2f& position)
{
    this->g_blendMode = sf::BlendAlpha;
    this->setTexture(texture);
    this->setPosition(position);
}
ObjLight::ObjLight(const fge::Texture& texture, const sf::IntRect& rectangle, const sf::Vector2f& position)
{
    this->g_blendMode = sf::BlendAlpha;
    this->setTexture(texture);
    this->setTextureRect(rectangle);
    this->setPosition(position);
}

void ObjLight::setBlendMode(const sf::BlendMode& blendMode)
{
    this->g_blendMode = blendMode;
}
const sf::BlendMode& ObjLight::getBlendMode() const
{
    return this->g_blendMode;
}

void ObjLight::setTexture(const fge::Texture& texture, bool resetRect)
{
    // Recompute the texture area if requested, or if there was no valid texture & rect before
    if ( resetRect || !this->g_texture.valid() )
    {
        this->setTextureRect( sf::IntRect(0, 0, texture.getTextureSize().x, texture.getTextureSize().y) );
    }

    // Assign the new texture
    this->g_texture = texture;
    this->setOrigin( static_cast<float>(this->g_textureRect.width)/2.0f, static_cast<float>(this->g_textureRect.height)/2.0f );
}
void ObjLight::setTextureRect(const sf::IntRect& rectangle)
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

void ObjLight::setColor(const sf::Color& color)
{
    this->g_vertices[0].color = color;
    this->g_vertices[1].color = color;
    this->g_vertices[2].color = color;
    this->g_vertices[3].color = color;
}

const fge::Texture& ObjLight::getTexture() const
{
    return this->g_texture;
}
const sf::IntRect& ObjLight::getTextureRect() const
{
    return this->g_textureRect;
}

const sf::Color& ObjLight::getColor() const
{
    return this->g_vertices[0].color;
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
    this->g_renderMap._renderTexture.clear(sf::Color(255,255,255,0));

    states.transform *= this->getTransform();
    states.texture = static_cast<const sf::Texture*>(this->g_texture);
    states.blendMode = sf::BlendAlpha;

    this->g_renderMap._renderTexture.draw(this->g_vertices, 4, sf::TriangleStrip, states);

    if ( this->_g_lightSystemGate.isOpen() )
    {
        fge::LightSystem* lightSystem = this->_g_lightSystemGate.getTunnel();

        sf::BlendMode noLightBlend = sf::BlendMode(sf::BlendMode::Factor::One, sf::BlendMode::Factor::One, sf::BlendMode::Equation::Add,
                                                sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::Zero, sf::BlendMode::Equation::Add);

        sf::FloatRect bounds = this->getGlobalBounds();
        float range = (bounds.width > bounds.height) ? bounds.width : bounds.height;

        for ( unsigned int i=0; i<lightSystem->getGatesSize(); ++i )
        {
            fge::LightObstacle* obstacle = lightSystem->get(i);

            std::size_t passCount = 0;

            static std::vector<sf::Vector2f> tmpHull;
            tmpHull.resize( obstacle->_g_myPoints.size()*2 );
            for ( std::size_t a=0; a<obstacle->_g_myPoints.size(); ++a )
            {
                float distance = range - fge::GetDistanceBetween(obstacle->_g_myPoints[a], this->getPosition());
                if (distance < 0)
                {
                    ++passCount;
                    distance = std::abs(distance) + range;
                }

                sf::Vector2f direction = fge::NormalizeVector2(obstacle->_g_myPoints[a] - this->getPosition());
                tmpHull[a] = sf::Vector2f( obstacle->_g_myPoints[a].x+direction.x*distance, obstacle->_g_myPoints[a].y+direction.y*distance );
                tmpHull[a+obstacle->_g_myPoints.size()] = obstacle->_g_myPoints[a];
            }
            if (passCount >= obstacle->_g_myPoints.size())
            {
                continue;
            }
            fge::GetConvexHull(tmpHull, tmpHull);

            sf::VertexArray polygon(sf::PrimitiveType::TriangleFan, tmpHull.size());
            for ( std::size_t a=0; a<tmpHull.size(); ++a )
            {
                polygon[a].position = tmpHull[a];
                polygon[a].color = sf::Color(255, 255, 255, 255);
            }

            this->g_renderMap._renderTexture.draw( polygon, sf::RenderStates(noLightBlend) );
        }
    }

    sf::RenderTarget* theTarget;
    if (this->g_renderObject)
    {
        theTarget = &reinterpret_cast<fge::ObjRenderMap*>(this->g_renderObject->getObject())->_renderTexture;
    }
    else
    {
        theTarget = &target;
    }

    theTarget->draw(this->g_renderMap, sf::RenderStates(this->g_blendMode));
}
#endif

void ObjLight::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["color"] = this->g_vertices[0].color.toInteger();
    jsonObject["texture"] = this->g_texture;
}
void ObjLight::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->setColor( sf::Color( jsonObject.value<uint32_t>("color", 0) ) );
    this->g_texture = jsonObject.value<std::string>("texture", FGE_TEXTURE_BAD);
}

void ObjLight::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_vertices[0].color << this->g_texture;
}
void ObjLight::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    sf::Color color;
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

sf::FloatRect ObjLight::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect ObjLight::getLocalBounds() const
{
    float width = static_cast<float>( std::abs(this->g_textureRect.width) );
    float height = static_cast<float>( std::abs(this->g_textureRect.height) );

    return {0.f, 0.f, width, height};
}

void ObjLight::updatePositions()
{
    sf::FloatRect bounds = this->getLocalBounds();

    this->g_vertices[0].position = sf::Vector2f(0, 0);
    this->g_vertices[1].position = sf::Vector2f(0, bounds.height);
    this->g_vertices[2].position = sf::Vector2f(bounds.width, 0);
    this->g_vertices[3].position = sf::Vector2f(bounds.width, bounds.height);
}

void ObjLight::updateTexCoords()
{
    float left   = static_cast<float>(this->g_textureRect.left);
    float right  = left + static_cast<float>(this->g_textureRect.width);
    float top    = static_cast<float>(this->g_textureRect.top);
    float bottom = top + static_cast<float>(this->g_textureRect.height);

    this->g_vertices[0].texCoords = sf::Vector2f(left, top);
    this->g_vertices[1].texCoords = sf::Vector2f(left, bottom);
    this->g_vertices[2].texCoords = sf::Vector2f(right, top);
    this->g_vertices[3].texCoords = sf::Vector2f(right, bottom);
}

}//end fge
