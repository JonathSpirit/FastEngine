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

GuiElement::~GuiElement()
{
    if (auto successor = this->_g_anchorSuccessor.lock())
    {
        if (auto* element = successor->getObject()->getGuiElement())
        {
            element->setAnchor(this->_g_anchorType, this->_g_anchorTarget);
        }
    }
}

void GuiElement::updateAnchor()
{
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

    switch (this->_g_anchorType)
    {
    default:
    case ANCHOR_NONE:
        break;
    case ANCHOR_TOP_LEFT:
    {
        auto target = scene->getObject(this->_g_anchorTarget);
        if ( fge::ObjectData::isValid(target) )
        {//On a target
            auto targetGlobalBounds = target->getObject()->getGlobalBounds();
            auto parentGlobalBounds = parent->getObject()->getGlobalBounds();

            sf::Vector2f targetAnchorPosition = targetGlobalBounds.getPosition() + sf::Vector2f{targetGlobalBounds.width, 0.0f};
            sf::Vector2f movePosition = targetAnchorPosition - parentGlobalBounds.getPosition();

            parent->getObject()->move(movePosition);
        }
        else if (this->_g_anchorTarget == FGE_SCENE_BAD_SID)
        {//On the render target
            auto* renderTarget = scene->getLinkedRenderTarget();
            if ( renderTarget != nullptr )
            {
                auto parentGlobalBounds = parent->getObject()->getGlobalBounds();

                sf::Vector2f targetAnchorPosition = sf::Vector2f{0.0f, 0.0f};
                sf::Vector2f movePosition = targetAnchorPosition - parentGlobalBounds.getPosition();

                parent->getObject()->move(movePosition);
            }
        }
    }
        break;
    case ANCHOR_BOT_LEFT:
    {
        auto target = scene->getObject(this->_g_anchorTarget);
        if ( fge::ObjectData::isValid(target) )
        {
            auto targetGlobalBounds = target->getObject()->getGlobalBounds();
            auto parentGlobalBounds = parent->getObject()->getGlobalBounds();

            sf::Vector2f targetAnchorPosition = targetGlobalBounds.getPosition() + sf::Vector2f{targetGlobalBounds.width, targetGlobalBounds.height-parentGlobalBounds.height};
            sf::Vector2f movePosition = targetAnchorPosition - parentGlobalBounds.getPosition();

            parent->getObject()->move(movePosition);
        }
        else if (this->_g_anchorTarget == FGE_SCENE_BAD_SID)
        {//On the render target
            auto* renderTarget = scene->getLinkedRenderTarget();
            if ( renderTarget != nullptr )
            {
                auto parentGlobalBounds = parent->getObject()->getGlobalBounds();

                sf::Vector2f targetAnchorPosition = sf::Vector2f{0.0f, renderTarget->getDefaultView().getSize().y-parentGlobalBounds.height};
                sf::Vector2f movePosition = targetAnchorPosition - parentGlobalBounds.getPosition();

                parent->getObject()->move(movePosition);
            }
        }
    }
        break;
    case ANCHOR_TOP_RIGHT:
    {
        auto target = scene->getObject(this->_g_anchorTarget);
        if ( fge::ObjectData::isValid(target) )
        {
            auto targetGlobalBounds = target->getObject()->getGlobalBounds();
            auto parentGlobalBounds = parent->getObject()->getGlobalBounds();

            sf::Vector2f targetAnchorPosition = targetGlobalBounds.getPosition() + sf::Vector2f{-parentGlobalBounds.width, 0.0f};
            sf::Vector2f movePosition = targetAnchorPosition - parentGlobalBounds.getPosition();

            parent->getObject()->move(movePosition);
        }
        else if (this->_g_anchorTarget == FGE_SCENE_BAD_SID)
        {//On the render target
            auto* renderTarget = scene->getLinkedRenderTarget();
            if ( renderTarget != nullptr )
            {
                auto parentGlobalBounds = parent->getObject()->getGlobalBounds();

                sf::Vector2f targetAnchorPosition = sf::Vector2f{renderTarget->getDefaultView().getSize().x-parentGlobalBounds.width, 0.0f};
                sf::Vector2f movePosition = targetAnchorPosition - parentGlobalBounds.getPosition();

                parent->getObject()->move(movePosition);
            }
        }
    }
        break;
    case ANCHOR_BOT_RIGHT:
    {
        auto target = scene->getObject(this->_g_anchorTarget);
        if ( fge::ObjectData::isValid(target) )
        {
            auto targetGlobalBounds = target->getObject()->getGlobalBounds();
            auto parentGlobalBounds = parent->getObject()->getGlobalBounds();

            sf::Vector2f targetAnchorPosition = targetGlobalBounds.getPosition() + sf::Vector2f{-parentGlobalBounds.width, targetGlobalBounds.height-parentGlobalBounds.height};
            sf::Vector2f movePosition = targetAnchorPosition - parentGlobalBounds.getPosition();

            parent->getObject()->move(movePosition);
        }
        else if (this->_g_anchorTarget == FGE_SCENE_BAD_SID)
        {//On the render target
            auto* renderTarget = scene->getLinkedRenderTarget();
            if ( renderTarget != nullptr )
            {
                auto parentGlobalBounds = parent->getObject()->getGlobalBounds();

                sf::Vector2f targetAnchorPosition = renderTarget->getDefaultView().getSize()-sf::Vector2f{parentGlobalBounds.width, parentGlobalBounds.height};
                sf::Vector2f movePosition = targetAnchorPosition - parentGlobalBounds.getPosition();

                parent->getObject()->move(movePosition);
            }
        }
    }
        break;
    }
}

}//end fge