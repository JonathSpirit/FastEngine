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

#include "FastEngine/object/C_objButton.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjButton::ObjButton() :
        g_color(fge::Color::White)
{}
ObjButton::ObjButton(fge::Texture textureOn, fge::Texture textureOff, const fge::Vector2f& pos) :
        g_textureOn(std::move(textureOn)),
        g_textureOff(std::move(textureOff)),
        g_color(fge::Color::White)
{
    this->setPosition(pos);
    this->g_sprite.setTexture(this->g_textureOff);
}
ObjButton::ObjButton(const fge::Texture& texture, const fge::Vector2f& pos) :
        g_textureOn(texture),
        g_textureOff(texture),
        g_color(fge::Color::White)
{
    this->setPosition(pos);
    this->g_sprite.setTexture(this->g_textureOff);
}

const fge::Texture& ObjButton::getTextureOn() const
{
    return this->g_textureOn;
}
const fge::Texture& ObjButton::getTextureOff() const
{
    return this->g_textureOff;
}
void ObjButton::setTextureOn(const fge::Texture& textureOn)
{
    this->g_textureOn = textureOn;
}
void ObjButton::setTextureOff(const fge::Texture& textureOff)
{
    this->g_textureOff = textureOff;
}

void ObjButton::setColor(const fge::Color& color)
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

    guiElementHandlerPtr->_onGuiVerify.add(new fge::CallbackFunctorObject(&fge::ObjButton::onGuiVerify, this), this);

    this->_onGuiMouseButtonPressed.add(new fge::CallbackFunctorObject(&fge::ObjButton::onGuiMouseButtonPressed, this),
                                       this);
    this->_onGuiMouseMoved.add(new fge::CallbackFunctorObject(&fge::ObjButton::onGuiMouseMoved, this), this);

    event._onMouseButtonUp.add(new fge::CallbackFunctorObject(&fge::ObjButton::onMouseButtonReleased, this), this);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjButton)
{
    auto copyStates = states.copy(this->_transform.start(*this, states._transform));
    this->g_sprite.setColor(this->g_statMouseOn ? (this->g_color - fge::Color(50, 50, 50, 0)) : this->g_color);
    target.draw(this->g_sprite, copyStates);
}
#endif

void ObjButton::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["color"] = this->g_color.toInteger();

    jsonObject["textureOn"] = this->g_textureOn;
    jsonObject["textureOff"] = this->g_textureOff;

    jsonObject["statMouseOn"] = this->g_statMouseOn;
    jsonObject["statActive"] = this->g_statActive;
}
void ObjButton::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->g_color = fge::Color(jsonObject.value<uint32_t>("color", 0));

    this->g_textureOn = jsonObject.value<std::string>("textureOn", FGE_TEXTURE_BAD);
    this->g_textureOff = jsonObject.value<std::string>("textureOff", FGE_TEXTURE_BAD);

    this->g_statMouseOn = jsonObject.value<bool>("statMouseOn", false);
    this->g_statActive = jsonObject.value<bool>("statActive", false);

    this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
}

void ObjButton::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_color << this->g_textureOn << this->g_textureOff << this->g_statMouseOn << this->g_statActive;
}
void ObjButton::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_color >> this->g_textureOn >> this->g_textureOff >> this->g_statMouseOn >> this->g_statActive;
}

const char* ObjButton::getClassName() const
{
    return FGE_OBJBUTTON_CLASSNAME;
}
const char* ObjButton::getReadableClassName() const
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

void ObjButton::onGuiMouseButtonPressed([[maybe_unused]] const fge::Event& evt,
                                        [[maybe_unused]] const SDL_MouseButtonEvent& arg,
                                        [[maybe_unused]] fge::GuiElementContext& context)
{
    this->g_statActive = true;
    this->g_sprite.setTexture(this->g_textureOn);
    this->_onButtonPressed.call(this);
}
void ObjButton::onMouseButtonReleased([[maybe_unused]] const fge::Event& evt,
                                      [[maybe_unused]] const SDL_MouseButtonEvent& arg)
{
    this->g_statActive = false;
    this->g_sprite.setTexture(this->g_textureOff);
}
void ObjButton::onGuiMouseMoved([[maybe_unused]] const fge::Event& evt,
                                [[maybe_unused]] const SDL_MouseMotionEvent& arg,
                                [[maybe_unused]] fge::GuiElementContext& context)
{
    this->g_statMouseOn = true;
}

void ObjButton::onGuiVerify([[maybe_unused]] const fge::Event& evt,
                            [[maybe_unused]] SDL_EventType evtType,
                            fge::GuiElementContext& context)
{
    if (this->verifyPriority(context._prioritizedElement))
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto scrollRect = transform * this->getLocalBounds();

        auto customView = this->_myObjectData.lock()->getLinkedScene()->getCustomView();
        fge::Vector2f mousePosition;
        if (customView)
        {
            mousePosition = context._handler->getRenderTarget().mapPixelToCoords(context._mousePosition, *customView);
        }
        else
        {
            mousePosition = context._handler->getRenderTarget().mapPixelToCoords(context._mousePosition);
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
