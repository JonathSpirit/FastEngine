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

#include "FastEngine/C_guiElement.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

fge::CallbackHandler<const sf::Vector2f&> GuiElement::_onGlobalGuiScaleChange;
sf::Vector2f GuiElement::_GlobalGuiScale{1.0f, 1.0f};

GuiElement::~GuiElement()
{
    if (auto successor = this->_g_anchorSuccessor.lock())
    {
        if (auto* element = successor->getObject()->getGuiElement())
        {
            element->setAnchor(this->_g_anchorType, this->_g_anchorShift, this->_g_anchorTarget);
        }
    }
}

void GuiElement::updateAnchor()
{
    if (this->_g_anchorType == fge::AnchorType::ANCHOR_NONE)
    {
        return;
    }
    auto parent = this->_g_objectParent.lock();
    if ( !fge::ObjectData::isValid(parent) )
    {
        return;
    }
    auto* scene = parent->getLinkedScene();
    if ( scene == nullptr )
    {
        return;
    }

    sf::Vector2f movePosition;

    sf::FloatRect targetGlobalBounds;
    sf::FloatRect parentGlobalBounds = parent->getObject()->getGlobalBounds();

    auto target = scene->getObject(this->_g_anchorTarget);
    if ( fge::ObjectData::isValid(target) )
    {//On a target
        targetGlobalBounds = target->getObject()->getGlobalBounds();
    }
    else if (this->_g_anchorTarget == FGE_SCENE_BAD_SID)
    {//On the render target
        auto* renderTarget = scene->getLinkedRenderTarget();
        if ( renderTarget != nullptr )
        {
            targetGlobalBounds = {{0.0f, 0.0f}, renderTarget->getDefaultView().getSize()};
        }
    }

    sf::Vector2f anchorPosition;
    switch (this->_g_anchorType)
    {
    case AnchorType::ANCHOR_UPLEFT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition();
        break;
    case AnchorType::ANCHOR_UPRIGHT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition() + sf::Vector2f{targetGlobalBounds.width, 0.0f};
        break;
    case AnchorType::ANCHOR_DOWNLEFT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition() + sf::Vector2f{0.0f, targetGlobalBounds.height};
        break;
    case AnchorType::ANCHOR_DOWNRIGHT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition() + sf::Vector2f{targetGlobalBounds.width, targetGlobalBounds.height};
        break;
    default:
        anchorPosition = {0.0f, 0.0f};
        break;
    }

    switch (this->_g_anchorShift.x)
    {
    case AnchorShift::SHIFT_NONE:
        break;
    case AnchorShift::SHIFT_POSITIVE_BOUNDS:
        anchorPosition.x += parentGlobalBounds.width;
        break;
    case AnchorShift::SHIFT_NEGATIVE_BOUNDS:
        anchorPosition.x -= parentGlobalBounds.width;
        break;
    }
    switch (this->_g_anchorShift.y)
    {
    case AnchorShift::SHIFT_NONE:
        break;
    case AnchorShift::SHIFT_POSITIVE_BOUNDS:
        anchorPosition.y += parentGlobalBounds.height;
        break;
    case AnchorShift::SHIFT_NEGATIVE_BOUNDS:
        anchorPosition.y -= parentGlobalBounds.height;
        break;
    }

    movePosition = anchorPosition - parentGlobalBounds.getPosition();
    parent->getObject()->move(movePosition);
}

}//end fge