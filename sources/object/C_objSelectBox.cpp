/*
 * Copyright 2025 Guillaume Guillet
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

#include "FastEngine/object/C_objSelectBox.hpp"
#include "FastEngine/C_scene.hpp"
#include "FastEngine/arbitraryJsonTypes.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define FGE_OBJSELECTBOX_SELECT_COLOR fge::Color(100, 100, 0, 0)

namespace fge
{

ObjSelectBox::ObjSelectBox()
{
    this->g_textSelected.setFillColor(this->g_colorText);
    this->g_textSelected.setCharacterSize(12);

    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_box.setOutlineThickness(1.0f);
    this->g_box.setSize({120.0f, 18.0f});
}
ObjSelectBox::ObjSelectBox(fge::Font font, fge::Vector2f const& pos) :
        ObjSelectBox()
{
    this->g_textSelected.setFont(std::move(font));
    this->setPosition(pos);
}

std::size_t ObjSelectBox::getItemCount() const
{
    return this->g_textList.size();
}
tiny_utf8::string const* ObjSelectBox::getItem(std::size_t index) const
{
    if (index < this->g_textList.size())
    {
        return &this->g_textList[index].getString();
    }
    return nullptr;
}
bool ObjSelectBox::setItem(std::size_t index, tiny_utf8::string text)
{
    if (index < this->g_textList.size())
    {
        this->g_textList[index].setString(std::move(text));
        return true;
    }
    return false;
}
void ObjSelectBox::addItem(tiny_utf8::string text)
{
    this->g_textList.emplace_back(std::move(text), this->g_textSelected.getFont(), fge::Vector2f{},
                                  this->g_textSelected.getCharacterSize());
    this->g_textList.back().setFillColor(this->g_colorText);
    this->g_textList.back().setPosition({0.0f, (this->g_box.getSize().y + this->g_box.getOutlineThickness() * 2.0f) *
                                                       static_cast<float>(this->g_textList.size())});
    this->updateBoxInstances();
}
void ObjSelectBox::clearItems()
{
    this->g_textList.clear();
    this->updateBoxInstances();
}

void ObjSelectBox::setSelectedText(tiny_utf8::string string)
{
    this->g_textSelected.setString(std::move(string));
}
tiny_utf8::string const& ObjSelectBox::getSelectedText() const
{
    return this->g_textSelected.getString();
}
void ObjSelectBox::clearSelectedText()
{
    this->g_textSelected.setString({});
}

void ObjSelectBox::setCharacterSize(fge::CharacterSize size)
{
    this->g_textSelected.setCharacterSize(size);
    for (auto& item: this->g_textList)
    {
        item.setCharacterSize(size);
    }
}

void ObjSelectBox::setActiveStat(bool active)
{
    this->g_statActive = active;
    if (!active)
    {
        this->g_cursor = std::numeric_limits<std::size_t>::max();
    }
    this->updateBoxInstances();
}
bool ObjSelectBox::getActiveStat() const
{
    return this->g_statActive;
}

void ObjSelectBox::setBoxSize(fge::Vector2f const& size)
{
    this->g_box.setSize(size);
    for (std::size_t i = 0; i < this->g_textList.size(); ++i)
    {
        this->g_textList[i].setPosition({0.0f, size.y * static_cast<float>(i + 1)});
    }
    this->updateBoxInstances();
}
void ObjSelectBox::setBoxColor(fge::Color color)
{
    this->g_colorBox = color;
    this->g_box.setFillColor(color);
}
void ObjSelectBox::setBoxOutlineColor(fge::Color color)
{
    this->g_colorBoxOutline = color;
    this->g_box.setOutlineColor(color);
}
void ObjSelectBox::setTextColor(fge::Color color)
{
    this->g_colorText = color;
    this->g_textSelected.setFillColor(color);
    for (auto& item: this->g_textList)
    {
        item.setFillColor(color);
    }
}

fge::CharacterSize ObjSelectBox::getCharacterSize() const
{
    return this->g_textSelected.getCharacterSize();
}

fge::Vector2f const& ObjSelectBox::getBoxSize() const
{
    return this->g_box.getSize();
}
fge::Color ObjSelectBox::getBoxColor() const
{
    return this->g_colorBox;
}
fge::Color ObjSelectBox::getBoxOutlineColor() const
{
    return this->g_colorBoxOutline;
}
fge::Color ObjSelectBox::getTextColor() const
{
    return this->g_colorText;
}

void ObjSelectBox::callbackRegister([[maybe_unused]] fge::Event& event, fge::GuiElementHandler* guiElementHandlerPtr)
{
    this->detachAll();

    guiElementHandlerPtr->_onGuiVerify.addObjectFunctor(&fge::ObjSelectBox::onGuiVerify, this, this);

    this->_onGuiMouseButtonPressed.addObjectFunctor(&fge::ObjSelectBox::onGuiMouseButtonPressed, this, this);
    this->_onGuiMouseMoved.addObjectFunctor(&fge::ObjSelectBox::onGuiMouseMotion, this, this);
}

#ifndef FGE_DEF_SERVER
FGE_OBJ_DRAW_BODY(ObjSelectBox)
{
    auto copyStates = states.copy();
    copyStates._resTransform.set(target.requestGlobalTransform(*this, states._resTransform));

    this->g_box.draw(target, copyStates);
    this->g_textSelected.draw(target, copyStates);

    if (this->g_statActive)
    {
        for (auto const& item: this->g_textList)
        {
            item.draw(target, copyStates);
        }
    }
}
#endif

void ObjSelectBox::save(nlohmann::json& jsonObject)
{
    fge::Object::save(jsonObject);

    jsonObject["colorBox"] = this->g_colorBox.toInteger();
    jsonObject["colorBoxOutline"] = this->g_colorBoxOutline.toInteger();
    jsonObject["colorText"] = this->g_colorText.toInteger();

    auto& jsonItemArray = jsonObject["texts"];
    jsonItemArray = nlohmann::json::array();

    for (auto& item: this->g_textList)
    {
        jsonItemArray.push_back(item.getString());
    }
    jsonObject["textSelected"] = this->g_textSelected.getString();

    jsonObject["characterSize"] = this->g_textSelected.getCharacterSize();
    jsonObject["font"] = this->g_textSelected.getFont();

    jsonObject["boxSizeX"] = this->g_box.getSize().x;
    jsonObject["boxSizeY"] = this->g_box.getSize().y;

    jsonObject["statActive"] = this->g_statActive;
    jsonObject["statMouseOn"] = this->g_statMouseOn;
}
void ObjSelectBox::load(nlohmann::json& jsonObject, std::filesystem::path const& filePath)
{
    fge::Object::load(jsonObject, filePath);

    this->g_colorBox = fge::Color(jsonObject.value<uint32_t>("colorBox", 0xFFFFFFFF));
    this->g_colorBoxOutline = fge::Color(jsonObject.value<uint32_t>("colorBoxOutline", 0));
    this->g_colorText = fge::Color(jsonObject.value<uint32_t>("colorText", 0));
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);
    this->g_textSelected.setFillColor(this->g_colorText);

    this->g_textList.clear();
    for (auto& it: jsonObject.at("texts"))
    {
        this->addItem(it.get<tiny_utf8::string>());
    }
    this->g_textSelected.setString(jsonObject.value<tiny_utf8::string>("textSelected", {}));

    this->g_textSelected.setCharacterSize(jsonObject.value<fge::CharacterSize>("characterSize", 12));
    this->g_textSelected.setFont(jsonObject.value<fge::Font>("font", FGE_FONT_BAD));

    fge::Vector2f boxSize;
    boxSize.x = jsonObject.value<float>("boxSizeX", 120);
    boxSize.y = jsonObject.value<float>("boxSizeY", 18);
    this->g_box.setSize(boxSize);

    this->g_statActive = jsonObject.value<bool>("statActive", false);
    this->g_statMouseOn = jsonObject.value<bool>("statMouseOn", false);

    this->updateBoxInstances();
}

void ObjSelectBox::pack(fge::net::Packet& pck)
{
    fge::Object::pack(pck);

    pck << this->g_colorBox << this->g_colorBoxOutline << this->g_colorText;

    pck << static_cast<fge::net::SizeType>(this->g_textList.size());
    for (auto& item: this->g_textList)
    {
        item.pack(pck);
    }

    this->g_textSelected.pack(pck);

    pck << this->g_box.getSize();
    pck << this->g_statActive << this->g_statMouseOn;
}
void ObjSelectBox::unpack(fge::net::Packet const& pck)
{
    fge::Object::unpack(pck);

    pck >> this->g_colorBox >> this->g_colorBoxOutline >> this->g_colorText;
    this->g_box.setFillColor(this->g_colorBox);
    this->g_box.setOutlineColor(this->g_colorBoxOutline);

    this->g_textList.clear();
    fge::net::SizeType itemCount = 0;
    pck >> itemCount;
    for (fge::net::SizeType i = 0; i < itemCount; ++i)
    {
        this->g_textList.emplace_back().unpack(pck);
    }

    this->g_textSelected.unpack(pck);

    fge::Vector2f boxSize;
    pck >> boxSize;
    this->g_box.setSize(boxSize);
    pck >> this->g_statActive >> this->g_statMouseOn;

    this->updateBoxInstances();
}

char const* ObjSelectBox::getClassName() const
{
    return FGE_OBJSELECTBOX_CLASSNAME;
}
char const* ObjSelectBox::getReadableClassName() const
{
    return "selection box";
}

fge::RectFloat ObjSelectBox::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjSelectBox::getLocalBounds() const
{
    if (this->g_statActive)
    {
        auto rect = this->g_box.getLocalBounds();
        rect._height += rect._height * static_cast<float>(this->g_textList.size());
        return rect;
    }
    return this->g_box.getLocalBounds();
}

void ObjSelectBox::onGuiMouseMotion([[maybe_unused]] fge::Event const& evt,
                                    [[maybe_unused]] SDL_MouseMotionEvent const& arg,
                                    [[maybe_unused]] fge::GuiElementContext& context)
{
    if (!this->g_statActive)
    {
        return;
    }

    auto const& view = this->requestView(context._handler->getRenderTarget(), this->_myObjectData);
    fge::Vector2f mousePosition =
            context._handler->getRenderTarget().mapFramebufferCoordsToWorldSpace(context._mousePosition, view);

    auto transform = this->getParentsTransform() * this->getTransform();

    auto boxRect = transform * this->getLocalBounds();

    float const individualBoxHeight = boxRect._height / static_cast<float>(this->g_textList.size() + 1);

    auto cursor = static_cast<std::size_t>((mousePosition.y - boxRect._y) / individualBoxHeight);
    if (cursor != 0)
    {
        cursor -= 1;

        if (cursor < this->g_textList.size())
        {
            if (this->g_cursor < this->g_textList.size())
            {
                this->g_box.setFillColor(this->g_colorBox, this->g_cursor + 1);
            }
            this->g_box.setFillColor(this->g_colorBox - FGE_OBJSELECTBOX_SELECT_COLOR, cursor + 1);
            this->g_cursor = cursor;
        }
    }
}
void ObjSelectBox::onGuiMouseButtonPressed([[maybe_unused]] fge::Event const& evt,
                                           [[maybe_unused]] SDL_MouseButtonEvent const& arg,
                                           [[maybe_unused]] fge::GuiElementContext& context)
{
    if (this->g_statActive)
    {
        if (this->g_cursor < this->g_textList.size())
        {
            this->g_textSelected.setString(this->g_textList[this->g_cursor].getString());
            this->_onSelect.call(*this, this->g_cursor);
            this->setActiveStat(false);
        }
    }
    else
    {
        this->setActiveStat(true);
    }
}

void ObjSelectBox::onGuiVerify([[maybe_unused]] fge::Event const& evt,
                               SDL_EventType evtType,
                               fge::GuiElementContext& context)
{
    if (this->verifyPriority(context._prioritizedElement))
    {
        auto transform = this->getParentsTransform() * this->getTransform();

        auto boxRect = transform * this->getLocalBounds();

        auto const& view = this->requestView(context._handler->getRenderTarget(), this->_myObjectData);
        fge::Vector2f mousePosition =
                context._handler->getRenderTarget().mapFramebufferCoordsToWorldSpace(context._mousePosition, view);

        if (boxRect.contains(mousePosition))
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

void ObjSelectBox::updateBoxInstances()
{
    if (this->g_statActive)
    {
        this->g_box.setInstancesCount(this->g_textList.size() + 1);
        for (std::size_t i = 0; i < this->g_textList.size(); ++i)
        {
            this->g_box.setOffset({0.0f, (this->g_box.getSize().y + this->g_box.getOutlineThickness() * 2.0f) *
                                                 static_cast<float>(i + 1)},
                                  i + 1);

            this->g_box.setFillColor((i == this->g_cursor) ? this->g_colorBox - FGE_OBJSELECTBOX_SELECT_COLOR
                                                           : this->g_colorBox);

            this->g_box.setOutlineColor(this->g_colorBoxOutline, i + 1);
        }
    }
    else
    {
        this->g_box.setInstancesCount(1);
    }
}

} // namespace fge
