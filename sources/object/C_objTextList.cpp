#include "FastEngine/object/C_objTextList.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjTextList::ObjTextList()
{
    this->g_box.setFillColor(sf::Color::Transparent);
    this->g_box.setOutlineColor(sf::Color{100,100,100,255});
    this->g_box.setOutlineThickness(2.0f);
}

void ObjTextList::first([[maybe_unused]] fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;

    this->g_text.setCharacterSize(14);
    this->g_text.setFillColor(sf::Color::White);
    this->g_text.setOutlineColor(sf::Color::Black);
    this->g_text.setOutlineThickness(1.0f);
}
void ObjTextList::callbackRegister([[maybe_unused]] fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->g_guiElementHandler = guiElementHandlerPtr;
    guiElementHandlerPtr->_onGuiResized.add( new fge::CallbackFunctorObject(&fge::ObjTextList::onGuiResized, this), this );

    this->refreshPosition(guiElementHandlerPtr->_lastSize);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTextList)
{
    states.transform *= this->getTransform();

    target.draw(this->g_box, states);

    sf::View backupView = target.getView();
    sf::View clipView = fge::ClipView(backupView, target, states.transform.transformRect({{0.0f,0.0f}, this->g_box.getSize()}),
                                      fge::ClipClampModes::CLIP_CLAMP_HIDE);

    target.setView(clipView);

    if (this->g_stringList.empty())
    {
        return;
    }

    float characterHeightOffset = static_cast<float>(this->g_text.getLineSpacing());

    this->g_text.setPosition(4.0f, this->g_box.getSize().y-characterHeightOffset);
    for (std::size_t i=static_cast<std::size_t>(static_cast<float>(this->g_stringList.size()-1)*this->getCursorRatio()); i<this->g_stringList.size(); ++i)
    {
        this->g_text.setString(this->g_stringList[i]);
        target.draw(this->g_text, states);

        this->g_text.move(0, -characterHeightOffset);
    }

    target.setView(backupView);
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

void ObjTextList::addString(tiny_utf8::string string)
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
tiny_utf8::string& ObjTextList::getString(std::size_t index)
{
    return this->g_stringList[index];
}
const tiny_utf8::string& ObjTextList::getString(std::size_t index) const
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

void ObjTextList::setBoxSize(const sf::Vector2f& size)
{
    this->g_box.setSize(size);
}
const sf::Vector2f& ObjTextList::getBoxSize()
{
    return this->g_box.getSize();
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
    this->g_textScrollRatio = std::clamp(ratio, 0.0f, 1.0f);
    //this->g_scrollPositionY = -ratio * (this->g_scrollBaseRect.getSize().y-this->g_scrollRect.getSize().y);
    this->refreshPosition(this->g_guiElementHandler->_lastSize);
}
float ObjTextList::getCursorRatio() const
{
    return this->g_textScrollRatio;
    //return std::abs( this->g_scrollPositionY / (this->g_scrollBaseRect.getSize().y-this->g_scrollRect.getSize().y) );
}

void ObjTextList::setMaxStrings(std::size_t max)
{
    this->g_maxStrings = max;
}
std::size_t ObjTextList::getMaxStrings() const
{
    return this->g_maxStrings;
}

void ObjTextList::refreshPosition([[maybe_unused]] const sf::Vector2f& size)
{
    /*this->g_scrollPositionY = std::clamp(this->g_scrollPositionY, -(this->g_scrollBaseRect.getSize().y - this->g_scrollRect.getSize().y), 0.0f);

    auto windowSize = size;

    this->setPosition( sf::Vector2f{0.0f, windowSize.y - this->g_bottomOffset} );

    this->g_scrollRect.setSize({10.0f, 30.0f});
    this->g_scrollBaseRect.setSize({10.0f, this->getPosition().y});
    this->g_scrollRect.setPosition({windowSize.x - 14.0f, this->g_scrollPositionY});
    this->g_scrollBaseRect.setPosition({windowSize.x - 14.0f, 0.0f});

    this->g_scrollBaseRect.setOrigin(0.0f, this->g_scrollBaseRect.getSize().y);
    this->g_scrollRect.setOrigin(0.0f, this->g_scrollRect.getSize().y);*/
}

}//end fge