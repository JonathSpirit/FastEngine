#ifndef _FGE_C_GUIELEMENT_HPP_INCLUDED
#define _FGE_C_GUIELEMENT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_event.hpp>
#include <FastEngine/C_tunnel.hpp>
#include <cstdint>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#define FGE_GUI_ELEMENT_PRIORITY_LAST std::numeric_limits<fge::GuiElement::Priority>::max()

namespace fge
{

/**
 * \class GuiElement
 * \ingroup objectControl
 * \brief A base class for all GUI elements
 *
 * A GUI element is a utility to handle mouse events by priority for superposed elements.
 *
 * \warning Work in progress, this class will handle drawing elements in the future.
 */
class GuiElement
{
public:
    using Priority = uint8_t;

    GuiElement() = default;
    explicit GuiElement(fge::GuiElement::Priority priority) :
            _g_priority(priority)
    {}
    virtual ~GuiElement() = default;

    /**
     * \brief Set the scale of the element
     *
     * \param scale The scale of the element
     */
    void setScale(const sf::Vector2f& scale)
    {
        this->_g_scale = scale;
    }
    /**
     * \brief Get the scale of the element
     *
     * \return The scale of the element
     */
    [[nodiscard]] const sf::Vector2f& getScale() const
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

    /**
     * \brief Function called to verify if the element is hovered/clicked by the mouse
     *
     * This function should call verifyPriority to verify the priority of the element.
     * If the priority is higher than the given element, the function should replace the
     * provided pointer reference \b element with the element itself.
     *
     * \param evt An fge::Event
     * \param arg The argument of the mouse button event
     * \param mouseGuiPos The position of the mouse already converted to the GUI coordinate system
     * \param element An element to compare with
     * \param index The index of the element if there are multiple elements
     */
    virtual void onGuiVerify(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, const sf::Vector2f& mouseGuiPos, fge::GuiElement*& element, std::size_t& index) = 0;

    /**
     * \brief Callback called when the element is verified and the mouse is pressed
     */
    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&, const sf::Vector2f&, std::size_t> _onGuiMouseButtonPressed;

protected:
    mutable fge::GuiElement::Priority _g_priority{FGE_GUI_ELEMENT_PRIORITY_LAST};
    sf::Vector2f _g_scale{1.0f,1.0f};
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
        event._onMouseButtonPressed.add( new fge::CallbackFunctorObject(&fge::GuiElementHandler::onMouseButtonPressed, this), this );
    }

    void onMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg)
    {
        fge::GuiElement* element = nullptr;
        std::size_t index = 0;
        sf::Vector2f mouseGuiPos = this->g_target->mapPixelToCoords({arg.x, arg.y}, this->g_target->getDefaultView());

        this->_onGuiVerify.call(evt, arg, mouseGuiPos, element, index);

        if (element != nullptr)
        {
            element->_onGuiMouseButtonPressed.call(evt, arg, mouseGuiPos, index);
        }
    }

    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&, const sf::Vector2f&, fge::GuiElement*&, std::size_t&> _onGuiVerify;

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

    void onGuiVerify(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, const sf::Vector2f& mouseGuiPos, fge::GuiElement*& element, std::size_t& index) override
    {
        if ( this->verifyPriority(element) )
        {
            sf::FloatRect rect{this->g_rect.getPosition(), {this->g_rect.width*this->_g_scale.x, this->g_rect.height*this->_g_scale.y}};
            if ( rect.contains(mouseGuiPos) )
            {
                element = this;
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

    void onGuiVerify(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, const sf::Vector2f& mouseGuiPos, fge::GuiElement*& element, std::size_t& index) override
    {
        if ( this->verifyPriority(element) )
        {
            element = this;
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
    GuiElementArray()
    {
        this->_onGuiMouseButtonPressed.add(new fge::CallbackFunctorObject(&fge::GuiElementArray::onGuiMouseButtonPressed, this), &this->g_subscriber);
    }
    explicit GuiElementArray(fge::GuiElement::Priority priority) :
            fge::GuiElement(priority)
    {
        this->_onGuiMouseButtonPressed.add(new fge::CallbackFunctorObject(&fge::GuiElementArray::onGuiMouseButtonPressed, this), &this->g_subscriber);
    }
    ~GuiElementArray() override = default;

    void onGuiVerify(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, const sf::Vector2f& mouseGuiPos, fge::GuiElement*& element, std::size_t& index) override
    {
        if ( this->verifyPriority(element) )
        {
            element = this;
        }
    }

    void refreshInternalCallback()
    {
        this->_onGuiMouseButtonPressed.del(&this->g_subscriber);
        this->_onGuiMouseButtonPressed.add(new fge::CallbackFunctorObject(&fge::GuiElementArray::onGuiMouseButtonPressed, this), &this->g_subscriber);
    }

    fge::Tunnel<fge::GuiElement> _elements;

private:
    void onGuiMouseButtonPressed(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, const sf::Vector2f& mouseGuiPos, std::size_t index)
    {
        fge::GuiElement* element = nullptr;
        std::size_t index2 = 0;

        for (std::size_t i=0; i<this->_elements.getGatesSize(); ++i)
        {
            this->_elements[i]->onGuiVerify(evt, arg, mouseGuiPos, element, index2);
            if (element == this->_elements[i])
            {
                index2 = i;
            }
        }

        if (element)
        {
            element->_onGuiMouseButtonPressed.call(evt, arg, mouseGuiPos, index2);
        }
    }

    fge::Subscriber g_subscriber;
};

}//end fge

#endif // _FGE_C_GUIELEMENT_HPP_INCLUDED
