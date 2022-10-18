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

#ifndef _FGE_C_OBJSELECTBOX_HPP_INCLUDED
#define _FGE_C_OBJSELECTBOX_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>

#include <FastEngine/C_object.hpp>
#include <FastEngine/C_flag.hpp>
#include <FastEngine/C_font.hpp>
#include <FastEngine/C_objText.hpp>

#define FGE_OBJSELECTBOX_CLASSNAME "FGE:OBJ:SELECTBOX"

namespace fge
{

class FGE_API ObjSelectBox : public fge::Object
{
public:
    ObjSelectBox();
    explicit ObjSelectBox(const fge::Font& font, const sf::Vector2f& pos = sf::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSelectBox)

    std::vector<tiny_utf8::string>& getTextList();
    const std::vector<tiny_utf8::string>& getTextList() const;

    void setSelectedText(tiny_utf8::string string);
    const tiny_utf8::string& getSelectedText() const;

    void setCharacterSize(fge::ObjText::CharacterSize size);

    void setActiveStat(bool active);

    void setBoxSize(const sf::Vector2f& size);
    void setBoxSize(float w, float h);

    void setBoxColor(const sf::Color& color);
    void setBoxOutlineColor(const sf::Color& color);
    void setTextColor(const sf::Color& color);

    fge::ObjText::CharacterSize getCharacterSize() const;

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
    sf::Color g_colorBox = sf::Color::White;
    sf::Color g_colorBoxOutline = sf::Color::Black;
    sf::Color g_colorText = sf::Color::Black;

    std::vector<tiny_utf8::string> g_textList;
    tiny_utf8::string g_textSelected;
    tiny_utf8::string* g_textCursor;

    mutable fge::ObjText g_text;
    mutable sf::RectangleShape g_box;

    sf::Vector2f g_boxSize = sf::Vector2f(120, 18);

    bool g_statMouseOn = false;
    bool g_statActive = false;

    fge::Flag g_flag;
};

}//end fge

#endif // _FGE_C_OBJSELECTBOX_HPP_INCLUDED
