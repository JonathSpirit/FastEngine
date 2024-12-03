/*
 * Copyright 2024 Guillaume Guillet
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
    this->g_text.setFillColor(fge::Color::Black);

    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_box.setOutlineThickness(1.0f);
    this->g_box.setSize(this->g_boxSize);

    this->g_cursorLine.setFillColor(fge::Color::Black);

    this->setCharacterSize(12);
}
ObjTextInputBox::ObjTextInputBox(fge::Font const& font, uint16_t maxLength, fge::Vector2f const& pos) :
        g_maxLength(maxLength),
        g_text(font)
{
    this->g_text.setFillColor(fge::Color::Black);

    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_box.setOutlineThickness(1.0f);
    this->g_box.setSize(this->g_boxSize);

    this->g_cursorLine.setFillColor(fge::Color::Black);
    this->g_cursorLine.setOrigin({0.0f, -4.0f});

    this->setCharacterSize(12);

    this->setPosition(pos);
}

void ObjTextInputBox::setString(tiny_utf8::string string)
{
    this->g_string = std::move(string);
    if (this->g_hide)
    {
        this->g_text.setString(tiny_utf8::string(this->g_string.length(), U'*'));
    }
    else
    {
        this->g_text.setString(this->g_string);
    }
    this->g_cursor = this->g_string.length();
}
void ObjTextInputBox::setFont(fge::Font font)
{
    this->g_text.setFont(std::move(font));
}
void ObjTextInputBox::setCharacterSize(fge::CharacterSize size)
{
    this->g_text.setCharacterSize(size);
    this->g_cursorLine.setSize({1.0f, static_cast<float>(this->g_text.getCharacterSize())});
}
void ObjTextInputBox::setHideTextFlag(bool flag)
{
    this->g_hide = flag;
    if (flag)
    {
        this->g_text.setString(tiny_utf8::string(this->g_string.length(), U'*'));
    }
    else
    {
        this->g_text.setString(this->g_string);
    }
}
void ObjTextInputBox::setMaxLength(uint16_t length)
{
    this->g_maxLength = length;
}

void ObjTextInputBox::setActiveStat(bool active)
{
    if (this->g_statActive != active)
    {
        this->g_statActive = active;
        this->_onStatChange.call(*this);
    }
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
    return this->g_text.getFillColor();
}

void ObjTextInputBox::callbackRegister([[maybe_unused]] fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    guiElementHandlerPtr->_onGuiVerify.addObjectFunctor(&ObjTextInputBox::onGuiVerify, this, this);

    this->_onGuiMouseButtonPressed.addObjectFunctor(&ObjTextInputBox::onGuiMouseButtonPressed, this, this);
    event._onTextInput.addObjectFunctor(&ObjTextInputBox::onTextInput, this, this);
    event._onKeyDown.addObjectFunctor(&ObjTextInputBox::onKeyDown, this, this);
}

#ifdef FGE_DEF_SERVER
FGE_OBJ_UPDATE_BODY(ObjTextInputBox) {}
#else
FGE_OBJ_UPDATE_BODY(ObjTextInputBox)
{
    this->g_cursorBlinkTime += std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime);
    if (this->g_cursorBlinkTime >= std::chrono::milliseconds{500})
    {
        this->g_cursorBlinkTime = std::chrono::milliseconds{0};
        bool const isVisible = this->g_cursorLine.getFillColor() != fge::Color::Transparent;
        this->g_cursorLine.setFillColor(isVisible ? fge::Color::Transparent : fge::Color::Black);
    }
}
#endif

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjTextInputBox)
{
    if (this->g_cursor >= this->g_string.length() && !this->g_string.empty())
    {
        auto const advance = this->g_text.getGlyphAdvance(this->g_string[this->g_string.length() - 1]);
        this->g_cursorLine.setPosition(this->g_text.findCharacterPos(this->g_string.length() - 1) +
                                       fge::Vector2f{advance, 0});
    }
    else
    {
        this->g_cursorLine.setPosition(this->g_text.findCharacterPos(this->g_cursor));
    }

    this->g_box.setFillColor(this->g_statActive ? this->g_colorBox - fge::Color(50, 50, 50, 0) : this->g_colorBox);

    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));
    this->g_box.draw(target, copyStates);
    this->g_text.draw(target, copyStates);
    if (this->g_statActive)
    {
        this->g_cursorLine.draw(target, copyStates);
    }
}
#endif

void ObjTextInputBox::save(nlohmann::json& jsonObject)
{
    fge::Object::save(jsonObject);

    jsonObject["cursor"] = this->g_cursor;
    jsonObject["maxLength"] = this->g_maxLength;
    jsonObject["hide"] = this->g_hide;

    jsonObject["colorBox"] = this->g_colorBox.toInteger();
    jsonObject["colorBoxOutline"] = this->g_colorBoxOutline.toInteger();
    jsonObject["colorText"] = this->g_text.getFillColor().toInteger();

    jsonObject["string"] = this->g_string;

    jsonObject["characterSize"] = this->g_text.getCharacterSize();
    jsonObject["font"] = this->g_text.getFont();

    jsonObject["boxSizeX"] = this->g_boxSize.x;
    jsonObject["boxSizeY"] = this->g_boxSize.y;

    jsonObject["statActive"] = this->g_statActive;
}
void ObjTextInputBox::load(nlohmann::json& jsonObject)
{
    fge::Object::load(jsonObject);

    this->g_cursor = jsonObject.value<uint16_t>("cursor", 0);
    this->g_maxLength = jsonObject.value<uint16_t>("maxLength", 10);
    this->g_hide = jsonObject.value<bool>("hide", false);

    this->g_colorBox = fge::Color(jsonObject.value<uint32_t>("colorBox", 0xFFFFFFFF));
    this->g_colorBoxOutline = fge::Color(jsonObject.value<uint32_t>("colorBoxOutline", 0));
    this->g_text.setFillColor(fge::Color(jsonObject.value<uint32_t>("colorText", 0)));
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);

    this->setString(jsonObject.value<tiny_utf8::string>("string", {}));

    this->g_text.setCharacterSize(jsonObject.value<fge::CharacterSize>("characterSize", 12));
    this->g_text.setFont(jsonObject.value<std::string>("font", std::string(FGE_FONT_BAD)));

    this->g_boxSize.x = jsonObject.value<float>("boxSizeX", 120);
    this->g_boxSize.y = jsonObject.value<float>("boxSizeY", 18);
    this->g_box.setSize(this->g_boxSize);

    this->g_statActive = jsonObject.value<bool>("statActive", false);
}

void ObjTextInputBox::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_cursor << this->g_maxLength << this->g_hide;

    pck << this->g_colorBox << this->g_colorBoxOutline << this->g_text.getFillColor();

    pck << this->g_string;

    pck << this->g_text.getCharacterSize() << this->g_text.getFont();

    pck << this->g_boxSize;

    pck << this->g_statActive;
}
void ObjTextInputBox::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_cursor >> this->g_maxLength >> this->g_hide;

    fge::Color color;
    pck >> this->g_colorBox >> this->g_colorBoxOutline >> color;
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_text.setFillColor(color);

    tiny_utf8::string string;
    pck >> string;
    this->setString(std::move(string));

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
    this->setActiveStat(true);
}
void ObjTextInputBox::onTextInput(fge::Event const& evt, [[maybe_unused]] SDL_TextInputEvent const& arg)
{
    if (!this->g_statActive)
    {
        return;
    }

    auto const key = evt.getKeyUnicode();

    //Ignore Unicode control char
    if (key < 32 || (key > 127 && key < 161))
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
    if (this->g_hide)
    {
        this->g_text.setString(tiny_utf8::string(this->g_string.length(), U'*'));
    }
    else
    {
        this->g_text.setString(this->g_string);
    }
}
void ObjTextInputBox::onKeyDown([[maybe_unused]] fge::Event const& evt, SDL_KeyboardEvent const& arg)
{
    if (!this->g_statActive)
    {
        return;
    }

    switch (arg.keysym.sym)
    {
    case SDLK_RETURN:
        this->setActiveStat(false);
        this->_onReturn.call(*this);
        break;

    case SDLK_LEFT:
        this->g_cursor = this->g_cursor != 0 ? this->g_cursor - 1 : 0;
        this->g_cursorBlinkTime = std::chrono::milliseconds{0};
        break;
    case SDLK_RIGHT:
        this->g_cursor = this->g_cursor < this->g_string.length() ? this->g_cursor + 1 : this->g_string.length();
        this->g_cursorBlinkTime = std::chrono::milliseconds{0};
        break;

    case SDLK_BACKSPACE:
        if (!this->g_string.empty() && this->g_cursor > 0)
        {
            this->g_string.erase(this->g_cursor - 1);
            if (this->g_hide)
            {
                this->g_text.setString(tiny_utf8::string(this->g_string.length(), U'*'));
            }
            else
            {
                this->g_text.setString(this->g_string);
            }
            --this->g_cursor;
            this->g_cursorBlinkTime = std::chrono::milliseconds{0};
        }
        break;
    case SDLK_DELETE:
        if (this->g_cursor < this->g_string.length())
        {
            this->g_string.erase(this->g_cursor);
            if (this->g_hide)
            {
                this->g_text.setString(tiny_utf8::string(this->g_string.length(), U'*'));
            }
            else
            {
                this->g_text.setString(this->g_string);
            }
            this->g_cursorBlinkTime = std::chrono::milliseconds{0};
        }
        break;

    default:
        break;
    }
}

void ObjTextInputBox::onGuiVerify([[maybe_unused]] fge::Event const& evt,
                                  SDL_EventType evtType,
                                  fge::GuiElementContext& context)
{
    if (this->verifyPriority(context._prioritizedElement))
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto scrollRect = transform * this->getLocalBounds();

        auto customView = this->_myObjectData.lock()->getScene()->getCustomView();
        fge::Vector2f mousePosition;
        if (customView)
        {
            mousePosition = context._handler->getRenderTarget().mapFramebufferCoordsToWorldSpace(context._mousePosition,
                                                                                                 *customView);
        }
        else
        {
            mousePosition =
                    context._handler->getRenderTarget().mapFramebufferCoordsToWorldSpace(context._mousePosition);
        }

        if (scrollRect.contains(mousePosition))
        {
            context._prioritizedElement = this;
        }
        else if (evtType == SDL_MOUSEBUTTONDOWN)
        {
            this->setActiveStat(false);
        }
    }
    else if (evtType == SDL_MOUSEBUTTONDOWN)
    {
        this->setActiveStat(false);
    }
}

} // namespace fge
