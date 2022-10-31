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

#include "FastEngine/object/C_objRenderMap.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

void ObjRenderMap::onClear([[maybe_unused]] const fge::Scene* scene, [[maybe_unused]] sf::RenderTarget& target, [[maybe_unused]] const sf::Color& color)
{
    this->_renderTexture.clear( this->g_colorClear );
}

void ObjRenderMap::setClearColor(const sf::Color& color)
{
    this->g_colorClear = color;
}
const sf::Color& ObjRenderMap::getClearColor() const
{
    return this->g_colorClear;
}

void ObjRenderMap::first(fge::Scene* scene)
{
    if (scene != nullptr)
    {
        scene->_onRenderTargetClear.add( new CallbackFunctorObject(&fge::ObjRenderMap::onClear, this), this );
    }
}

#ifdef FGE_DEF_SERVER
FGE_OBJ_UPDATE_BODY(ObjRenderMap){}
#else
FGE_OBJ_UPDATE_BODY(ObjRenderMap)
{
    if ( screen.getSize() != this->g_windowSize )
    {
        this->g_windowSize = screen.getSize();
        this->_renderTexture.create(this->g_windowSize.x, this->g_windowSize.y);

        this->updatePositions();
        this->updateTexCoords();

        this->g_windowView = screen.getDefaultView();
        this->g_windowView.setSize(static_cast<sf::Vector2f>(this->g_windowSize));
        this->g_windowView.setCenter(static_cast<float>(this->g_windowSize.x)/2.0f, static_cast<float>(this->g_windowSize.y)/2.0f);
    }
}
#endif

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjRenderMap)
{
    this->_renderTexture.setView(target.getView());
    this->_renderTexture.display();

    target.setView(this->g_windowView);

    states.texture = &this->_renderTexture.getTexture();
    target.draw(this->g_vertices, 4, sf::TriangleStrip, states);

    target.setView( this->_renderTexture.getView() );
}
#endif

void ObjRenderMap::removed([[maybe_unused]] fge::Scene* scene)
{
    this->detachAll();
}

void ObjRenderMap::save([[maybe_unused]] nlohmann::json& jsonObject, [[maybe_unused]] fge::Scene* scene)
{
}
void ObjRenderMap::load([[maybe_unused]] nlohmann::json& jsonObject, [[maybe_unused]] fge::Scene* scene)
{
}
void ObjRenderMap::pack([[maybe_unused]] fge::net::Packet& pck)
{
}
void ObjRenderMap::unpack([[maybe_unused]] fge::net::Packet& pck)
{
}

const char* ObjRenderMap::getClassName() const
{
    return FGE_OBJLIGHTMAP_CLASSNAME;
}
const char* ObjRenderMap::getReadableClassName() const
{
    return "render map";
}

sf::FloatRect ObjRenderMap::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect ObjRenderMap::getLocalBounds() const
{
    float width = static_cast<float>( this->g_windowSize.x );
    float height = static_cast<float>( this->g_windowSize.y );

    return {0.f, 0.f, width, height};
}

void ObjRenderMap::updatePositions()
{
    sf::FloatRect bounds = this->getLocalBounds();

    this->g_vertices[0].position = sf::Vector2f(0, 0);
    this->g_vertices[1].position = sf::Vector2f(0, bounds.height);
    this->g_vertices[2].position = sf::Vector2f(bounds.width, 0);
    this->g_vertices[3].position = sf::Vector2f(bounds.width, bounds.height);
}

void ObjRenderMap::updateTexCoords()
{
    sf::FloatRect bounds = this->getLocalBounds();

    this->g_vertices[0].texCoords = sf::Vector2f(0, 0);
    this->g_vertices[1].texCoords = sf::Vector2f(0, bounds.height);
    this->g_vertices[2].texCoords = sf::Vector2f(bounds.width, 0);
    this->g_vertices[3].texCoords = sf::Vector2f(bounds.width, bounds.height);
}

}//end fge
