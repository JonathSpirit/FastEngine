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

#include "FastEngine/C_objRenderMap.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

void ObjRenderMap::onClear(const fge::Scene* scene, sf::RenderTarget& target, const sf::Color& color)
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

void ObjRenderMap::first(fge::Scene* scene_ptr)
{
    if (scene_ptr)
    {
        scene_ptr->_onRenderTargetClear.add( new CallbackFunctorObject(&fge::ObjRenderMap::onClear, this), this );
    }
}
void ObjRenderMap::update(sf::RenderWindow& screen, fge::Event& event, const std::chrono::milliseconds& deltaTime, fge::Scene* scene_ptr)
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
void ObjRenderMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    this->_renderTexture.setView(target.getView());
    this->_renderTexture.display();

    target.setView(this->g_windowView);

    states.blendMode = sf::BlendMultiply;
    states.texture = &this->_renderTexture.getTexture();
    target.draw(this->g_vertices, 4, sf::TriangleStrip, states);

    target.setView( this->_renderTexture.getView() );
}
void ObjRenderMap::removed(fge::Scene* scene_ptr)
{
    this->detachAll();
}

void ObjRenderMap::save(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
}
void ObjRenderMap::load(nlohmann::json& jsonObject, fge::Scene* scene_ptr)
{
}
void ObjRenderMap::pack(fge::net::Packet& pck)
{
}
void ObjRenderMap::unpack(fge::net::Packet& pck)
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
