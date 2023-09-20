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

#ifndef _FGE_C_GUIELEMENT_HPP_INCLUDED
#define _FGE_C_GUIELEMENT_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "C_rect.hpp"
#include "FastEngine/C_event.hpp"
#include "FastEngine/C_tunnel.hpp"
#include "FastEngine/graphic/C_renderTarget.hpp"
#include <cstdint>

#define FGE_GUI_ELEMENT_PRIORITY_LAST 0
#define FGE_GUI_ELEMENT_PRIORITY_MAX std::numeric_limits<fge::GuiElement::Priority>::max()
#define FGE_SCENE_BAD_SID std::numeric_limits<fge::ObjectSid>::max()

namespace fge
{

using ObjectSid = uint32_t;

class ObjectData;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;
using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;

class GuiElement;
class GuiElementHandler;

struct GuiElementContext
{
    fge::GuiElement* _prioritizedElement{nullptr};
    bool _recursive{false};
    std::size_t _index{0};
    fge::Vector2f _mouseGuiPosition;
    fge::Vector2i _mousePosition;
    fge::GuiElementHandler* _handler{nullptr};
    std::vector<fge::ObjectDataShared>* _keepAliveObject{nullptr};
};

struct DynamicSize
{
    enum class SizeModes
    {
        SIZE_FIXED,
        SIZE_AUTO,
        SIZE_DEFAULT = SIZE_AUTO
    };

    fge::Vector2f _fixedSize{0.0f, 0.0f};
    fge::Vector2<fge::DynamicSize::SizeModes> _sizeMode{fge::DynamicSize::SizeModes::SIZE_DEFAULT,
                                                        fge::DynamicSize::SizeModes::SIZE_DEFAULT};
    fge::Vector2f _offset{0.0f, 0.0f};

    [[nodiscard]] inline fge::Vector2f getSize(fge::Vector2f const& position, fge::Vector2f const& targetSize) const
    {
        fge::Vector2f size{};

        switch (this->_sizeMode.x)
        {
        case SizeModes::SIZE_FIXED:
            size.x = this->_fixedSize.x;
            break;
        case SizeModes::SIZE_AUTO:
            size.x = (targetSize.x - position.x) + this->_offset.x;
            break;
        default:
            break;
        }

        switch (this->_sizeMode.y)
        {
        case SizeModes::SIZE_FIXED:
            size.y = this->_fixedSize.y;
            break;
        case SizeModes::SIZE_AUTO:
            size.y = (targetSize.y - position.y) + this->_offset.y;
            break;
        default:
            break;
        }

        if (size.x < 0.0f)
        {
            size.x = 0.0f;
        }
        if (size.y < 0.0f)
        {
            size.y = 0.0f;
        }

        return size;
    }
};

/**
 * \class GuiElement
 * \ingroup objectControl
 * \brief A base class for all GUI elements
 *
 * A GUI element is a utility to handle mouse events by priority for superposed elements.
 *
 * \warning Work in progress, this class will handle drawing elements in the future.
 */
class FGE_API GuiElement
{
public:
    using Priority = uint8_t;

    GuiElement() = default;
    explicit GuiElement(fge::GuiElement::Priority priority) :
            _g_priority(priority)
    {}
    virtual ~GuiElement() = default;

    /**
     * \brief Check if this GuiElement is recursive
     *
     * A gui element is recursive if it handle others GuiElements.
     *
     * \return \b true if it is recursive, \b false otherwise
     */
    [[nodiscard]] virtual bool isRecursive() const { return false; }
    /**
     * \brief Set the scale of the element
     *
     * \param scale The scale of the element
     */
    void setGuiScale(fge::Vector2f const& scale) { this->_g_scale = scale; }
    /**
     * \brief Get the scale of the element
     *
     * \return The scale of the element
     */
    [[nodiscard]] fge::Vector2f const& getGuiScale() const { return this->_g_scale; }
    /**
     * \brief Set the priority of the element
     *
     * The priority value can be used with the scene DepthPlan.
     *
     * \param priority The priority of the element
     */
    void setPriority(fge::GuiElement::Priority priority) const { this->_g_priority = priority; }
    /**
     * \brief Get the priority of the element
     *
     * \return The priority of the element
     */
    [[nodiscard]] fge::GuiElement::Priority getPriority() const { return this->_g_priority; }
    /**
     * \brief Verify if the priority of the element is higher than the given element
     *
     * If the provided element is null, the function will assume that the element is the highest priority.
     *
     * \param element The element to compare with
     * \return \b true if the priority of the element is higher than the given element, \b false otherwise
     */
    [[nodiscard]] bool verifyPriority(fge::GuiElement* element) const
    {
        if (element == nullptr)
        {
            return true;
        }
        if (this->_g_priority > element->getPriority())
        {
            return true;
        }
        return false;
    }

    /**
     * \brief Function called to verify if the element is hovered by the mouse
     *
     * This function should call verifyPriority to verify the priority of the element.
     * If the priority is higher than the given element, the function should replace the
     * provided pointer reference \b element with the element itself.
     *
     * \param evt An fge::Event
     * \param evtType The type of the SDL event called
     * \param context The GuiElement context
     */
    virtual void onGuiVerify(fge::Event const& evt, SDL_EventType evtType, fge::GuiElementContext& context) = 0;

    fge::CallbackHandler<fge::Event const&, SDL_MouseWheelEvent const&, fge::GuiElementContext&>
            _onGuiMouseWheelScrolled; ///< Callback called when the element is verified and the mouse wheel is scrolled
    fge::CallbackHandler<fge::Event const&, SDL_MouseButtonEvent const&, fge::GuiElementContext&>
            _onGuiMouseButtonPressed; ///< Callback called when the element is verified and the mouse is pressed
    fge::CallbackHandler<fge::Event const&, SDL_MouseButtonEvent const&, fge::GuiElementContext&>
            _onGuiMouseButtonReleased; ///< Callback called when the element is verified and a mouse button is released
    fge::CallbackHandler<fge::Event const&, SDL_MouseMotionEvent const&, fge::GuiElementContext&>
            _onGuiMouseMoved; ///< Callback called when the element is verified and the mouse is moved

    static fge::CallbackHandler<fge::Vector2f const&> _onGlobalGuiScaleChange;
    inline static void setGlobalGuiScale(fge::Vector2f const& scale)
    {
        fge::GuiElement::_GlobalGuiScale = scale;
        fge::GuiElement::_onGlobalGuiScaleChange.call(scale);
    }
    inline static fge::Vector2f const& getGlobalGuiScale() { return fge::GuiElement::_GlobalGuiScale; }

protected:
    mutable fge::GuiElement::Priority _g_priority{FGE_GUI_ELEMENT_PRIORITY_LAST};
    fge::Vector2f _g_scale{1.0f, 1.0f};

private:
    static fge::Vector2f _GlobalGuiScale;
};

class GuiElementRecursive : public virtual fge::GuiElement
{
public:
    bool isRecursive() const final { return true; }
};

/**
 * \class GuiElementHandler
 * \ingroup objectControl
 * \brief A class to handle highest priority selection of GUI elements
 */
class FGE_API GuiElementHandler : public fge::Subscriber
{
public:
    GuiElementHandler() = default;
    GuiElementHandler(fge::Event& event, fge::RenderTarget const& target) :
            g_event(&event),
            g_target(&target)
    {}
    ~GuiElementHandler() override = default;

    inline void setEvent(fge::Event& event) { this->g_event = &event; }
    [[nodiscard]] inline fge::Event& getEvent() { return *this->g_event; }
    [[nodiscard]] inline fge::Event const& getEvent() const { return *this->g_event; }
    inline void setRenderTarget(fge::RenderTarget const& target) { this->g_target = &target; }
    [[nodiscard]] inline fge::RenderTarget const& getRenderTarget() const { return *this->g_target; }

    void setEventCallback(fge::Event& event);

    void onMouseWheelScrolled(fge::Event const& evt, SDL_MouseWheelEvent const& arg);
    void onMouseButtonPressed(fge::Event const& evt, SDL_MouseButtonEvent const& arg);
    void onMouseButtonReleased(fge::Event const& evt, SDL_MouseButtonEvent const& arg);
    void onMouseMoved(fge::Event const& evt, SDL_MouseMotionEvent const& arg);

    void onResized(fge::Event const& evt, SDL_WindowEvent const& arg);

    fge::CallbackHandler<fge::Event const&, SDL_EventType, fge::GuiElementContext&> _onGuiVerify;
    fge::CallbackHandler<fge::GuiElementHandler const&, fge::Vector2f const&> _onGuiResized;
    fge::Vector2f _lastSize{0.0f, 0.0f};

private:
    fge::Event* g_event{nullptr};
    fge::RenderTarget const* g_target{nullptr};
};

/**
 * \class GuiElementRectangle
 * \ingroup objectControl
 * \brief A GUI element that verify if the mouse is inside a rectangle
 */
class GuiElementRectangle : public fge::GuiElement
{
public:
    GuiElementRectangle() = default;
    GuiElementRectangle(fge::RectFloat const& rect, fge::GuiElement::Priority priority) :
            fge::GuiElement(priority),
            g_rect(rect)
    {}
    explicit GuiElementRectangle(fge::GuiElement::Priority priority) :
            fge::GuiElement(priority)
    {}
    ~GuiElementRectangle() override = default;

    void onGuiVerify([[maybe_unused]] fge::Event const& evt,
                     [[maybe_unused]] SDL_EventType evtType,
                     fge::GuiElementContext& context) override
    {
        if (this->verifyPriority(context._prioritizedElement))
        {
            fge::RectFloat rect{this->g_rect.getPosition(),
                                {this->g_rect._width * this->_g_scale.x, this->g_rect._height * this->_g_scale.y}};
            if (rect.contains(context._mouseGuiPosition))
            {
                context._prioritizedElement = this;
            }
        }
    }

    void setRectangle(fge::RectFloat const& rect) { this->g_rect = rect; }
    fge::RectFloat const& getRectangle() const { return this->g_rect; }

private:
    fge::RectFloat g_rect;
};

/**
 * \class GuiElementDefault
 * \ingroup objectControl
 * \brief A GUI element that does not any verification bounds of the mouse the mouse
 */
class GuiElementDefault : public fge::GuiElement
{
public:
    GuiElementDefault() = default;
    explicit GuiElementDefault(fge::GuiElement::Priority priority) :
            fge::GuiElement(priority)
    {}
    ~GuiElementDefault() override = default;

    void onGuiVerify([[maybe_unused]] fge::Event const& evt,
                     [[maybe_unused]] SDL_EventType evtType,
                     fge::GuiElementContext& context) override
    {
        if (this->verifyPriority(context._prioritizedElement))
        {
            context._prioritizedElement = this;
        }
    }
};

/**
 * \class GuiElementArray
 * \ingroup objectControl
 * \brief A GUI element that verify a list of GUI elements
 */
class GuiElementArray : public fge::GuiElementRecursive
{
public:
    GuiElementArray() = default;
    explicit GuiElementArray(fge::GuiElement::Priority priority) :
            fge::GuiElement(priority)
    {}
    ~GuiElementArray() override = default;

    void onGuiVerify(fge::Event const& evt, SDL_EventType evtType, fge::GuiElementContext& context) override
    {
        if (context._recursive)
        {
            this->verifyRecursively(evt, evtType, context);
        }
        else
        {
            if (this->verifyPriority(context._prioritizedElement))
            {
                context._prioritizedElement = this;
            }
        }
    }

    fge::Tunnel<fge::GuiElement> _elements;

protected:
    void verifyRecursively(fge::Event const& evt, SDL_EventType evtType, fge::GuiElementContext& context) const
    {
        fge::GuiElementContext context2{};
        context2._mouseGuiPosition = context._mouseGuiPosition;
        context2._mousePosition = context._mousePosition;
        context2._handler = context._handler;
        context2._keepAliveObject = context._keepAliveObject;

        for (std::size_t i = 0; i < this->_elements.getGatesSize(); ++i)
        {
            context2._index = i;
            this->_elements[i]->onGuiVerify(evt, evtType, context2);
        }

        if (context2._prioritizedElement != nullptr)
        {
            if (context2._prioritizedElement->isRecursive())
            {
                context2._recursive = true;
                context2._prioritizedElement->onGuiVerify(evt, evtType, context2);
            }
        }

        context._prioritizedElement = context2._prioritizedElement;
        context._index = context2._index;
    }
};

} // namespace fge

#endif // _FGE_C_GUIELEMENT_HPP_INCLUDED
