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

#ifndef _FGE_C_OBJTEXTINPUTBOX_HPP_INCLUDED
#define _FGE_C_OBJTEXTINPUTBOX_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_object.hpp>
#include <FastEngine/C_flag.hpp>
#include <FastEngine/C_objText.hpp>

#define FGE_OBJTEXTINBOX_CLASSNAME "FGE:OBJ:TEXTINBOX"

namespace fge
{

class FGE_API ObjTextInputBox : public fge::Object
{
public:
    ObjTextInputBox();
    explicit ObjTextInputBox(const fge::Font& font, uint16_t maxLength = 10, const sf::Vector2f& pos = sf::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTextInputBox)

    void setString(const sf::String& string);
    void setCharacterSize(fge::ObjText::CharacterSize size);
    void setHideTextFlag(bool flag);
    void setMaxLength(uint16_t length);

    void setActiveStat(bool active);

    void setBoxSize(const sf::Vector2f& size);
    void setBoxSize(float w, float h);

    void setBoxColor(const sf::Color& color);
    void setBoxOutlineColor(const sf::Color& color);
    void setTextColor(const sf::Color& color);

    const sf::String& getString() const;
    fge::ObjText::CharacterSize getCharacterSize() const;
    bool isTextHide() const;
    uint16_t getMaxLength() const;

    bool getActiveStat() const;

    const sf::Vector2f& getBoxSize() const;

    const sf::Color& getBoxColor() const;
    const sf::Color& getBoxOutlineColor() const;
    const sf::Color& getTextColor() const;

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
    uint16_t g_cursor = 0;
    uint16_t g_maxLength = 10;
    bool g_hide = false;

    sf::Color g_colorBox = sf::Color::White;
    sf::Color g_colorBoxOutline = sf::Color::Black;
    sf::Color g_colorText = sf::Color::Black;

    sf::String g_string;
    mutable fge::ObjText g_text;
    mutable sf::RectangleShape g_box;

    sf::Vector2f g_boxSize = sf::Vector2f(120, 18);

    bool g_statActive = false;
    fge::Flag g_flagMouse;
};

}//end fge

#endif // _FGE_C_OBJTEXTINPUTBOX_HPP_INCLUDED
