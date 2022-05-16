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

class GuiElement
{
public:
    using Priority = uint8_t;

    GuiElement() = default;
    explicit GuiElement(fge::GuiElement::Priority priority) :
            _g_priority(priority)
    {}
    virtual ~GuiElement() = default;

    void setScale(const sf::Vector2f& scale)
    {
        this->_g_scale = scale;
    }
    [[nodiscard]] const sf::Vector2f& getScale() const
    {
        return this->_g_scale;
    }
    void setPriority(fge::GuiElement::Priority priority) const
    {
        this->_g_priority = priority;
    }
    [[nodiscard]] fge::GuiElement::Priority getPriority() const
    {
        return this->_g_priority;
    }
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

    virtual void onGuiVerify(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, const sf::Vector2f& mouseGuiPos, fge::GuiElement*& element, std::size_t& index) = 0;

    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&, const sf::Vector2f&, std::size_t> _onGuiMouseButtonPressed;

protected:
    mutable fge::GuiElement::Priority _g_priority{FGE_GUI_ELEMENT_PRIORITY_LAST};
    sf::Vector2f _g_scale{1.0f,1.0f};
};

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

        if (element)
        {
            element->_onGuiMouseButtonPressed.call(evt, arg, mouseGuiPos, index);
        }
    }

    fge::CallbackHandler<const fge::Event&, const sf::Event::MouseButtonEvent&, const sf::Vector2f&, fge::GuiElement*&, std::size_t&> _onGuiVerify;

private:
    fge::Event* g_event;
    const sf::RenderTarget* g_target;
};

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
