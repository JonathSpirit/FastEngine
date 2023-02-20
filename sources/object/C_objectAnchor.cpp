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

#include "FastEngine/object/C_objectAnchor.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

Anchor::Anchor(fge::Object* parent) :
        _g_objectParent(parent)
{}
Anchor::Anchor(fge::Object* parent, const Anchor& anchor) :
        _g_anchorType(anchor._g_anchorType),
        _g_anchorShift(anchor._g_anchorShift),
        _g_anchorTarget(anchor._g_anchorTarget),
        _g_anchorNeedUpdate(true),
        _g_objectParent(parent)
{}
Anchor::~Anchor()
{
    if (auto successor = this->_g_anchorSuccessor.lock())
    {
        successor->getObject()->setAnchor(this->_g_anchorType, this->_g_anchorShift, this->_g_anchorTarget);
    }
}

Anchor& Anchor::operator=(const Anchor& r)
{
    this->_g_anchorType = r._g_anchorType;
    this->_g_anchorShift = r._g_anchorShift;
    this->_g_anchorTarget = r._g_anchorTarget;
    this->_g_anchorNeedUpdate = true;
    this->_g_anchorSuccessor.reset();
    return *this;
}

void Anchor::updateAnchor(const fge::Vector2f& customTargetSize)
{
    this->_g_anchorNeedUpdate = false;

    if (this->_g_anchorType == fge::Anchor::Types::ANCHOR_NONE)
    {
        return;
    }
    auto parent = this->_g_objectParent->_myObjectData.lock();
    if (!fge::ObjectData::isValid(parent))
    {
        return;
    }
    auto* scene = parent->getLinkedScene();
    if (scene == nullptr)
    {
        return;
    }

    fge::Vector2f movePosition;

    fge::RectFloat targetGlobalBounds;
    fge::RectFloat parentGlobalBounds = parent->getObject()->getGlobalBounds();

    auto target = scene->getObject(this->_g_anchorTarget);
    if (fge::ObjectData::isValid(target))
    { //On a target
        targetGlobalBounds = target->getObject()->getGlobalBounds();
    }
    else if (this->_g_anchorTarget == FGE_SCENE_BAD_SID)
    { //On the render target
        if (customTargetSize.x != 0.0f && customTargetSize.y != 0.0f)
        {
            targetGlobalBounds = {{0.0f, 0.0f}, customTargetSize};
        }
        else
        {
            auto* renderTarget = scene->getLinkedRenderTarget();
            if (renderTarget != nullptr)
            {
                targetGlobalBounds = {{0.0f, 0.0f}, renderTarget->getDefaultView().getSize()};
            }
        }
    }

    fge::Vector2f anchorPosition;
    switch (this->_g_anchorType)
    {
    case fge::Anchor::Types::ANCHOR_UPLEFT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition();
        break;
    case fge::Anchor::Types::ANCHOR_UPRIGHT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition() + fge::Vector2f{targetGlobalBounds._width, 0.0f};
        break;
    case fge::Anchor::Types::ANCHOR_DOWNLEFT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition() + fge::Vector2f{0.0f, targetGlobalBounds._height};
        break;
    case fge::Anchor::Types::ANCHOR_DOWNRIGHT_CORNER:
        anchorPosition =
                targetGlobalBounds.getPosition() + fge::Vector2f{targetGlobalBounds._width, targetGlobalBounds._height};
        break;
    default:
        anchorPosition = {0.0f, 0.0f};
        break;
    }

    switch (this->_g_anchorShift.x)
    {
    case fge::Anchor::Shifts::SHIFT_NONE:
        break;
    case fge::Anchor::Shifts::SHIFT_POSITIVE_BOUNDS:
        anchorPosition.x += parentGlobalBounds._width;
        break;
    case fge::Anchor::Shifts::SHIFT_NEGATIVE_BOUNDS:
        anchorPosition.x -= parentGlobalBounds._width;
        break;
    }
    switch (this->_g_anchorShift.y)
    {
    case fge::Anchor::Shifts::SHIFT_NONE:
        break;
    case fge::Anchor::Shifts::SHIFT_POSITIVE_BOUNDS:
        anchorPosition.y += parentGlobalBounds._height;
        break;
    case fge::Anchor::Shifts::SHIFT_NEGATIVE_BOUNDS:
        anchorPosition.y -= parentGlobalBounds._height;
        break;
    }

    movePosition = anchorPosition - parentGlobalBounds.getPosition();
    parent->getObject()->move(movePosition);
}

void Anchor::setAnchor(fge::Anchor::Types type, const fge::Vector2<fge::Anchor::Shifts>& shift, fge::ObjectSid target)
{
    this->_g_anchorType = type;
    this->_g_anchorShift = shift;
    this->_g_anchorTarget = target;
    this->_g_anchorNeedUpdate = true;
}

fge::Anchor::Types Anchor::getAnchorType() const
{
    return this->_g_anchorType;
}
fge::ObjectSid Anchor::getAnchorTarget() const
{
    return this->_g_anchorTarget;
}

void Anchor::setAnchorTarget(fge::ObjectSid target)
{
    this->_g_anchorTarget = target;
    this->_g_anchorNeedUpdate = true;
}
void Anchor::setAnchorSuccessor(fge::ObjectDataWeak successor)
{
    this->_g_anchorSuccessor = std::move(successor);
}
fge::ObjectDataWeak Anchor::getAnchorSuccessor() const
{
    return this->_g_anchorSuccessor;
}

void Anchor::needAnchorUpdate(bool flag)
{
    this->_g_anchorNeedUpdate = flag;
}
bool Anchor::isNeedingAnchorUpdate() const
{
    return this->_g_anchorNeedUpdate;
}

} // namespace fge
