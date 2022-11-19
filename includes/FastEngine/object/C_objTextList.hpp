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

#ifndef _FGE_C_OBJTEXTLIST_HPP_INCLUDED
#define _FGE_C_OBJTEXTLIST_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/object/C_object.hpp"
#include "FastEngine/object/C_objText.hpp"
#include "FastEngine/C_guiElement.hpp"
#include <deque>

#define FGE_OBJTEXTLIST_CLASSNAME "FGE:OBJ:TEXTLIST"

namespace fge
{

class FGE_API ObjTextList : public fge::Object, public fge::Subscriber
{
public:
    ObjTextList();
    ~ObjTextList() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjTextList)

    void first(fge::Scene* scene) override;
    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;
    FGE_OBJ_DRAW_DECLARE

    const char* getClassName() const override;
    const char* getReadableClassName() const override;

    void addString(tiny_utf8::string string);
    std::size_t getStringsSize() const;
    tiny_utf8::string& getString(std::size_t index);
    const tiny_utf8::string& getString(std::size_t index) const;
    void removeAllStrings();

    void setFont(fge::Font font);
    const fge::Font& getFont() const;

    void setBoxSize(const fge::DynamicSize& size);
    sf::Vector2f getBoxSize();

    void setTextScrollRatio(float ratio);
    float getTextScrollRatio() const;

    void setMaxStrings(std::size_t max);
    std::size_t getMaxStrings() const;

    void refreshSize();

private:
    void onGuiResized(const fge::GuiElementHandler& handler, const sf::Vector2f& size);
    void refreshSize(const sf::Vector2f& targetSize);

    mutable fge::ObjText g_text;

    fge::GuiElementHandler* g_guiElementHandler{nullptr};

    mutable sf::RectangleShape g_box{};
    float g_textScrollRatio{0.0f};
    fge::DynamicSize g_boxSize;

    std::deque<tiny_utf8::string> g_stringList;
    std::size_t g_maxStrings{100};
};

}//end fge

#endif //_FGE_C_OBJTEXTLIST_HPP_INCLUDED