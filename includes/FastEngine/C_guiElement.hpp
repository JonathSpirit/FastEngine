#ifndef _FGE_C_GUIELEMENT_HPP_INCLUDED
#define _FGE_C_GUIELEMENT_HPP_INCLUDED

#include <FastEngine/fastengine_extern.hpp>
#include <FastEngine/C_event.hpp>
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
    fge::GuiElement::Priority _g_priority{FGE_GUI_ELEMENT_PRIORITY_LAST};
};

class GuiElementHandler : public fge::Subscriber
{
public:
    GuiElementHandler(fge::Event& event, const sf::RenderTarget& target) :
            g_event(&event),
            g_target(&target)
    {}
    ~GuiElementHandler() override = default;

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

class GuiRectElement : public fge::GuiElement
{
public:
    GuiRectElement() = default;
    GuiRectElement(const sf::FloatRect& rect, fge::GuiElement::Priority priority) :
            _g_rect(rect),
            fge::GuiElement(priority)
    {}
    explicit GuiRectElement(fge::GuiElement::Priority priority) :
            fge::GuiElement(priority)
    {}
    ~GuiRectElement() override = default;

    void onGuiVerify(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, const sf::Vector2f& mouseGuiPos, fge::GuiElement*& element, std::size_t& index) override
    {
        if ( this->verifyPriority(element) )
        {
            if ( this->_g_rect.contains(mouseGuiPos) )
            {
                element = this;
                index = 0;
            }
        }
    }

protected:
    sf::FloatRect _g_rect;
};

}//end fge

#endif // _FGE_C_GUIELEMENT_HPP_INCLUDED
