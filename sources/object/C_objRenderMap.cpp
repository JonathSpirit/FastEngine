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

#include "FastEngine/object/C_objRenderMap.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

ObjRenderMap::ObjRenderMap() :
        g_colorClear(fge::Color::Transparent),
        g_vertexBuffer(fge::vulkan::GetActiveContext()),
        g_windowSize(0, 0)
{
    this->g_vertexBuffer.create(4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
}
ObjRenderMap::ObjRenderMap(fge::ObjRenderMap const& r) :
        fge::Object(r),
        fge::Subscriber(r),
        g_colorClear(r.g_colorClear),
        g_vertexBuffer(r.g_vertexBuffer.getContext()),
        g_windowSize(0, 0)
{
    this->g_vertexBuffer.create(4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
}
ObjRenderMap::ObjRenderMap(fge::ObjRenderMap& r) :
        fge::Object(r),
        fge::Subscriber(r),
        g_colorClear(r.g_colorClear),
        g_vertexBuffer(r.g_vertexBuffer.getContext()),
        g_windowSize(0, 0)
{
    this->g_vertexBuffer.create(4, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
}

void ObjRenderMap::onDraw([[maybe_unused]] fge::Scene const& scene, [[maybe_unused]] fge::RenderTarget& target)
{
    this->_renderTexture.setClearColor(this->g_colorClear);
    this->_renderTexture.beginRenderPass(this->_renderTexture.prepareNextFrame(nullptr, FGE_RENDER_TIMEOUT_BLOCKING));
}

void ObjRenderMap::setClearColor(fge::Color const& color)
{
    this->g_colorClear = color;
}
fge::Color const& ObjRenderMap::getClearColor() const
{
    return this->g_colorClear;
}

void ObjRenderMap::first(fge::Scene& scene)
{
    scene._onDraw.addObjectFunctor(&fge::ObjRenderMap::onDraw, this, this);
}

#ifdef FGE_DEF_SERVER
FGE_OBJ_UPDATE_BODY(ObjRenderMap) {}
#else
FGE_OBJ_UPDATE_BODY(ObjRenderMap)
{
    if (target.getSize() != this->g_windowSize)
    {
        this->g_windowSize = target.getSize();
        this->_renderTexture.resize({this->g_windowSize.x, this->g_windowSize.y});

        this->updatePositions();
        this->updateTexCoords();

        this->g_windowView = target.getDefaultView();
        this->g_windowView.setSize(static_cast<fge::Vector2f>(this->g_windowSize));
        this->g_windowView.setCenter(
                {static_cast<float>(this->g_windowSize.x) / 2.0f, static_cast<float>(this->g_windowSize.y) / 2.0f});
    }
}
#endif

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjRenderMap)
{
    this->_renderTexture.setView(target.getView());
    this->_renderTexture.endRenderPass();
    this->_renderTexture.display(FGE_RENDER_BAD_IMAGE_INDEX);

    target.setView(this->g_windowView);

    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this));
    copyStates._resTextures.set(&this->_renderTexture.getTextureImage(), 1);
    copyStates._vertexBuffer = &this->g_vertexBuffer;
    target.draw(copyStates);

    target.setView(this->_renderTexture.getView());
}
#endif

void ObjRenderMap::removed([[maybe_unused]] fge::Scene& scene)
{
    this->detachAll();
}

void ObjRenderMap::save([[maybe_unused]] nlohmann::json& jsonObject) {}
void ObjRenderMap::load([[maybe_unused]] nlohmann::json& jsonObject,
                        [[maybe_unused]] std::filesystem::path const& filePath)
{}
void ObjRenderMap::pack([[maybe_unused]] fge::net::Packet& pck) {}
void ObjRenderMap::unpack([[maybe_unused]] fge::net::Packet const& pck) {}

char const* ObjRenderMap::getClassName() const
{
    return FGE_OBJRENDERMAP_CLASSNAME;
}
char const* ObjRenderMap::getReadableClassName() const
{
    return "render map";
}

fge::RectFloat ObjRenderMap::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjRenderMap::getLocalBounds() const
{
    auto width = static_cast<float>(this->g_windowSize.x);
    auto height = static_cast<float>(this->g_windowSize.y);

    return {{0.f, 0.f}, {width, height}};
}

void ObjRenderMap::updatePositions()
{
    fge::RectFloat const bounds = this->getLocalBounds();

    this->g_vertexBuffer.getVertices()[0]._position = fge::Vector2f(0.0f, 0.0f);
    this->g_vertexBuffer.getVertices()[1]._position = fge::Vector2f(0.0f, bounds._height);
    this->g_vertexBuffer.getVertices()[2]._position = fge::Vector2f(bounds._width, 0.0f);
    this->g_vertexBuffer.getVertices()[3]._position = fge::Vector2f(bounds._width, bounds._height);
}

void ObjRenderMap::updateTexCoords()
{
    this->g_vertexBuffer.getVertices()[0]._texCoords = fge::Vector2f(0.0f, 0.0f);
    this->g_vertexBuffer.getVertices()[1]._texCoords = fge::Vector2f(0.0f, 1.0f);
    this->g_vertexBuffer.getVertices()[2]._texCoords = fge::Vector2f(1.0f, 0.0f);
    this->g_vertexBuffer.getVertices()[3]._texCoords = fge::Vector2f(1.0f, 1.0f);
}

} // namespace fge
