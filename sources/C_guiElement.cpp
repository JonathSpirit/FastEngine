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

#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

fge::CallbackHandler<const sf::Vector2f&> GuiElement::_onGlobalGuiScaleChange;
sf::Vector2f GuiElement::_GlobalGuiScale{1.0f, 1.0f};

//GuiElementHandler

void GuiElementHandler::setEventCallback(fge::Event& event)
{
    this->detachAll();
    event._onMouseWheelScrolled.add(new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseWheelScrolled, this),
                                    this);
    event._onMouseButtonPressed.add(new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseButtonPressed, this),
                                    this);
    event._onMouseButtonReleased.add(
            new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseButtonReleased, this), this);
    event._onMouseMoved.add(new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseMoved, this), this);
    event._onResized.add(new fge::CallbackFunctorObject(&fge::GuiElementHandler::onResized, this), this);
    this->onResized(event, {event.getWindowSize().x, event.getWindowSize().y});
}

void GuiElementHandler::onMouseWheelScrolled(const fge::Event& evt, const sf::Event::MouseWheelScrollEvent& arg)
{
    fge::GuiElementContext context{};
    context._mousePosition = {arg.x, arg.y};
    context._mouseGuiPosition =
            this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
    context._handler = this;

    std::vector<fge::ObjectDataShared> keepAliveObject;
    context._keepAliveObject = &keepAliveObject;

    this->_onGuiVerify.call(evt, sf::Event::EventType::MouseWheelScrolled, context);

    if (context._prioritizedElement != nullptr)
    {
        context._prioritizedElement->_onGuiMouseWheelScrolled.call(evt, arg, context);

        if (context._prioritizedElement->isRecursive())
        {
            context._recursive = true;
            auto* element = context._prioritizedElement;
            context._prioritizedElement = nullptr;
            element->onGuiVerify(evt, sf::Event::EventType::MouseWheelScrolled, context);
            if (context._prioritizedElement != nullptr)
            {
                context._prioritizedElement->_onGuiMouseWheelScrolled.call(evt, arg, context);
            }
        }
    }
}
void GuiElementHandler::onMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg)
{
    fge::GuiElementContext context{};
    context._mousePosition = {arg.x, arg.y};
    context._mouseGuiPosition =
            this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
    context._handler = this;

    std::vector<fge::ObjectDataShared> keepAliveObject;
    context._keepAliveObject = &keepAliveObject;

    this->_onGuiVerify.call(evt, sf::Event::EventType::MouseButtonPressed, context);

    if (context._prioritizedElement != nullptr)
    {
        context._prioritizedElement->_onGuiMouseButtonPressed.call(evt, arg, context);

        if (context._prioritizedElement->isRecursive())
        {
            context._recursive = true;
            auto* element = context._prioritizedElement;
            context._prioritizedElement = nullptr;
            element->onGuiVerify(evt, sf::Event::EventType::MouseButtonPressed, context);
            if (context._prioritizedElement != nullptr)
            {
                context._prioritizedElement->_onGuiMouseButtonPressed.call(evt, arg, context);
            }
        }
    }
}
void GuiElementHandler::onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg)
{
    fge::GuiElementContext context{};
    context._mousePosition = {arg.x, arg.y};
    context._mouseGuiPosition =
            this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
    context._handler = this;

    std::vector<fge::ObjectDataShared> keepAliveObject;
    context._keepAliveObject = &keepAliveObject;

    this->_onGuiVerify.call(evt, sf::Event::EventType::MouseButtonReleased, context);

    if (context._prioritizedElement != nullptr)
    {
        context._prioritizedElement->_onGuiMouseButtonReleased.call(evt, arg, context);

        if (context._prioritizedElement->isRecursive())
        {
            context._recursive = true;
            auto* element = context._prioritizedElement;
            context._prioritizedElement = nullptr;
            element->onGuiVerify(evt, sf::Event::EventType::MouseButtonReleased, context);
            if (context._prioritizedElement != nullptr)
            {
                context._prioritizedElement->_onGuiMouseButtonReleased.call(evt, arg, context);
            }
        }
    }
}
void GuiElementHandler::onMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg)
{
    fge::GuiElementContext context{};
    context._mousePosition = {arg.x, arg.y};
    context._mouseGuiPosition =
            this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
    context._handler = this;

    std::vector<fge::ObjectDataShared> keepAliveObject;
    context._keepAliveObject = &keepAliveObject;

    this->_onGuiVerify.call(evt, sf::Event::EventType::MouseMoved, context);

    if (context._prioritizedElement != nullptr)
    {
        context._prioritizedElement->_onGuiMouseMoved.call(evt, arg, context);

        if (context._prioritizedElement->isRecursive())
        {
            context._recursive = true;
            auto* element = context._prioritizedElement;
            context._prioritizedElement = nullptr;
            element->onGuiVerify(evt, sf::Event::EventType::MouseMoved, context);
            if (context._prioritizedElement != nullptr)
            {
                context._prioritizedElement->_onGuiMouseMoved.call(evt, arg, context);
            }
        }
    }
}

void GuiElementHandler::onResized([[maybe_unused]] const fge::Event& evt, const sf::Event::SizeEvent& arg)
{
    const sf::Vector2f size{static_cast<float>(arg.width), static_cast<float>(arg.height)};
    this->_onGuiResized.call(*this, size);
    this->_lastSize = size;
}

} // namespace fge