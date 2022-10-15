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

#ifndef _FGE_C_GUIELEMENT_HPP_INCLUDED
#define _FGE_C_GUIELEMENT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_event.hpp>
#include <FastEngine/C_tunnel.hpp>
#include <cstdint>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#define FGE_GUI_ELEMENT_PRIORITY_LAST std::numeric_limits<fge::GuiElement::Priority>::max()
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
    sf::Vector2f _mouseGuiPosition;
    sf::Vector2i _mousePosition;
    fge::GuiElementHandler* _handler{nullptr};
};

enum class AnchorType
{
    ANCHOR_NONE,
    ANCHOR_UPLEFT_CORNER,
    ANCHOR_UPRIGHT_CORNER,
    ANCHOR_DOWNLEFT_CORNER,
    ANCHOR_DOWNRIGHT_CORNER
};

enum class AnchorShift
{
    SHIFT_NONE,
    SHIFT_POSITIVE_BOUNDS,
    SHIFT_NEGATIVE_BOUNDS
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
    virtual ~GuiElement();

    /**
     * \brief Check if this GuiElement is recursive
     *
     * A gui element is recursive if it handle others GuiElements.
     *
     * \return \b true if it is recursive, \b false otherwise
     */
    [[nodiscard]] virtual bool isRecursive() const
    {
        return false;
    }
    /**
     * \brief Set the scale of the element
     *
     * \param scale The scale of the element
     */
    void setGuiScale(const sf::Vector2f& scale)
    {
        this->_g_scale = scale;
    }
    /**
     * \brief Get the scale of the element
     *
     * \return The scale of the element
     */
    [[nodiscard]] const sf::Vector2f& getGuiScale() const
    {
        return this->_g_scale;
    }
    /**
     * \brief Set the priority of the element
     *
     * The priority value can be used with the scene DepthPlan.
     *
     * \param priority The priority of the element
     */
    void setPriority(fge::GuiElement::Priority priority) const
    {
        this->_g_priority = priority;
    }
    /**
     * \brief Get the priority of the element
     *
     * \return The priority of the element
     */
    [[nodiscard]] fge::GuiElement::Priority getPriority() const
    {
        return this->_g_priority;
    }
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
        if (this->_g_priority < element->getPriority())
        {
            return true;
        }
        return false;
    }

    void updateAnchor();
    inline void setObjectGuiParent(fge::ObjectDataWeak parent)
    {
        this->_g_objectParent = std::move(parent);
    }
    inline void setAnchor(fge::AnchorType type, sf::Vector2<fge::AnchorShift> shift, fge::ObjectSid target=FGE_SCENE_BAD_SID)
    {
        this->_g_anchorType = type;
        this->_g_anchorShift = shift;
        this->_g_anchorTarget = target;
        this->_g_anchorNeedUpdate = true;
    }
    inline fge::AnchorType getAnchorType() const
    {
        return this->_g_anchorType;
    }
    inline fge::ObjectSid getAnchorTarget() const
    {
        return this->_g_anchorTarget;
    }
    inline void setAnchorSuccessor(fge::ObjectDataWeak successor)
    {
        this->_g_anchorSuccessor = std::move(successor);
    }
    inline fge::ObjectDataWeak getAnchorSuccessor() const
    {
        return this->_g_anchorSuccessor;
    }
    inline void needAnchorUpdate(bool flag)
    {
        this->_g_anchorNeedUpdate = flag;
    }
    inline bool isNeedingAnchorUpdate() const
    {
        return this->_g_anchorNeedUpdate;
    }

    /**
     * \brief Function called to verify if the element is hovered by the mouse
     *
     * This function should call verifyPriority to verify the priority of the element.
     * If the priority is higher than the given element, the function should replace the
     * provided pointer reference \b element with the element itself.
     *
     * \param evt An fge::Event
     * \param evtType The type of the SFML event called
     * \param context The GuiElement context
     */
    virtual void onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context) = 0;

    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseWheelScrollEvent&, fge::GuiElementContext&> _onGuiMouseWheelScrolled; ///< Callback called when the element is verified and the mouse wheel is scrolled
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&, fge::GuiElementContext&> _onGuiMouseButtonPressed; ///< Callback called when the element is verified and the mouse is pressed
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&, fge::GuiElementContext&> _onGuiMouseButtonReleased; ///< Callback called when the element is verified and a mouse button is released
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseMoveEvent&, fge::GuiElementContext&> _onGuiMouseMoved; ///< Callback called when the element is verified and the mouse is moved

    static fge::CallbackHandler<const sf::Vector2f&> _onGlobalGuiScaleChange;
    inline static void setGlobalGuiScale(const sf::Vector2f& scale)
    {
        fge::GuiElement::_GlobalGuiScale = scale;
        fge::GuiElement::_onGlobalGuiScaleChange.call(scale);
    }
    inline static const sf::Vector2f& getGlobalGuiScale()
    {
        return fge::GuiElement::_GlobalGuiScale;
    }

protected:
    mutable fge::GuiElement::Priority _g_priority{FGE_GUI_ELEMENT_PRIORITY_LAST};
    sf::Vector2f _g_scale{1.0f,1.0f};
    fge::AnchorType _g_anchorType{fge::AnchorType::ANCHOR_NONE};
    sf::Vector2<fge::AnchorShift> _g_anchorShift{fge::AnchorShift::SHIFT_NONE,fge::AnchorShift::SHIFT_NONE};
    fge::ObjectSid _g_anchorTarget{FGE_SCENE_BAD_SID};
    bool _g_anchorNeedUpdate{true};
    fge::ObjectDataWeak _g_anchorSuccessor{};
    fge::ObjectDataWeak _g_objectParent{};

private:
    static sf::Vector2f _GlobalGuiScale;
};

/**
 * \class GuiElementHandler
 * \ingroup objectControl
 * \brief A class to handle highest priority selection of GUI elements
 */
class GuiElementHandler : public fge::Subscriber
{
public:
    GuiElementHandler(fge::Event& event, const sf::RenderTarget& target) :
            g_event(&event),
            g_target(&target)
    {}
    ~GuiElementHandler() override = default;

    fge::Event& getEvent()
    {
        return *this->g_event;
    }
    const fge::Event& getEvent() const
    {
        return *this->g_event;
    }
    const sf::RenderTarget& getRenderTarget() const
    {
        return *this->g_target;
    }

    void setEventCallback(fge::Event& event)
    {
        this->detachAll();
        event._onMouseWheelScrolled.add( new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseWheelScrolled, this), this );
        event._onMouseButtonPressed.add( new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseButtonPressed, this), this );
        event._onMouseButtonReleased.add( new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseButtonReleased, this), this );
        event._onMouseMoved.add( new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseMoved, this), this );
    }

    void onMouseWheelScrolled(const fge::Event& evt, const sf::Event::MouseWheelScrollEvent& arg)
    {
        fge::GuiElementContext context{};
        context._mousePosition = {arg.x, arg.y};
        context._mouseGuiPosition = this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
        context._handler = this;

        this->_onGuiVerify.call(evt, sf::Event::EventType::MouseWheelScrolled, context);

        if (context._prioritizedElement != nullptr)
        {
            if (context._prioritizedElement->isRecursive())
            {
                context._prioritizedElement->_onGuiMouseWheelScrolled.call(evt, arg, context);
                context._recursive = true;
                auto* element = context._prioritizedElement;
                context._prioritizedElement = nullptr;
                element->onGuiVerify(evt, sf::Event::EventType::MouseWheelScrolled, context);
                if (context._prioritizedElement != nullptr)
                {
                    context._prioritizedElement->_onGuiMouseWheelScrolled.call(evt, arg, context);
                }
            }
            else
            {
                context._prioritizedElement->_onGuiMouseWheelScrolled.call(evt, arg, context);
            }
        }
    }
    void onMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg)
    {
        fge::GuiElementContext context{};
        context._mousePosition = {arg.x, arg.y};
        context._mouseGuiPosition = this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
        context._handler = this;

        this->_onGuiVerify.call(evt, sf::Event::EventType::MouseButtonPressed, context);

        if (context._prioritizedElement != nullptr)
        {
            if (context._prioritizedElement->isRecursive())
            {
                context._prioritizedElement->_onGuiMouseButtonPressed.call(evt, arg, context);
                context._recursive = true;
                auto* element = context._prioritizedElement;
                context._prioritizedElement = nullptr;
                element->onGuiVerify(evt, sf::Event::EventType::MouseButtonPressed, context);
                if (context._prioritizedElement != nullptr)
                {
                    context._prioritizedElement->_onGuiMouseButtonPressed.call(evt, arg, context);
                }
            }
            else
            {
                context._prioritizedElement->_onGuiMouseButtonPressed.call(evt, arg, context);
            }
        }
    }
    void onMouseButtonReleased(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg)
    {
        fge::GuiElementContext context{};
        context._mousePosition = {arg.x, arg.y};
        context._mouseGuiPosition = this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
        context._handler = this;

        this->_onGuiVerify.call(evt, sf::Event::EventType::MouseButtonReleased, context);

        if (context._prioritizedElement != nullptr)
        {
            if (context._prioritizedElement->isRecursive())
            {
                context._prioritizedElement->_onGuiMouseButtonReleased.call(evt, arg, context);
                context._recursive = true;
                auto* element = context._prioritizedElement;
                context._prioritizedElement = nullptr;
                element->onGuiVerify(evt, sf::Event::EventType::MouseButtonReleased, context);
                if (context._prioritizedElement != nullptr)
                {
                    context._prioritizedElement->_onGuiMouseButtonReleased.call(evt, arg, context);
                }
            }
            else
            {
                context._prioritizedElement->_onGuiMouseButtonReleased.call(evt, arg, context);
            }
        }
    }
    void onMouseMoved(const fge::Event& evt, const sf::Event::MouseMoveEvent& arg)
    {
        fge::GuiElementContext context{};
        context._mousePosition = {arg.x, arg.y};
        context._mouseGuiPosition = this->g_target->mapPixelToCoords(context._mousePosition, this->g_target->getDefaultView());
        context._handler = this;

        this->_onGuiVerify.call(evt, sf::Event::EventType::MouseMoved, context);

        if (context._prioritizedElement != nullptr)
        {
            if (context._prioritizedElement->isRecursive())
            {
                context._prioritizedElement->_onGuiMouseMoved.call(evt, arg, context);
                context._recursive = true;
                auto* element = context._prioritizedElement;
                context._prioritizedElement = nullptr;
                element->onGuiVerify(evt, sf::Event::EventType::MouseMoved, context);
                if (context._prioritizedElement != nullptr)
                {
                    context._prioritizedElement->_onGuiMouseMoved.call(evt, arg, context);
                }
            }
            else
            {
                context._prioritizedElement->_onGuiMouseMoved.call(evt, arg, context);
            }
        }
    }

    fge::CallbackHandler<const fge::Event&, sf::Event::EventType, fge::GuiElementContext&> _onGuiVerify;

private:
    fge::Event* g_event;
    const sf::RenderTarget* g_target;
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
    GuiElementRectangle(const sf::FloatRect& rect, fge::GuiElement::Priority priority) :
            fge::GuiElement(priority),
            g_rect(rect)
    {}
    explicit GuiElementRectangle(fge::GuiElement::Priority priority) :
            fge::GuiElement(priority)
    {}
    ~GuiElementRectangle() override = default;

    void onGuiVerify([[maybe_unused]] const fge::Event& evt, [[maybe_unused]] sf::Event::EventType evtType, fge::GuiElementContext& context) override
    {
        if ( this->verifyPriority(context._prioritizedElement) )
        {
            sf::FloatRect rect{this->g_rect.getPosition(), {this->g_rect.width*this->_g_scale.x, this->g_rect.height*this->_g_scale.y}};
            if ( rect.contains(context._mouseGuiPosition) )
            {
                context._prioritizedElement = this;
            }
        }
    }

    void setRectangle(const sf::FloatRect& rect)
    {
        this->g_rect = rect;
    }
    const sf::FloatRect& getRectangle() const
    {
        return this->g_rect;
    }

private:
    sf::FloatRect g_rect;
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

    void onGuiVerify([[maybe_unused]] const fge::Event& evt, [[maybe_unused]] sf::Event::EventType evtType, fge::GuiElementContext& context) override
    {
        if ( this->verifyPriority(context._prioritizedElement) )
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
class GuiElementArray : public fge::GuiElement
{
public:
    GuiElementArray() = default;
    explicit GuiElementArray(fge::GuiElement::Priority priority) :
            fge::GuiElement(priority)
    {}
    ~GuiElementArray() override = default;

    [[nodiscard]] bool isRecursive() const override
    {
        return true;
    }

    void onGuiVerify(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context) override
    {
        if (context._recursive)
        {
            this->verifyRecursively(evt, evtType, context);
        }
        else
        {
            if ( this->verifyPriority(context._prioritizedElement) )
            {
                context._prioritizedElement = this;
            }
        }
    }

    fge::Tunnel<fge::GuiElement> _elements;

protected:
    void verifyRecursively(const fge::Event& evt, sf::Event::EventType evtType, fge::GuiElementContext& context) const
    {
        fge::GuiElementContext context2{};
        context2._mouseGuiPosition = context._mouseGuiPosition;
        context2._mousePosition = context._mousePosition;
        context2._handler = context._handler;

        for (std::size_t i=0; i<this->_elements.getGatesSize(); ++i)
        {
            context2._index = i;
            this->_elements[i]->onGuiVerify(evt, evtType, context2);
        }

        if (context2._prioritizedElement != nullptr)
        {
            if (context2._prioritizedElement->isRecursive())
            {
                context2._recursive = true;
                context2._prioritizedElement = nullptr;
                context2._prioritizedElement->onGuiVerify(evt, evtType, context2);
            }
        }

        context._prioritizedElement = context2._prioritizedElement;
        context._index = context2._index;
    }
};

}//end fge

#endif // _FGE_C_GUIELEMENT_HPP_INCLUDED
