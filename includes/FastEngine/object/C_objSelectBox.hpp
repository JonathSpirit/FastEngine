/*
 * Copyright 2023 Guillaume Guillet
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

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/object/C_objRectangleShape.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_object.hpp"
#include <limits>

#define FGE_OBJSELECTBOX_CLASSNAME "FGE:OBJ:SELECTBOX"

namespace fge
{

class FGE_API ObjSelectBox : public fge::Object, public fge::Subscriber, public fge::GuiElement
{
public:
    ObjSelectBox();
    explicit ObjSelectBox(fge::Font font, fge::Vector2f const& pos = fge::Vector2f());

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSelectBox)

    fge::GuiElement* getGuiElement() override { return this; }

    std::size_t getItemCount() const;
    tiny_utf8::string const* getItem(std::size_t index) const;
    bool setItem(std::size_t index, tiny_utf8::string text);
    void addItem(tiny_utf8::string text);
    void clearItems();

    void setSelectedText(tiny_utf8::string string);
    tiny_utf8::string const& getSelectedText() const;
    void clearSelectedText();

    void setCharacterSize(fge::CharacterSize size);

    void setActiveStat(bool active);
    bool getActiveStat() const;

    void setBoxSize(fge::Vector2f const& size);
    void setBoxColor(fge::Color color);
    void setBoxOutlineColor(fge::Color color);
    void setTextColor(fge::Color color);

    fge::CharacterSize getCharacterSize() const;

    fge::Vector2f const& getBoxSize() const;
    fge::Color getBoxColor() const;
    fge::Color getBoxOutlineColor() const;
    fge::Color getTextColor() const;

    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;

    FGE_OBJ_DRAW_DECLARE

    void save(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void load(nlohmann::json& jsonObject, fge::Scene* scene) override;
    void pack(fge::net::Packet& pck) override;
    void unpack(fge::net::Packet const& pck) override;

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

    fge::CallbackHandler<fge::ObjSelectBox&, std::size_t> _onSelect;

private:
    void
    onGuiMouseButtonPressed(fge::Event const& evt, SDL_MouseButtonEvent const& arg, fge::GuiElementContext& context);
    void onGuiMouseMotion(fge::Event const& evt, SDL_MouseMotionEvent const& arg, fge::GuiElementContext& context);

    void onGuiVerify(fge::Event const& evt, SDL_EventType evtType, fge::GuiElementContext& context) override;

    void updateBoxInstances();

    fge::Color g_colorBox = fge::Color::White;
    fge::Color g_colorBoxOutline = fge::Color::Black;
    fge::Color g_colorText = fge::Color::Black;

    mutable std::vector<fge::ObjText> g_textList;
    mutable fge::ObjText g_textSelected;
    mutable fge::ObjRectangleShape g_box;

    std::size_t g_cursor = std::numeric_limits<std::size_t>::max();

    bool g_statMouseOn = false;
    bool g_statActive = false;
};

} // namespace fge

#endif // _FGE_C_OBJSELECTBOX_HPP_INCLUDED
