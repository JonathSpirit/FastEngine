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

#ifndef _FGE_C_OBJSLIDER_HPP_INCLUDED
#define _FGE_C_OBJSLIDER_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/object/C_objRectangleShape.hpp"
#include "FastEngine/object/C_object.hpp"

#define FGE_OBJSLIDER_CLASSNAME "FGE:OBJ:SLIDER"

namespace fge
{

class FGE_API ObjSlider : public fge::Object, public fge::Subscriber, public fge::GuiElement
{
public:
    ObjSlider() = default;
    ~ObjSlider() override = default;

    FGE_OBJ_DEFAULT_COPYMETHOD(fge::ObjSlider)

    [[nodiscard]] fge::GuiElement* getGuiElement() override { return this; }

    void first(fge::Scene& scene) override;
    void callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr) override;
    FGE_OBJ_DRAW_DECLARE

    void setSize(fge::DynamicSize const& size);
    [[nodiscard]] fge::Vector2f getSize() const;

    void setScrollInversion(bool inverted);
    void setCursorPosition(float position);
    void setCursorRatio(float ratio);
    [[nodiscard]] float getCursorRatio() const;
    [[nodiscard]] bool isScrollPressed() const;
    [[nodiscard]] bool isScrollInverted() const;

    void refreshSize();

    void setScrollRectFillColor(fge::Color color);
    void setScrollRectOutlineColor(fge::Color color);
    void setScrollBaseRectFillColor(fge::Color color);

    [[nodiscard]] char const* getClassName() const override;
    [[nodiscard]] char const* getReadableClassName() const override;

    [[nodiscard]] fge::RectFloat getGlobalBounds() const override;
    [[nodiscard]] fge::RectFloat getLocalBounds() const override;

    fge::CallbackHandler<float> _onSlide;

private:
    void
    onGuiMouseButtonPressed(fge::Event const& evt, SDL_MouseButtonEvent const& arg, fge::GuiElementContext& context);
    void onMouseButtonReleased(fge::Event const& evt, SDL_MouseButtonEvent const& arg);
    void onMouseMoved(fge::Event const& evt, SDL_MouseMotionEvent const& arg);

    void onGuiResized(fge::GuiElementHandler const& handler, fge::Vector2f const& size);

    void onGuiVerify(fge::Event const& evt, SDL_EventType evtType, fge::GuiElementContext& context) override;

    void refreshSize(fge::Vector2f const& targetSize);

    mutable fge::ObjRectangleShape g_scrollRect;
    mutable fge::ObjRectangleShape g_scrollBaseRect;

    fge::GuiElementHandler* g_guiElementHandler{nullptr};

    fge::DynamicSize g_size;

    bool g_scrollPressed{false};
    float g_scrollPositionY{0.0f};
    float g_scrollLastPositionY{0.0f};
    float g_lastMousePositionY{0.0f};
    bool g_scrollInverted{false};
};

} // namespace fge

#endif //_FGE_C_OBJSLIDER_HPP_INCLUDED