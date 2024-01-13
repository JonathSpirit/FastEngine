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

#ifndef _FGE_C_OBJSWITCH_HPP_INCLUDED
#define _FGE_C_OBJSWITCH_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_objSprite.hpp"
#include "C_object.hpp"
#include "FastEngine/C_flag.hpp"

#define FGE_OBJSWITCH_CLASSNAME "FGE:OBJ:SWITCH"

namespace fge
{

class FGE_API ObjSwitch : public fge::Object
{
public:
    ObjSwitch();
    ObjSwitch(fge::Texture const& t_on, fge::Texture const& t_off, fge::Vector2f const& pos = fge::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSwitch)

    fge::Texture const& getTextureOn() const;
    fge::Texture const& getTextureOff() const;
    void setTextureOn(fge::Texture const& t_on);
    void setTextureOff(fge::Texture const& t_off);

    void setColor(fge::Color const& color);

    void setActiveStat(bool active);
    bool getActiveStat() const;

    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet const& pck) override;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

private:
    mutable fge::ObjSprite g_sprite;

    fge::Texture g_textureOn;
    fge::Texture g_textureOff;

    fge::Color g_color;

    bool g_statMouseOn = false;
    bool g_statActive = false;

    fge::Flag g_flag = false;
};

} // namespace fge

#endif // _FGE_C_OBJSWITCH_HPP_INCLUDED
