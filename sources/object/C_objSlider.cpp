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

#include "FastEngine/object/C_objSlider.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

void ObjSlider::first([[maybe_unused]] fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

    this->g_scrollBaseRect.setFillColor(sf::Color{100, 100, 100, 80});
    this->g_scrollRect.setFillColor(sf::Color{60, 60, 60, 140});
    this->g_scrollRect.setOutlineColor(sf::Color{255, 255, 255, 80});
}
void ObjSlider::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    this->g_guiElementHandler = guiElementHandlerPtr;

    guiElementHandlerPtr->_onGuiVerify.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiVerify, this), this);

    guiElementHandlerPtr->_onGuiResized.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiResized, this), this);
    this->_onGuiMouseButtonPressed.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onGuiMouseButtonPressed, this),
                                       this);

    event._onMouseMoved.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onMouseMoved, this), this);
    event._onMouseButtonReleased.add(new fge::CallbackFunctorObject(&fge::ObjSlider::onMouseButtonReleased, this),
                                     this);

    this->refreshSize(guiElementHandlerPtr->_lastSize);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSlider)
{
    states.transform *= this->getTransform();

    target.draw(this->g_scrollBaseRect, states);
    target.draw(this->g_scrollRect, states);
}
#endif

void ObjSlider::setSize(const fge::DynamicSize& size)
{
    this->g_size = size;
    this->refreshSize(this->g_guiElementHandler->_lastSize);
}
sf::Vector2f ObjSlider::getSize() const
{
    return this->g_size.getSize(this->getPosition(), this->g_guiElementHandler->_lastSize);
    ;
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

void ObjSlider::setScrollRectFillColor(sf::Color color)
{
    this->g_scrollRect.setFillColor(color);
}
void ObjSlider::setScrollRectOutlineColor(sf::Color color)
{
    this->g_scrollRect.setOutlineColor(color);
}
void ObjSlider::setScrollBaseRectFillColor(sf::Color color)
{
    this->g_scrollBaseRect.setFillColor(color);
}

void ObjSlider::refreshSize(const sf::Vector2f& targetSize)
{
    this->g_scrollPositionY = std::clamp(this->g_scrollPositionY, 0.0f,
                                         this->g_scrollBaseRect.getSize().y - this->g_scrollRect.getSize().y);

    auto rectSize = this->g_size.getSize(this->getPosition(), targetSize);

    this->g_scrollRect.setSize({rectSize.x, 30.0f});
    this->g_scrollBaseRect.setSize({rectSize.x, rectSize.y});
    this->g_scrollRect.setPosition({0.0f, this->g_scrollPositionY});
    this->g_scrollBaseRect.setPosition({0.0f, 0.0f});

    this->g_scrollBaseRect.setOrigin(0.0f, 0.0f);
    this->g_scrollRect.setOrigin(0.0f, 0.0f);

    this->_onSlide.call(this->getCursorRatio());
}

void ObjSlider::onGuiMouseButtonPressed([[maybe_unused]] const fge::Event& evt,
                                        [[maybe_unused]] const sf::Event::MouseButtonEvent& arg,
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
void ObjSlider::onMouseButtonReleased([[maybe_unused]] const fge::Event& evt,
                                      [[maybe_unused]] const sf::Event::MouseButtonEvent& arg)
{
    if (this->g_scrollPressed)
    {
        this->g_scrollPressed = false;
        this->g_scrollRect.setOutlineThickness(0.0f);
    }
}
void ObjSlider::onMouseMoved([[maybe_unused]] const fge::Event& evt, const sf::Event::MouseMoveEvent& arg)
{
    if (this->g_scrollPressed)
    {
        const sf::RenderTarget& renderTarget = this->g_guiElementHandler->getRenderTarget();

        sf::Vector2f mousePos = renderTarget.mapPixelToCoords(
                {arg.x, arg.y}, *this->_myObjectData.lock()->getLinkedScene()->getRelatedView());

        auto scale = this->getParentsScale().y * this->getScale().y;
        this->g_scrollPositionY = this->g_scrollLastPositionY + (mousePos.y - this->g_lastMousePositionY) / scale;

        this->refreshSize(this->g_guiElementHandler->_lastSize);
    }
}

void ObjSlider::onGuiResized([[maybe_unused]] const fge::GuiElementHandler& handler, const sf::Vector2f& size)
{
    this->updateAnchor(size);
    this->refreshSize(size);
}

void ObjSlider::onGuiVerify([[maybe_unused]] const fge::Event& evt,
                            sf::Event::EventType evtType,
                            fge::GuiElementContext& context)
{
    if (evtType != sf::Event::MouseButtonPressed)
    {
        return;
    }

    if (this->verifyPriority(context._prioritizedElement))
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto scrollRect = transform.transformRect(this->g_scrollRect.getGlobalBounds());

        auto customView = this->_myObjectData.lock()->getLinkedScene()->getCustomView();
        sf::Vector2f mousePosition;
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

const char* ObjSlider::getClassName() const
{
    return FGE_OBJSLIDER_CLASSNAME;
}
const char* ObjSlider::getReadableClassName() const
{
    return "slider";
}

sf::FloatRect ObjSlider::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect ObjSlider::getLocalBounds() const
{
    return this->g_scrollBaseRect.getLocalBounds();
}

} // namespace fge