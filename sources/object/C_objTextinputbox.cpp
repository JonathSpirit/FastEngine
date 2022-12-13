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

#include "FastEngine/object/C_objTextinputbox.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/arbitraryJsonTypes.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjTextInputBox::ObjTextInputBox()
{
    this->g_text.setFillColor(this->g_colorText);
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_box.setOutlineThickness(1.0f);
    this->g_box.setSize(this->g_boxSize);

    this->g_text.setCharacterSize(12);
}
ObjTextInputBox::ObjTextInputBox(const fge::Font& font, uint16_t maxLength, const sf::Vector2f& pos) :
        g_maxLength(maxLength),
        g_text(font)
{
    this->g_text.setFillColor(this->g_colorText);
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_box.setOutlineThickness(1.0f);
    this->g_box.setSize(this->g_boxSize);

    this->g_text.setCharacterSize(12);

    this->setPosition(pos);
}

void ObjTextInputBox::setString(tiny_utf8::string string)
{
    this->g_string = std::move(string);
}
void ObjTextInputBox::setCharacterSize(fge::ObjText::CharacterSize size)
{
    this->g_text.setCharacterSize(size);
}
void ObjTextInputBox::setHideTextFlag(bool flag)
{
    this->g_hide = flag;
}
void ObjTextInputBox::setMaxLength(uint16_t length)
{
    this->g_maxLength = length;
}

void ObjTextInputBox::setActiveStat(bool active)
{
    this->g_statActive = active;
}

void ObjTextInputBox::setBoxSize(const sf::Vector2f& size)
{
    this->g_boxSize = size;
    this->g_box.setSize(this->g_boxSize);
}
void ObjTextInputBox::setBoxSize(float w, float h)
{
    this->g_boxSize.x = w;
    this->g_boxSize.y = h;
    this->g_box.setSize(this->g_boxSize);
}

void ObjTextInputBox::setBoxColor(const sf::Color& color)
{
    this->g_colorBox = color;
    this->g_box.setFillColor(color);
}
void ObjTextInputBox::setBoxOutlineColor(const sf::Color& color)
{
    this->g_colorBoxOutline = color;
    this->g_box.setOutlineColor(color);
}
void ObjTextInputBox::setTextColor(const sf::Color& color)
{
    this->g_colorText = color;
    this->g_text.setFillColor(color);
}

const tiny_utf8::string& ObjTextInputBox::getString() const
{
    return this->g_string;
}
fge::ObjText::CharacterSize ObjTextInputBox::getCharacterSize() const
{
    return this->g_text.getCharacterSize();
}
bool ObjTextInputBox::isTextHide() const
{
    return this->g_hide;
}
uint16_t ObjTextInputBox::getMaxLength() const
{
    return this->g_maxLength;
}

bool ObjTextInputBox::getActiveStat() const
{
    return this->g_statActive;
}

const sf::Vector2f& ObjTextInputBox::getBoxSize() const
{
    return this->g_boxSize;
}

const sf::Color& ObjTextInputBox::getBoxColor() const
{
    return this->g_colorBox;
}
const sf::Color& ObjTextInputBox::getBoxOutlineColor() const
{
    return this->g_colorBoxOutline;
}
const sf::Color& ObjTextInputBox::getTextColor() const
{
    return this->g_colorText;
}

void ObjTextInputBox::callbackRegister([[maybe_unused]] fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    guiElementHandlerPtr->_onGuiVerify.add(new fge::CallbackFunctorObject(&fge::ObjTextInputBox::onGuiVerify, this),
                                           this);

    this->_onGuiMouseButtonPressed.add(
            new fge::CallbackFunctorObject(&fge::ObjTextInputBox::onGuiMouseButtonPressed, this), this);
}

#ifdef FGE_DEF_SERVER
FGE_OBJ_UPDATE_BODY(ObjTextInputBox) {}
#else
FGE_OBJ_UPDATE_BODY(ObjTextInputBox)
{
    if (event.isEventType(sf::Event::EventType::KeyPressed))
    {
        if (this->g_statActive)
        {
            uint32_t key = event.getKeyUnicode();

            if (event.isKeyPressed(sf::Keyboard::Return))
            {
                this->g_statActive = false;
                return;
            }
            if (event.isKeyPressed(sf::Keyboard::Left))
            {
                this->g_cursor = this->g_cursor ? (this->g_cursor - 1) : 0;
                return;
            }
            if (event.isKeyPressed(sf::Keyboard::Right))
            {
                this->g_cursor =
                        (this->g_cursor < this->g_string.length()) ? (this->g_cursor + 1) : this->g_string.length();
                return;
            }

            //BackSpace
            if (key == 8)
            {
                if (this->g_string.length() > 0 && this->g_cursor > 0)
                {
                    this->g_string.erase(this->g_cursor - 1);
                    --this->g_cursor;
                }
                return;
            }
            //Delete
            if (event.isKeyPressed(sf::Keyboard::Key::Delete))
            {
                if (this->g_cursor < this->g_string.length())
                {
                    this->g_string.erase(this->g_cursor);
                }
                return;
            }

            //Ignore Unicode control char
            if ((key < 32) || ((key > 127) && (key < 161)))
            {
                return;
            }

            //Insert Unicode char
            if (this->g_string.length() < this->g_maxLength)
            {
                if (this->g_cursor >= this->g_string.length())
                {
                    this->g_string += key;
                }
                else
                {
                    this->g_string.insert(this->g_cursor, key);
                }
                ++this->g_cursor;
            }
        }
    }
}
#endif

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTextInputBox)
{
    tiny_utf8::string tmpString;

    if (this->g_hide)
    {
        tmpString.assign(this->g_string.length(), '*');
    }
    else
    {
        tmpString = this->g_string;
    }

    if (this->g_statActive)
    {
        if (this->g_cursor >= this->g_string.length())
        {
            tmpString += '|';
        }
        else
        {
            tmpString.insert(this->g_cursor, '|');
        }
    }

    this->g_text.setString(tmpString);

    this->g_box.setFillColor(this->g_statActive ? (this->g_colorBox - sf::Color(50, 50, 50, 0)) : this->g_colorBox);

    states.transform *= this->getTransform();
    target.draw(this->g_box, states);
    target.draw(this->g_text, states);
}
#endif

void ObjTextInputBox::save(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::save(jsonObject, scene);

    jsonObject["cursor"] = this->g_cursor;
    jsonObject["maxLength"] = this->g_maxLength;
    jsonObject["hide"] = this->g_hide;

    jsonObject["colorBox"] = this->g_colorBox.toInteger();
    jsonObject["colorBoxOutline"] = this->g_colorBoxOutline.toInteger();
    jsonObject["colorText"] = this->g_colorText.toInteger();

    jsonObject["string"] = this->g_string;

    jsonObject["characterSize"] = this->g_text.getCharacterSize();
    jsonObject["font"] = this->g_text.getFont();

    jsonObject["boxSizeX"] = this->g_boxSize.x;
    jsonObject["boxSizeY"] = this->g_boxSize.y;

    jsonObject["statActive"] = this->g_statActive;
}
void ObjTextInputBox::load(nlohmann::json& jsonObject, fge::Scene* scene)
{
    fge::Object::load(jsonObject, scene);

    this->g_cursor = jsonObject.value<uint16_t>("cursor", 0);
    this->g_maxLength = jsonObject.value<uint16_t>("maxLength", 10);
    this->g_hide = jsonObject.value<bool>("hide", false);

    this->g_colorBox = sf::Color(jsonObject.value<uint32_t>("colorBox", 0xFFFFFFFF));
    this->g_colorBoxOutline = sf::Color(jsonObject.value<uint32_t>("colorBoxOutline", 0));
    this->g_colorText = sf::Color(jsonObject.value<uint32_t>("colorText", 0));
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_text.setFillColor(this->g_colorText);

    this->g_string = jsonObject.value<tiny_utf8::string>("string", {});

    this->g_text.setCharacterSize(jsonObject.value<fge::ObjText::CharacterSize>("characterSize", 12));
    this->g_text.setFont(jsonObject.value<std::string>("font", FGE_FONT_BAD));

    this->g_boxSize.x = jsonObject.value<float>("boxSizeX", 120);
    this->g_boxSize.y = jsonObject.value<float>("boxSizeY", 18);
    this->g_box.setSize(this->g_boxSize);

    this->g_statActive = jsonObject.value<bool>("statActive", false);
}

void ObjTextInputBox::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_cursor << this->g_maxLength << this->g_hide;

    pck << this->g_colorBox << this->g_colorBoxOutline << this->g_colorText;

    pck << this->g_string;

    pck << this->g_text.getCharacterSize() << this->g_text.getFont();

    pck << this->g_boxSize;

    pck << this->g_statActive;
}
void ObjTextInputBox::unpack(fge::net::Packet& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_cursor >> this->g_maxLength >> this->g_hide;

    pck >> this->g_colorBox >> this->g_colorBoxOutline >> this->g_colorText;
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_text.setFillColor(this->g_colorText);

    pck >> this->g_string;

    fge::ObjText::CharacterSize tmpCharSize = 12;
    fge::Font tmpFont;
    pck >> tmpCharSize >> tmpFont;
    this->g_text.setCharacterSize(tmpCharSize);
    this->g_text.setFont(tmpFont);

    pck >> this->g_boxSize;
    this->g_box.setSize(this->g_boxSize);

    pck >> this->g_statActive;
}

const char* ObjTextInputBox::getClassName() const
{
    return FGE_OBJTEXTINBOX_CLASSNAME;
}
const char* ObjTextInputBox::getReadableClassName() const
{
    return "text input box";
}

sf::FloatRect ObjTextInputBox::getGlobalBounds() const
{
    return this->getTransform().transformRect(this->g_box.getLocalBounds());
}
sf::FloatRect ObjTextInputBox::getLocalBounds() const
{
    return this->g_box.getLocalBounds();
}

void ObjTextInputBox::onGuiMouseButtonPressed([[maybe_unused]] const fge::Event& evt,
                                              [[maybe_unused]] const sf::Event::MouseButtonEvent& arg,
                                              [[maybe_unused]] fge::GuiElementContext& context)
{
    this->g_statActive = true;
}

void ObjTextInputBox::onGuiVerify([[maybe_unused]] const fge::Event& evt,
                                  sf::Event::EventType evtType,
                                  fge::GuiElementContext& context)
{
    if (this->verifyPriority(context._prioritizedElement))
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto scrollRect = transform.transformRect(this->getLocalBounds());

        auto customView = this->_myObjectData.lock()->getLinkedScene()->getCustomView();
        sf::Vector2f mousePosition;
        if (customView)
        {
            mousePosition = context._handler->getRenderTarget().mapPixelToCoords(context._mousePosition, *customView);
        }
        else
        {
            mousePosition = context._handler->getRenderTarget().mapPixelToCoords(context._mousePosition);
        }

        if (scrollRect.contains(mousePosition))
        {
            context._prioritizedElement = this;
        }
        else if (evtType == sf::Event::EventType::MouseButtonPressed)
        {
            this->g_statActive = false;
        }
    }
    else if (evtType == sf::Event::EventType::MouseButtonPressed)
    {
        this->g_statActive = false;
    }
}

} // namespace fge
