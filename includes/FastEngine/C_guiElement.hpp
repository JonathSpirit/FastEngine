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
            if ( this->g_rect.contains(mouseGuiPos) )
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

class GuiElementArray : public fge::GuiElement
{
public:
    GuiElementArray() = default;
    ~GuiElementArray() override = default;

    void onGuiVerify(const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, const sf::Vector2f& mouseGuiPos, fge::GuiElement*& element, std::size_t& index) override
    {
        for (std::size_t i=0; i<this->g_elements.size(); ++i)
        {
            this->g_elements[i]->onGuiVerify(evt, arg, mouseGuiPos, element, index);
            if (element == this->g_elements[i])
            {
                index = i;
            }
        }
    }

    void addElement(fge::GuiElement* element)
    {
        this->g_elements.push_back(element);
    }
    void delElement(fge::GuiElement* element)
    {
        for (auto it = this->g_elements.cbegin(); it != this->g_elements.cend(); ++it)
        {
            this->g_elements.erase(it);
        }
    }
    void delAllElement()
    {
        this->g_elements.clear();
    }
    [[nodiscard]] fge::GuiElement* getElement(std::size_t index) const
    {
        return this->g_elements[index];
    }
    [[nodiscard]] std::size_t getElementCount() const
    {
        return this->g_elements.size();
    }

private:
    std::vector<fge::GuiElement*> g_elements;
};

}//end fge

#endif // _FGE_C_GUIELEMENT_HPP_INCLUDED
