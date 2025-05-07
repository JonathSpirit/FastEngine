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

#include "FastEngine/object/C_objButton.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjButton::ObjButton() :
        g_color(fge::Color::White)
{
    this->setActiveStat(false);
}
ObjButton::ObjButton(fge::Texture textureOn, fge::Texture textureOff, fge::Vector2f const& pos) :
        g_textureOn(std::move(textureOn)),
        g_textureOff(std::move(textureOff)),
        g_color(fge::Color::White)
{
    this->setPosition(pos);
    this->setActiveStat(false);
}
ObjButton::ObjButton(fge::Texture const& texture, fge::Vector2f const& pos) :
        g_textureOn(texture),
        g_textureOff(texture),
        g_color(fge::Color::White)
{
    this->setPosition(pos);
    this->setActiveStat(false);
}

fge::Texture const& ObjButton::getTextureOn() const
{
    return this->g_textureOn;
}
fge::Texture const& ObjButton::getTextureOff() const
{
    return this->g_textureOff;
}
void ObjButton::setTextureOn(fge::Texture const& textureOn)
{
    this->g_textureOn = textureOn;
    if (this->g_statActive)
    {
        this->g_sprite.setTexture(this->g_textureOn);
    }
}
void ObjButton::setTextureOff(fge::Texture const& textureOff)
{
    this->g_textureOff = textureOff;
    if (!this->g_statActive)
    {
        this->g_sprite.setTexture(this->g_textureOff);
    }
}

void ObjButton::setColor(fge::Color const& color)
{
    this->g_color = color;
}

void ObjButton::setActiveStat(bool active)
{
    this->g_statActive = active;
    this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
}
bool ObjButton::getActiveStat() const
{
    return this->g_statActive;
}

void ObjButton::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    guiElementHandlerPtr->_onGuiVerify.addObjectFunctor(&fge::ObjButton::onGuiVerify, this, this);

    this->_onGuiMouseButtonPressed.addObjectFunctor(&fge::ObjButton::onGuiMouseButtonPressed, this, this);
    this->_onGuiMouseMoved.addObjectFunctor(&fge::ObjButton::onGuiMouseMoved, this, this);

    event._onMouseButtonUp.addObjectFunctor(&fge::ObjButton::onMouseButtonReleased, this, this);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjButton)
{
    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));
    this->g_sprite.setColor(this->g_statMouseOn ? (this->g_color - fge::Color(50, 50, 50, 0)) : this->g_color);
    this->g_sprite.draw(target, copyStates);
}
#endif

void ObjButton::save(nlohmann::json& jsonObject)
{
    fge::Object::save(jsonObject);

    jsonObject["color"] = this->g_color.toInteger();

    jsonObject["textureOn"] = this->g_textureOn;
    jsonObject["textureOff"] = this->g_textureOff;

    jsonObject["statMouseOn"] = this->g_statMouseOn;
    jsonObject["statActive"] = this->g_statActive;
}
void ObjButton::load(nlohmann::json& jsonObject, std::filesystem::path const& filePath)
{
    fge::Object::load(jsonObject, filePath);

    this->g_color = fge::Color(jsonObject.value<uint32_t>("color", 0));

    this->g_textureOn = jsonObject.value<std::string>("textureOn", std::string{FGE_TEXTURE_BAD});
    this->g_textureOff = jsonObject.value<std::string>("textureOff", std::string{FGE_TEXTURE_BAD});

    this->g_statMouseOn = jsonObject.value<bool>("statMouseOn", false);
    this->g_statActive = jsonObject.value<bool>("statActive", false);

    this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
}

void ObjButton::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_color << this->g_textureOn << this->g_textureOff << this->g_statMouseOn << this->g_statActive;
}
void ObjButton::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_color >> this->g_textureOn >> this->g_textureOff >> this->g_statMouseOn >> this->g_statActive;
}

char const* ObjButton::getClassName() const
{
    return FGE_OBJBUTTON_CLASSNAME;
}
char const* ObjButton::getReadableClassName() const
{
    return "button";
}

fge::RectFloat ObjButton::getGlobalBounds() const
{
    return this->getTransform() * this->g_sprite.getLocalBounds();
}
fge::RectFloat ObjButton::getLocalBounds() const
{
    return this->g_sprite.getLocalBounds();
}

void ObjButton::onGuiMouseButtonPressed([[maybe_unused]] fge::Event const& evt,
                                        [[maybe_unused]] SDL_MouseButtonEvent const& arg,
                                        [[maybe_unused]] fge::GuiElementContext& context)
{
    this->g_statActive = true;
    this->g_sprite.setTexture(this->g_textureOn);
    this->_onButtonPressed.call(this);
}
void ObjButton::onMouseButtonReleased([[maybe_unused]] fge::Event const& evt,
                                      [[maybe_unused]] SDL_MouseButtonEvent const& arg)
{
    this->g_statActive = false;
    this->g_sprite.setTexture(this->g_textureOff);
}
void ObjButton::onGuiMouseMoved([[maybe_unused]] fge::Event const& evt,
                                [[maybe_unused]] SDL_MouseMotionEvent const& arg,
                                [[maybe_unused]] fge::GuiElementContext& context)
{
    this->g_statMouseOn = true;
}

void ObjButton::onGuiVerify([[maybe_unused]] fge::Event const& evt,
                            [[maybe_unused]] SDL_EventType evtType,
                            fge::GuiElementContext& context)
{
    if (this->verifyPriority(context._prioritizedElement))
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto scrollRect = transform * this->getLocalBounds();

        auto customView = this->_myObjectData.lock()->getScene()->getCustomView();
        fge::Vector2f mousePosition;
        if (customView)
        {
            mousePosition = context._handler->getRenderTarget().mapFramebufferCoordsToWorldSpace(context._mousePosition,
                                                                                                 *customView);
        }
        else
        {
            mousePosition =
                    context._handler->getRenderTarget().mapFramebufferCoordsToWorldSpace(context._mousePosition);
        }

        if (scrollRect.contains(mousePosition))
        {
            context._prioritizedElement = this;
        }
        else
        {
            this->g_statMouseOn = false;
        }
    }
    else
    {
        this->g_statMouseOn = false;
    }
}

} // namespace fge
