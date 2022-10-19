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

#ifndef _FGE_C_OBJSWITCH_HPP_INCLUDED
#define _FGE_C_OBJSWITCH_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "C_object.hpp"
#include "FastEngine/C_flag.hpp"
#include "C_objSprite.hpp"

#define FGE_OBJSWITCH_CLASSNAME "FGE:OBJ:SWITCH"

namespace fge
{

class FGE_API ObjSwitch : public fge::Object
{
public:
    ObjSwitch();
    ObjSwitch(const fge::Texture& t_on, const fge::Texture& t_off, const sf::Vector2f& pos=sf::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSwitch)

    const fge::Texture& getTextureOn() const;
    const fge::Texture& getTextureOff() const;
    void setTextureOn(const fge::Texture& t_on);
    void setTextureOff(const fge::Texture& t_off);

    void setColor(const sf::Color& color);

    void setActiveStat(bool active);
    bool getActiveStat() const;

    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    sf::FloatRect getGlobalBounds() const override;
    sf::FloatRect getLocalBounds() const override;

private:
    mutable fge::ObjSprite g_sprite;

    fge::Texture g_textureOn;
    fge::Texture g_textureOff;

    sf::Color g_color;

    bool g_statMouseOn = false;
    bool g_statActive = false;

    fge::Flag g_flag = false;
};

}//end fge

#endif // _FGE_C_OBJSWITCH_HPP_INCLUDED
