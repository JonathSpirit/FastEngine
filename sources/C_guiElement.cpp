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

#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

fge::CallbackHandler<fge::Vector2f const&> GuiElement::_onGlobalGuiScaleChange;
fge::Vector2f GuiElement::_GlobalGuiScale{1.0f, 1.0f};

//GuiElementHandler

void GuiElementHandler::setEventCallback(fge::Event& event)
{
    this->detachAll();
    event._onMouseWheel.addObjectFunctor(&fge::GuiElementHandler::onMouseWheelScrolled, this, this);
    event._onMouseButtonDown.addObjectFunctor(&fge::GuiElementHandler::onMouseButtonPressed, this, this);
    event._onMouseButtonUp.addObjectFunctor(&fge::GuiElementHandler::onMouseButtonReleased, this, this);
    event._onMouseMotion.addObjectFunctor(&fge::GuiElementHandler::onMouseMoved, this, this);

    auto const size = this->g_target->getView().getSize();
    this->_onGuiResized.call(*this, size);
    this->_lastSize = size;
}

void GuiElementHandler::onMouseWheelScrolled(fge::Event const& evt, SDL_MouseWheelEvent const& arg)
{
    fge::GuiElementContext context{};
    context._mousePosition = {arg.x, arg.y};
    context._mouseGuiPosition =
            this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
    context._handler = this;

    std::vector<fge::ObjectDataShared> keepAliveObject;
    context._keepAliveObject = &keepAliveObject;

    this->_onGuiVerify.call(evt, SDL_MOUSEWHEEL, context);

    if (context._prioritizedElement != nullptr)
    {
        context._prioritizedElement->_onGuiMouseWheelScrolled.call(evt, arg, context);

        if (context._prioritizedElement->isRecursive())
        {
            context._recursive = true;
            auto* element = context._prioritizedElement;
            context._prioritizedElement = nullptr;
            element->onGuiVerify(evt, SDL_MOUSEWHEEL, context);
            if (context._prioritizedElement != nullptr)
            {
                context._prioritizedElement->_onGuiMouseWheelScrolled.call(evt, arg, context);
            }
        }
    }
}
void GuiElementHandler::onMouseButtonPressed(fge::Event const& evt, SDL_MouseButtonEvent const& arg)
{
    fge::GuiElementContext context{};
    context._mousePosition = {arg.x, arg.y};
    context._mouseGuiPosition =
            this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
    context._handler = this;

    std::vector<fge::ObjectDataShared> keepAliveObject;
    context._keepAliveObject = &keepAliveObject;

    this->_onGuiVerify.call(evt, SDL_MOUSEBUTTONDOWN, context);

    if (context._prioritizedElement != nullptr)
    {
        context._prioritizedElement->_onGuiMouseButtonPressed.call(evt, arg, context);

        if (context._prioritizedElement->isRecursive())
        {
            context._recursive = true;
            auto* element = context._prioritizedElement;
            context._prioritizedElement = nullptr;
            element->onGuiVerify(evt, SDL_MOUSEBUTTONDOWN, context);
            if (context._prioritizedElement != nullptr)
            {
                context._prioritizedElement->_onGuiMouseButtonPressed.call(evt, arg, context);
            }
        }
    }
}
void GuiElementHandler::onMouseButtonReleased(fge::Event const& evt, SDL_MouseButtonEvent const& arg)
{
    fge::GuiElementContext context{};
    context._mousePosition = {arg.x, arg.y};
    context._mouseGuiPosition =
            this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
    context._handler = this;

    std::vector<fge::ObjectDataShared> keepAliveObject;
    context._keepAliveObject = &keepAliveObject;

    this->_onGuiVerify.call(evt, SDL_MOUSEBUTTONUP, context);

    if (context._prioritizedElement != nullptr)
    {
        context._prioritizedElement->_onGuiMouseButtonReleased.call(evt, arg, context);

        if (context._prioritizedElement->isRecursive())
        {
            context._recursive = true;
            auto* element = context._prioritizedElement;
            context._prioritizedElement = nullptr;
            element->onGuiVerify(evt, SDL_MOUSEBUTTONUP, context);
            if (context._prioritizedElement != nullptr)
            {
                context._prioritizedElement->_onGuiMouseButtonReleased.call(evt, arg, context);
            }
        }
    }
}
void GuiElementHandler::onMouseMoved(fge::Event const& evt, SDL_MouseMotionEvent const& arg)
{
    fge::GuiElementContext context{};
    context._mousePosition = {arg.x, arg.y};
    context._mouseGuiPosition =
            this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
    context._handler = this;

    std::vector<fge::ObjectDataShared> keepAliveObject;
    context._keepAliveObject = &keepAliveObject;

    this->_onGuiVerify.call(evt, SDL_MOUSEMOTION, context);

    if (context._prioritizedElement != nullptr)
    {
        context._prioritizedElement->_onGuiMouseMoved.call(evt, arg, context);

        if (context._prioritizedElement->isRecursive())
        {
            context._recursive = true;
            auto* element = context._prioritizedElement;
            context._prioritizedElement = nullptr;
            element->onGuiVerify(evt, SDL_MOUSEMOTION, context);
            if (context._prioritizedElement != nullptr)
            {
                context._prioritizedElement->_onGuiMouseMoved.call(evt, arg, context);
            }
        }
    }
}

void GuiElementHandler::checkViewSize()
{
    if (this->g_target == nullptr)
    {
        return;
    }

    if (this->g_target->getView().getSize() != this->_lastSize)
    {
        auto const size = this->g_target->getView().getSize();
        this->_onGuiResized.call(*this, size);
        this->_lastSize = size;
    }
}

} // namespace fge