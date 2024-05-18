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

#ifndef _FGE_C_OBJTEXTLIST_HPP_INCLUDED
#define _FGE_C_OBJTEXTLIST_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/object/C_objRectangleShape.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/object/C_object.hpp"
#include <list>

#define FGE_OBJTEXTLIST_CLASSNAME "FGE:OBJ:TEXTLIST"

namespace fge
{

class FGE_API ObjTextList : public fge::Object, public fge::Subscriber
{
public:
    ObjTextList();
    ObjTextList(ObjTextList const& r);
    ~ObjTextList() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTextList)

    void first(fge::Scene& scene) override;
    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;
    FGE_OBJ_DRAW_DECLARE

    char const* getClassName() const override;
    char const* getReadableClassName() const override;

    fge::RectFloat getGlobalBounds() const override;
    fge::RectFloat getLocalBounds() const override;

    void addText(tiny_utf8::string string);
    std::size_t getTextCount() const;
    fge::ObjText* getText(std::size_t index);
    fge::ObjText const* getText(std::size_t index) const;
    void removeAllTexts();

    void setFont(fge::Font font);
    fge::Font const& getFont() const;

    void setBoxSize(fge::DynamicSize const& size);
    fge::Vector2f getBoxSize() const;

    void setTextScrollRatio(float ratio);
    float getTextScrollRatio() const;

    void setMaxTextCount(std::size_t max);
    std::size_t getMaxTextCount() const;

    void refreshSize();

private:
    void onGuiResized(fge::GuiElementHandler const& handler, fge::Vector2f const& size);
    void refreshSize(fge::Vector2f const& targetSize);

    fge::GuiElementHandler* g_guiElementHandler{nullptr};

    mutable fge::ObjRectangleShape g_box{};
    float g_textScrollRatio{0.0f};
    fge::DynamicSize g_boxSize;

    fge::Font g_font;

    mutable std::list<fge::ObjText> g_textList;
    std::size_t g_maxStrings{100};
};

} // namespace fge

#endif //_FGE_C_OBJTEXTLIST_HPP_INCLUDED