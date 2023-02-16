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

#include "FastEngine/fastengine_extern.hpp"

#include "C_objText.hpp"
#include "C_object.hpp"
#include "FastEngine/C_flag.hpp"
#include "FastEngine/C_font.hpp"
#include "FastEngine/graphic/C_rectangleShape.hpp"

#define FGE_OBJSELECTBOX_CLASSNAME "FGE:OBJ:SELECTBOX"

namespace fge
{

class FGE_API ObjSelectBox : public fge::Object
{
public:
    ObjSelectBox();
    explicit ObjSelectBox(const fge::Font& font, const fge::Vector2f& pos = fge::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSelectBox)

    std::vector<tiny_utf8::string>& getTextList();
    const std::vector<tiny_utf8::string>& getTextList() const;

    void setSelectedText(tiny_utf8::string string);
    const tiny_utf8::string& getSelectedText() const;

    void setCharacterSize(fge::CharacterSize size);

    void setActiveStat(bool active);

    void setBoxSize(const fge::Vector2f& size);
    void setBoxSize(float w, float h);

    void setBoxColor(const fge::Color& color);
    void setBoxOutlineColor(const fge::Color& color);
    void setTextColor(const fge::Color& color);

    fge::CharacterSize getCharacterSize() const;

    bool getActiveStat() const;

    const fge::Vector2f& getBoxSize() const;

    const fge::Color& getBoxColor() const;
    const fge::Color& getBoxOutlineColor() const;
    const fge::Color& getTextColor() const;

    FGE_OBJ_UPDATE_DECLARE
    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet& pck) override;

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

private:
    fge::Color g_colorBox = fge::Color::White;
    fge::Color g_colorBoxOutline = fge::Color::Black;
    fge::Color g_colorText = fge::Color::Black;

    std::vector<tiny_utf8::string> g_textList;
    tiny_utf8::string g_textSelected;
    tiny_utf8::string* g_textCursor;

    mutable fge::ObjText g_text;
    mutable fge::RectangleShape g_box;

    fge::Vector2f g_boxSize = fge::Vector2f(120, 18);

    bool g_statMouseOn = false;
    bool g_statActive = false;

    fge::Flag g_flag;
};

} // namespace fge

#endif // _FGE_C_OBJSELECTBOX_HPP_INCLUDED
