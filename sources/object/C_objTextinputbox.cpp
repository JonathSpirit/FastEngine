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
ObjTextInputBox::ObjTextInputBox(fge::Font const& font, uint16_t maxLength, fge::Vector2f const& pos) :
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
void ObjTextInputBox::setCharacterSize(fge::CharacterSize size)
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

void ObjTextInputBox::setBoxSize(fge::Vector2f const& size)
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

void ObjTextInputBox::setBoxColor(fge::Color const& color)
{
    this->g_colorBox = color;
    this->g_box.setFillColor(color);
}
void ObjTextInputBox::setBoxOutlineColor(fge::Color const& color)
{
    this->g_colorBoxOutline = color;
    this->g_box.setOutlineColor(color);
}
void ObjTextInputBox::setTextColor(fge::Color const& color)
{
    this->g_colorText = color;
    this->g_text.setFillColor(color);
}

tiny_utf8::string const& ObjTextInputBox::getString() const
{
    return this->g_string;
}
fge::CharacterSize ObjTextInputBox::getCharacterSize() const
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

fge::Vector2f const& ObjTextInputBox::getBoxSize() const
{
    return this->g_boxSize;
}

fge::Color const& ObjTextInputBox::getBoxColor() const
{
    return this->g_colorBox;
}
fge::Color const& ObjTextInputBox::getBoxOutlineColor() const
{
    return this->g_colorBoxOutline;
}
fge::Color const& ObjTextInputBox::getTextColor() const
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
    if (event.isEventType(SDL_KEYDOWN))
    {
        if (this->g_statActive)
        {
            uint32_t key = event.getKeyUnicode();

            if (event.isKeyPressed(SDLK_RETURN))
            {
                this->g_statActive = false;
                return;
            }
            if (event.isKeyPressed(SDLK_LEFT))
            {
                this->g_cursor = this->g_cursor ? (this->g_cursor - 1) : 0;
                return;
            }
            if (event.isKeyPressed(SDLK_RIGHT))
            {
                this->g_cursor =
                        (this->g_cursor < this->g_string.length()) ? (this->g_cursor + 1) : this->g_string.length();
                return;
            }

            //BackSpace
            if (event.isKeyPressed(SDLK_BACKSPACE))
            {
                if (this->g_string.length() > 0 && this->g_cursor > 0)
                {
                    this->g_string.erase(this->g_cursor - 1);
                    --this->g_cursor;
                }
                return;
            }
            //Delete
            if (event.isKeyPressed(SDLK_DELETE))
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

    this->g_box.setFillColor(this->g_statActive ? (this->g_colorBox - fge::Color(50, 50, 50, 0)) : this->g_colorBox);

    auto copyStates = states.copy(this->_transform.start(*this, states._resTransform.get()));
    target.draw(this->g_box, copyStates);
    target.draw(this->g_text, copyStates);
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

    this->g_colorBox = fge::Color(jsonObject.value<uint32_t>("colorBox", 0xFFFFFFFF));
    this->g_colorBoxOutline = fge::Color(jsonObject.value<uint32_t>("colorBoxOutline", 0));
    this->g_colorText = fge::Color(jsonObject.value<uint32_t>("colorText", 0));
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_text.setFillColor(this->g_colorText);

    this->g_string = jsonObject.value<tiny_utf8::string>("string", {});

    this->g_text.setCharacterSize(jsonObject.value<fge::CharacterSize>("characterSize", 12));
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
void ObjTextInputBox::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_cursor >> this->g_maxLength >> this->g_hide;

    pck >> this->g_colorBox >> this->g_colorBoxOutline >> this->g_colorText;
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_text.setFillColor(this->g_colorText);

    pck >> this->g_string;

    fge::CharacterSize tmpCharSize = 12;
    fge::Font tmpFont;
    pck >> tmpCharSize >> tmpFont;
    this->g_text.setCharacterSize(tmpCharSize);
    this->g_text.setFont(tmpFont);

    pck >> this->g_boxSize;
    this->g_box.setSize(this->g_boxSize);

    pck >> this->g_statActive;
}

char const* ObjTextInputBox::getClassName() const
{
    return FGE_OBJTEXTINBOX_CLASSNAME;
}
char const* ObjTextInputBox::getReadableClassName() const
{
    return "text input box";
}

fge::RectFloat ObjTextInputBox::getGlobalBounds() const
{
    return this->getTransform() * this->g_box.getLocalBounds();
}
fge::RectFloat ObjTextInputBox::getLocalBounds() const
{
    return this->g_box.getLocalBounds();
}

void ObjTextInputBox::onGuiMouseButtonPressed([[maybe_unused]] fge::Event const& evt,
                                              [[maybe_unused]] SDL_MouseButtonEvent const& arg,
                                              [[maybe_unused]] fge::GuiElementContext& context)
{
    this->g_statActive = true;
}

void ObjTextInputBox::onGuiVerify([[maybe_unused]] fge::Event const& evt,
                                  SDL_EventType evtType,
                                  fge::GuiElementContext& context)
{
    if (this->verifyPriority(context._prioritizedElement))
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto scrollRect = transform * this->getLocalBounds();

        auto customView = this->_myObjectData.lock()->getLinkedScene()->getCustomView();
        fge::Vector2f mousePosition;
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
        else if (evtType == SDL_MOUSEBUTTONDOWN)
        {
            this->g_statActive = false;
        }
    }
    else if (evtType == SDL_MOUSEBUTTONDOWN)
    {
        this->g_statActive = false;
    }
}

} // namespace fge
