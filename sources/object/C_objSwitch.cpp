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

#include "FastEngine/object/C_objSwitch.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjSwitch::ObjSwitch() :
        g_color(fge::Color::White)
{}
ObjSwitch::ObjSwitch(fge::Texture const& t_on, fge::Texture const& t_off, fge::Vector2f const& pos) :
        g_textureOn(t_on),
        g_textureOff(t_off),
        g_color(fge::Color::White)
{
    this->setPosition(pos);
    this->g_sprite.setTexture(t_off);
}

fge::Texture const& ObjSwitch::getTextureOn() const
{
    return this->g_textureOn;
}
fge::Texture const& ObjSwitch::getTextureOff() const
{
    return this->g_textureOff;
}
void ObjSwitch::setTextureOn(fge::Texture const& t_on)
{
    this->g_textureOn = t_on;
}
void ObjSwitch::setTextureOff(fge::Texture const& t_off)
{
    this->g_textureOff = t_off;
}

void ObjSwitch::setColor(fge::Color const& color)
{
    this->g_color = color;
}

void ObjSwitch::setActiveStat(bool active)
{
    this->g_statActive = active;
    this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
}
bool ObjSwitch::getActiveStat() const
{
    return this->g_statActive;
}

#ifdef FGE_DEF_SERVER
FGE_OBJ_UPDATE_BODY(ObjSwitch) {}
#else
FGE_OBJ_UPDATE_BODY(ObjSwitch)
{
    this->g_statMouseOn =
            fge::IsMouseOn(target.mapFramebufferCoordsToWorldSpace(event.getMousePixelPos()), this->getGlobalBounds());

    if (this->g_flag.check(event.isMouseButtonPressed(SDL_BUTTON_LEFT)))
    {
        if (this->g_statMouseOn)
        {
            this->g_statActive = !this->g_statActive;
            this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
        }
    }
}
#endif

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSwitch)
{
    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));
    this->g_sprite.setColor(this->g_statMouseOn ? (this->g_color - fge::Color(50, 50, 50, 0)) : this->g_color);
    this->g_sprite.draw(target, copyStates);
}
#endif

void ObjSwitch::save(nlohmann::json& jsonObject)
{
    fge::Object::save(jsonObject);

    jsonObject["color"] = this->g_color.toInteger();

    jsonObject["textureOn"] = this->g_textureOn;
    jsonObject["textureOff"] = this->g_textureOff;

    jsonObject["statMouseOn"] = this->g_statMouseOn;
    jsonObject["statActive"] = this->g_statActive;
}
void ObjSwitch::load(nlohmann::json& jsonObject, std::filesystem::path const& filePath)
{
    fge::Object::load(jsonObject, filePath);

    this->g_color = fge::Color(jsonObject.value<uint32_t>("color", 0));

    this->g_textureOn = jsonObject.value<std::string>("textureOn", std::string{FGE_TEXTURE_BAD});
    this->g_textureOff = jsonObject.value<std::string>("textureOff", std::string{FGE_TEXTURE_BAD});

    this->g_statMouseOn = jsonObject.value<bool>("statMouseOn", false);
    this->g_statActive = jsonObject.value<bool>("statActive", false);

    this->g_sprite.setTexture(this->g_statActive ? this->g_textureOn : this->g_textureOff);
}

void ObjSwitch::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_color << this->g_textureOn << this->g_textureOff << this->g_statMouseOn << this->g_statActive;
}
void ObjSwitch::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_color >> this->g_textureOn >> this->g_textureOff >> this->g_statMouseOn >> this->g_statActive;
}

char const* ObjSwitch::getClassName() const
{
    return FGE_OBJSWITCH_CLASSNAME;
}
char const* ObjSwitch::getReadableClassName() const
{
    return "switch";
}

fge::RectFloat ObjSwitch::getGlobalBounds() const
{
    return this->getTransform() * this->g_sprite.getLocalBounds();
}
fge::RectFloat ObjSwitch::getLocalBounds() const
{
    return this->g_sprite.getLocalBounds();
}

} // namespace fge
