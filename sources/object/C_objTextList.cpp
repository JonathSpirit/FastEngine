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

#include "FastEngine/object/C_objTextList.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace fge
{

ObjTextList::ObjTextList()
{
    this->g_box.setFillColor(fge::Color::Transparent);
    this->g_box.setOutlineColor(fge::Color{100, 100, 100, 255});
    this->g_box.setOutlineThickness(-2.0f);
}
ObjTextList::ObjTextList(const ObjTextList& r) :
        fge::Object(r),
        fge::Subscriber(r),

        g_guiElementHandler{nullptr},
        g_textScrollRatio{r.g_textScrollRatio},
        g_boxSize(r.g_boxSize),

        g_font(r.g_font),

        g_textList(r.g_textList),
        g_maxStrings(r.g_maxStrings)
{
    this->g_box.setFillColor(fge::Color::Transparent);
    this->g_box.setOutlineColor(fge::Color{100, 100, 100, 255});
    this->g_box.setOutlineThickness(-2.0f);

    /*this->g_textList.resize(r.g_textList.size());

    for (std::size_t i = 0; i < r.g_textList.size(); ++i)
    {
        this->g_textList[i].reset(reinterpret_cast<fge::ObjText*>(r.g_textList[i]->copy()));
    }*/
}

void ObjTextList::first([[maybe_unused]] fge::Scene* scene)
{
    this->_drawMode = fge::Object::DrawModes::DRAW_ALWAYS_DRAWN;
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
    auto copyStates = states.copy(this->_transform.start(*this, states._resTransform.get()));

    target.draw(this->g_box, copyStates);

    const fge::View backupView = target.getView();
    fge::View clipView = fge::ClipView(backupView, target,
                                       copyStates._resTransform.get()->getData()._modelTransform *
                                               fge::RectFloat{{0.0f, 0.0f}, this->g_box.getSize()},
                                       fge::ClipClampModes::CLIP_CLAMP_HIDE);

    target.setView(clipView);

    if (this->g_textList.empty())
    {
        return;
    }

    float characterHeightOffset = static_cast<float>(this->g_textList.begin()->getLineSpacing());
    fge::Vector2f textPosition = {4.0f, this->g_box.getSize().y - characterHeightOffset};

    std::size_t indexStart =
            static_cast<std::size_t>(static_cast<float>(this->g_textList.size() - 1) * this->getTextScrollRatio());
    if (indexStart >= this->g_textList.size())
    {
        indexStart = this->g_textList.size() - 1;
    }

    for (auto it = std::next(this->g_textList.begin(), indexStart); it != this->g_textList.end(); ++it)
    {
        it->setPosition(textPosition);
        it->draw(target, copyStates);

        textPosition.y -= characterHeightOffset;
        characterHeightOffset = static_cast<float>(it->getLineSpacing());
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

fge::RectFloat ObjTextList::getGlobalBounds() const
{
    return this->getTransform() * this->getLocalBounds();
}
fge::RectFloat ObjTextList::getLocalBounds() const
{
    return this->g_box.getLocalBounds();
}

void ObjTextList::addText(tiny_utf8::string string)
{
    auto& ref = this->g_textList.emplace_front(std::move(string), this->g_font, fge::Vector2f{}, 14);
    ref.setFillColor(fge::Color::White);
    ref.setOutlineColor(fge::Color::Black);
    ref.setOutlineThickness(1.0f);

    if (this->g_textList.size() > this->g_maxStrings)
    {
        this->g_textList.pop_back();
    }
}
std::size_t ObjTextList::getTextCount() const
{
    return this->g_textList.size();
}
fge::ObjText* ObjTextList::getText(std::size_t index)
{
    auto it = std::next(this->g_textList.begin(), index);
    return it == this->g_textList.end() ? nullptr : &(*it);
}
const fge::ObjText* ObjTextList::getText(std::size_t index) const
{
    auto it = std::next(this->g_textList.begin(), index);
    return it == this->g_textList.end() ? nullptr : &(*it);
}
void ObjTextList::removeAllTexts()
{
    this->g_textList.clear();
}

void ObjTextList::setFont(fge::Font font)
{
    this->g_font = std::move(font);

    for (auto& text: this->g_textList)
    {
        text.setFont(this->g_font);
    }
}
const fge::Font& ObjTextList::getFont() const
{
    return this->g_font;
}

void ObjTextList::setBoxSize(const fge::DynamicSize& size)
{
    this->g_boxSize = size;
    this->refreshSize(this->g_guiElementHandler->_lastSize);
}
fge::Vector2f ObjTextList::getBoxSize() const
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

void ObjTextList::setMaxTextCount(std::size_t max)
{
    this->g_maxStrings = max;
}
std::size_t ObjTextList::getMaxTextCount() const
{
    return this->g_maxStrings;
}

void ObjTextList::refreshSize()
{
    this->refreshSize(this->g_guiElementHandler->_lastSize);
}

void ObjTextList::onGuiResized([[maybe_unused]] const fge::GuiElementHandler& handler, const fge::Vector2f& size)
{
    this->refreshSize(size);
}
void ObjTextList::refreshSize(const fge::Vector2f& targetSize)
{
    this->g_box.setSize(this->g_boxSize.getSize(this->getPosition(), targetSize));
}

} // namespace fge