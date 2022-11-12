#include "FastEngine/object/C_objTextList.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "FastEngine/object/C_objWindow.hpp"

namespace fge
{

void ObjTextList::first(fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;
    this->g_parentPtr = fge::ObjWindow::getWindowFromScene(scene);

    this->g_text.setCharacterSize(14);
    this->g_text.setFillColor(sf::Color::White);
    this->g_text.setOutlineColor(sf::Color::Black);
    this->g_text.setOutlineThickness(1.0f);

    this->g_scrollBaseRect.setFillColor(sf::Color{100,100,100,80});
    this->g_scrollRect.setFillColor(sf::Color{160,160,160,80});
    this->g_scrollRect.setOutlineColor(sf::Color{255,255,255,80});

    //this->g_defaultElementGate.setData(&this->g_defaultElement);
    ///this->g_parentPtr->_elements.addGate(this->g_defaultElementGate);
    //this->g_defaultElement._onGuiMouseButtonPressed.add( new fge::CallbackFunctorObject(&fge::ObjTextList::onGuiMouseButtonPressed, this), this );
}
void ObjTextList::callbackRegister(fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->g_guiElementHandler = guiElementHandlerPtr;
    event._onMouseMoved.add(new fge::CallbackFunctorObject(&fge::ObjTextList::onMouseMoved, this), this);
    event._onMouseButtonReleased.add(new fge::CallbackFunctorObject(&fge::ObjTextList::onMouseButtonReleased, this), this);
    guiElementHandlerPtr->_onGuiResized.add( new fge::CallbackFunctorObject(&fge::ObjTextList::onGuiResized, this), this );

    this->refreshPosition(guiElementHandlerPtr->_lastSize);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTextList)
{
    states.transform *= this->getTransform();

    float textOffset = -static_cast<float>(this->g_text.getCharacterSize());

    this->g_text.setPosition(4.0f, textOffset);
    for (std::size_t i=static_cast<std::size_t>(static_cast<float>(this->g_maxStrings-1)*this->getCursorRatio()); i<this->g_stringList.size()-1; ++i)
    {
        this->g_text.setString(this->g_stringList[i]);
        target.draw(this->g_text, states);

        this->g_text.move(0, textOffset);
    }

    target.draw(this->g_scrollBaseRect, states);
    target.draw(this->g_scrollRect, states);
}
#endif

const char* ObjTextList::getClassName() const
{
    return FGE_OBJTEXTLIST_CLASSNAME;
}
const char* ObjTextList::getReadableClassName() const
{
    return "text list";
}

void ObjTextList::onGuiResized([[maybe_unused]] const fge::GuiElementHandler& handler, const sf::Vector2f& size)
{
    this->refreshPosition(size);
}

void ObjTextList::addString(std::string string)
{
    this->g_stringList.insert(this->g_stringList.begin(), std::move(string));
    if (this->g_stringList.size() > this->g_maxStrings)
    {
        this->g_stringList.erase(this->g_stringList.end()-1);
    }
}
std::size_t ObjTextList::getStringsSize() const
{
    return this->g_stringList.size();
}
std::string& ObjTextList::getString(std::size_t index)
{
    return this->g_stringList[index];
}
const std::string& ObjTextList::getString(std::size_t index) const
{
    return this->g_stringList[index];
}
void ObjTextList::removeAllStrings()
{
    this->g_stringList.clear();
}

void ObjTextList::setFont(const fge::Font& font)
{
    this->g_text.setFont(font);
}
const fge::Font& ObjTextList::getFont() const
{
    return this->g_text.getFont();
}

void ObjTextList::setBottomOffset(float offset)
{
    this->g_bottomOffset = offset;
}
float ObjTextList::getBottomOffset() const
{
    return this->g_bottomOffset;
}

void ObjTextList::setCursorRatio(float ratio)
{
    this->g_scrollPositionY = -ratio * (this->g_scrollBaseRect.getSize().y-this->g_scrollRect.getSize().y);
    this->refreshPosition(this->g_guiElementHandler->_lastSize);
}
float ObjTextList::getCursorRatio() const
{
    return std::abs( this->g_scrollPositionY / (this->g_scrollBaseRect.getSize().y-this->g_scrollRect.getSize().y) );
}
bool ObjTextList::isScrollPressed() const
{
    return this->g_scrollPressed;
}

void ObjTextList::setMaxStrings(std::size_t max)
{
    this->g_maxStrings = max;
}
std::size_t ObjTextList::getMaxStrings() const
{
    return this->g_maxStrings;
}

void ObjTextList::refreshPosition(const sf::Vector2f& size)
{
    this->g_scrollPositionY = std::clamp(this->g_scrollPositionY, -(this->g_scrollBaseRect.getSize().y - this->g_scrollRect.getSize().y), 0.0f);

    auto windowSize = size;

    this->setPosition( sf::Vector2f{0.0f, windowSize.y - this->g_bottomOffset} );

    this->g_scrollRect.setSize({10.0f, 30.0f});
    this->g_scrollBaseRect.setSize({10.0f, this->getPosition().y});
    this->g_scrollRect.setPosition({windowSize.x - 14.0f, this->g_scrollPositionY});
    this->g_scrollBaseRect.setPosition({windowSize.x - 14.0f, 0.0f});

    this->g_scrollBaseRect.setOrigin(0.0f, this->g_scrollBaseRect.getSize().y);
    this->g_scrollRect.setOrigin(0.0f, this->g_scrollRect.getSize().y);
}

void ObjTextList::onGuiMouseButtonPressed([[maybe_unused]] const fge::Event& evt, const sf::Event::MouseButtonEvent& arg, [[maybe_unused]] fge::GuiElementContext& context)
{
    auto scrollRect = this->getTransform().transformRect(this->g_scrollRect.getGlobalBounds());

    auto mousePosition = this->_myObjectData.lock()->getLinkedScene()->getLinkedRenderTarget()->mapPixelToCoords({arg.x, arg.y}, this->g_parentPtr->_windowView);
    if ( scrollRect.contains(mousePosition) )
    {
        this->g_scrollPressed = true;
        this->g_scrollRelativePosY = (mousePosition.y - this->getPosition().y) - this->g_scrollPositionY;
        this->g_scrollRect.setOutlineThickness(1.0f);
    }
}
void ObjTextList::onMouseButtonReleased([[maybe_unused]] const fge::Event& evt, [[maybe_unused]] const sf::Event::MouseButtonEvent& arg)
{
    if ( this->g_scrollPressed )
    {
        this->g_scrollPressed = false;
        this->g_scrollRect.setOutlineThickness(0.0f);
    }
}
void ObjTextList::onMouseMoved([[maybe_unused]] const fge::Event& evt, const sf::Event::MouseMoveEvent& arg)
{
    if ( this->g_scrollPressed )
    {
        sf::RenderTarget* renderTarget = this->_myObjectData.lock()->getLinkedScene()->getLinkedRenderTarget();

        sf::Vector2f mousePos = renderTarget->mapPixelToCoords({arg.x, arg.y}, this->g_parentPtr->_windowView);

        this->g_scrollPositionY = (mousePos.y-this->g_scrollRelativePosY) - this->getPosition().y;

        this->refreshPosition(this->g_guiElementHandler->_lastSize);
    }
}

}//end fge