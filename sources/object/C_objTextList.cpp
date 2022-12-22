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

#include "FastEngine/object/C_objTextList.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjTextList::ObjTextList()
{
    this->g_box.setFillColor(sf::Color::Transparent);
    this->g_box.setOutlineColor(sf::Color{100, 100, 100, 255});
    this->g_box.setOutlineThickness(-2.0f);
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
    this->detachAll();

    this->g_guiElementHandler = guiElementHandlerPtr;

    guiElementHandlerPtr->_onGuiResized.add(new fge::CallbackFunctorObject(&fge::ObjTextList::onGuiResized, this),
                                            this);

    this->refreshSize(guiElementHandlerPtr->_lastSize);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTextList)
{
    states.transform *= this->getTransform();

    target.draw(this->g_box, states);

    sf::View backupView = target.getView();
    sf::View clipView =
            fge::ClipView(backupView, target, states.transform.transformRect({{0.0f, 0.0f}, this->g_box.getSize()}),
                          fge::ClipClampModes::CLIP_CLAMP_HIDE);

    target.setView(clipView);

    if (this->g_stringList.empty())
    {
        return;
    }

    float characterHeightOffset = static_cast<float>(this->g_text.getLineSpacing());

    this->g_text.setPosition(4.0f, this->g_box.getSize().y - characterHeightOffset);
    for (std::size_t i = static_cast<std::size_t>(static_cast<float>(this->g_stringList.size() - 1) *
                                                  this->getTextScrollRatio());
         i < this->g_stringList.size(); ++i)
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

sf::FloatRect ObjTextList::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->getLocalBounds());
}
sf::FloatRect ObjTextList::getLocalBounds() const
{
    return this->g_box.getLocalBounds();
}

void ObjTextList::addString(tiny_utf8::string string)
{
    this->g_stringList.insert(this->g_stringList.begin(), std::move(string));
    if (this->g_stringList.size() > this->g_maxStrings)
    {
        this->g_stringList.erase(this->g_stringList.end() - 1);
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

void ObjTextList::setFont(fge::Font font)
{
    this->g_text.setFont(std::move(font));
}
const fge::Font& ObjTextList::getFont() const
{
    return this->g_text.getFont();
}

void ObjTextList::setBoxSize(const fge::DynamicSize& size)
{
    this->g_boxSize = size;
    this->refreshSize(this->g_guiElementHandler->_lastSize);
}
sf::Vector2f ObjTextList::getBoxSize() const
{
    return this->g_boxSize.getSize(this->getPosition(), this->g_guiElementHandler->_lastSize);
}

void ObjTextList::setTextScrollRatio(float ratio)
{
    this->g_textScrollRatio = std::clamp(ratio, 0.0f, 1.0f);
}
float ObjTextList::getTextScrollRatio() const
{
    return this->g_textScrollRatio;
}

void ObjTextList::setMaxStrings(std::size_t max)
{
    this->g_maxStrings = max;
}
std::size_t ObjTextList::getMaxStrings() const
{
    return this->g_maxStrings;
}

void ObjTextList::refreshSize()
{
    this->refreshSize(this->g_guiElementHandler->_lastSize);
}

void ObjTextList::onGuiResized([[maybe_unused]] const fge::GuiElementHandler& handler, const sf::Vector2f& size)
{
    this->refreshSize(size);
}
void ObjTextList::refreshSize(const sf::Vector2f& targetSize)
{
    this->g_box.setSize(this->g_boxSize.getSize(this->getPosition(), targetSize));
}

} // namespace fge