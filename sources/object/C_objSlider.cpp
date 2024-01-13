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

#include "FastEngine/object/C_objSlider.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

void ObjSlider::first([[maybe_unused]] fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

    this->g_scrollBaseRect.setFillColor(fge::Color{100, 100, 100, 80});
    this->g_scrollRect.setFillColor(fge::Color{60, 60, 60, 140});
    this->g_scrollRect.setOutlineColor(fge::Color{255, 255, 255, 80});
}
void ObjSlider::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    this->g_guiElementHandler = guiElementHandlerPtr;

    guiElementHandlerPtr->_onGuiVerify.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiVerify, this), this);

    guiElementHandlerPtr->_onGuiResized.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiResized, this), this);
    this->_onGuiMouseButtonPressed.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiMouseButtonPressed, this),
                                       this);

    event._onMouseMotion.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onMouseMoved, this), this);
    event._onMouseButtonUp.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onMouseButtonReleased, this), this);

    this->refreshSize(guiElementHandlerPtr->_lastSize);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSlider)
{
    auto copyStates = states.copy(this->_transform.start(*this, states._resTransform.get()));

    target.draw(this->g_scrollBaseRect, copyStates);
    target.draw(this->g_scrollRect, copyStates);
}
#endif

void ObjSlider::setSize(fge::DynamicSize const& size)
{
    this->g_size = size;
    this->refreshSize(this->g_guiElementHandler->_lastSize);
}
fge::Vector2f ObjSlider::getSize() const
{
    return this->g_size.getSize(this->getPosition(), this->g_guiElementHandler->_lastSize);
}

void ObjSlider::setCursorRatio(float ratio)
{
    ratio = std::clamp(ratio, 0.0f, 1.0f);

    this->g_scrollPositionY = ratio * (this->g_scrollBaseRect.getSize().y - this->g_scrollRect.getSize().y);
    this->refreshSize(this->g_guiElementHandler->_lastSize);
}
float ObjSlider::getCursorRatio() const
{
    return std::clamp(
            std::abs(this->g_scrollPositionY / (this->g_scrollBaseRect.getSize().y - this->g_scrollRect.getSize().y)),
            0.0f, 1.0f);
}
bool ObjSlider::isScrollPressed() const
{
    return this->g_scrollPressed;
}

void ObjSlider::refreshSize()
{
    this->refreshSize(this->g_guiElementHandler->_lastSize);
}

void ObjSlider::setScrollRectFillColor(fge::Color color)
{
    this->g_scrollRect.setFillColor(color);
}
void ObjSlider::setScrollRectOutlineColor(fge::Color color)
{
    this->g_scrollRect.setOutlineColor(color);
}
void ObjSlider::setScrollBaseRectFillColor(fge::Color color)
{
    this->g_scrollBaseRect.setFillColor(color);
}

void ObjSlider::refreshSize(fge::Vector2f const& targetSize)
{
    this->g_scrollPositionY = std::clamp(this->g_scrollPositionY, 0.0f,
                                         this->g_scrollBaseRect.getSize().y - this->g_scrollRect.getSize().y);

    auto rectSize = this->g_size.getSize(this->getPosition(), targetSize);

    this->g_scrollRect.setSize({rectSize.x, 30.0f});
    this->g_scrollBaseRect.setSize({rectSize.x, rectSize.y});
    this->g_scrollRect.setPosition({0.0f, this->g_scrollPositionY});
    this->g_scrollBaseRect.setPosition({0.0f, 0.0f});

    this->g_scrollBaseRect.setOrigin({0.0f, 0.0f});
    this->g_scrollRect.setOrigin({0.0f, 0.0f});

    this->_onSlide.call(this->getCursorRatio());
}

void ObjSlider::onGuiMouseButtonPressed([[maybe_unused]] fge::Event const& evt,
                                        [[maybe_unused]] SDL_MouseButtonEvent const& arg,
                                        fge::GuiElementContext& context)
{
    auto mousePosition = context._handler->getRenderTarget().mapPixelToCoords(
            {context._mousePosition.x, context._mousePosition.y},
            *this->_myObjectData.lock()->getLinkedScene()->getRelatedView());

    this->g_scrollPressed = true;
    this->g_scrollLastPositionY = this->g_scrollPositionY;
    this->g_lastMousePositionY = mousePosition.y;
    this->g_scrollRect.setOutlineThickness(2.0f);
}
void ObjSlider::onMouseButtonReleased([[maybe_unused]] fge::Event const& evt,
                                      [[maybe_unused]] SDL_MouseButtonEvent const& arg)
{
    if (this->g_scrollPressed)
    {
        this->g_scrollPressed = false;
        this->g_scrollRect.setOutlineThickness(0.0f);
    }
}
void ObjSlider::onMouseMoved([[maybe_unused]] fge::Event const& evt, SDL_MouseMotionEvent const& arg)
{
    if (this->g_scrollPressed)
    {
        fge::RenderTarget const& renderTarget = this->g_guiElementHandler->getRenderTarget();

        fge::Vector2f mousePos = renderTarget.mapPixelToCoords(
                {arg.x, arg.y}, *this->_myObjectData.lock()->getLinkedScene()->getRelatedView());

        auto scale = this->getParentsScale().y * this->getScale().y;
        this->g_scrollPositionY = this->g_scrollLastPositionY + (mousePos.y - this->g_lastMousePositionY) / scale;

        this->refreshSize(this->g_guiElementHandler->_lastSize);
    }
}

void ObjSlider::onGuiResized([[maybe_unused]] fge::GuiElementHandler const& handler, fge::Vector2f const& size)
{
    this->updateAnchor(size);
    this->refreshSize(size);
}

void ObjSlider::onGuiVerify([[maybe_unused]] fge::Event const& evt,
                            SDL_EventType evtType,
                            fge::GuiElementContext& context)
{
    if (evtType != SDL_MOUSEBUTTONDOWN)
    {
        return;
    }

    if (this->verifyPriority(context._prioritizedElement))
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto scrollRect = transform * this->g_scrollRect.getGlobalBounds();

        auto customView = this->_myObjectData.lock()->getLinkedScene()->getCustomView();
        fge::Vector2f mousePosition;
        if (customView)
        {
            mousePosition = context._handler->getRenderTarget().mapPixelToCoords(context._mousePosition, *customView);
        }
        else
        {
            mousePosition = context._handler->getRenderTarget().mapPixelToCoords(context._mousePosition);
        }

        if (scrollRect.contains(mousePosition))
        {
            context._prioritizedElement = this;
        }
    }
}

char const* ObjSlider::getClassName() const
{
    return FGE_OBJSLIDER_CLASSNAME;
}
char const* ObjSlider::getReadableClassName() const
{
    return "slider";
}

fge::RectFloat ObjSlider::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjSlider::getLocalBounds() const
{
    return this->g_scrollBaseRect.getLocalBounds();
}

} // namespace fge