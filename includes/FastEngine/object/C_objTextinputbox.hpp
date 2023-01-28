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

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/C_flag.hpp"
#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/graphic/C_rectangleShape.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_object.hpp"

#define FGE_OBJTEXTINBOX_CLASSNAME "FGE:OBJ:TEXTINBOX"

namespace fge
{

class FGE_API ObjTextInputBox : public fge::Object, public fge::Subscriber, public fge::GuiElement
{
public:
    ObjTextInputBox();
    explicit ObjTextInputBox(const fge::Font& font,
                             uint16_t maxLength = 10,
                             const fge::Vector2f& pos = fge::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTextInputBox)

    fge::GuiElement* getGuiElement() override { return this; }

    void setString(tiny_utf8::string string);
    void setCharacterSize(fge::ObjText::CharacterSize size);
    void setHideTextFlag(bool flag);
    void setMaxLength(uint16_t length);

    void setActiveStat(bool active);

    void setBoxSize(const fge::Vector2f& size);
    void setBoxSize(float w, float h);

    void setBoxColor(const fge::Color& color);
    void setBoxOutlineColor(const fge::Color& color);
    void setTextColor(const fge::Color& color);

    const tiny_utf8::string& getString() const;
    fge::ObjText::CharacterSize getCharacterSize() const;
    bool isTextHide() const;
    uint16_t getMaxLength() const;

    bool getActiveStat() const;

    const fge::Vector2f& getBoxSize() const;

    const fge::Color& getBoxColor() const;
    const fge::Color& getBoxOutlineColor() const;
    const fge::Color& getTextColor() const;

    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;

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
    void
    onGuiMouseButtonPressed(const fge::Event& evt, const SDL_MouseButtonEvent& arg, fge::GuiElementContext& context);

    void onGuiVerify(const fge::Event& evt, SDL_EventType evtType, fge::GuiElementContext& context) override;

    uint16_t g_cursor = 0;
    uint16_t g_maxLength = 10;
    bool g_hide = false;

    fge::Color g_colorBox = fge::Color::White;
    fge::Color g_colorBoxOutline = fge::Color::Black;
    fge::Color g_colorText = fge::Color::Black;

    tiny_utf8::string g_string;
    mutable fge::ObjText g_text;
    mutable fge::RectangleShape g_box;

    fge::Vector2f g_boxSize = fge::Vector2f(120, 18);

    bool g_statActive = false;
};

} // namespace fge

#endif // _FGE_C_OBJTEXTINPUTBOX_HPP_INCLUDED
