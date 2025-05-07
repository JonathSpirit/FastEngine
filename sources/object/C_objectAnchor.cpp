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

#include "FastEngine/object/C_objectAnchor.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_scene.hpp"

namespace fge
{

Anchor::Anchor(fge::Object* owner) :
        g_anchorOwner(owner)
{}
Anchor::Anchor(fge::Object* owner, Anchor const& anchor) :
        g_anchorType(anchor.g_anchorType),
        g_anchorShift(anchor.g_anchorShift),
        g_anchorTarget(anchor.g_anchorTarget),
        g_anchorOwner(owner)
{}
Anchor::~Anchor()
{
    if (auto successor = this->g_anchorSuccessor.lock())
    {
        successor->getObject()->setAnchor(this->g_anchorType, this->g_anchorShift, this->g_anchorTarget);
    }
}

Anchor& Anchor::operator=(Anchor const& r)
{
    this->g_anchorType = r.g_anchorType;
    this->g_anchorShift = r.g_anchorShift;
    this->g_anchorTarget = r.g_anchorTarget;
    this->g_anchorNeedUpdate = true;
    this->g_anchorSuccessor.reset();
    return *this;
}

void Anchor::updateAnchor(fge::Vector2f const& customTargetSize)
{
    this->g_anchorNeedUpdate = false;

    if (this->g_anchorType == Types::ANCHOR_NONE)
    {
        return;
    }
    auto parent = this->g_anchorOwner->_myObjectData.lock();
    if (!fge::ObjectData::isValid(parent))
    {
        return;
    }
    auto* scene = parent->getScene();
    if (scene == nullptr)
    {
        return;
    }

    fge::RectFloat targetGlobalBounds;
    fge::RectFloat parentGlobalBounds = parent->getObject()->getGlobalBounds();

    auto target = scene->getObject(this->g_anchorTarget);
    if (fge::ObjectData::isValid(target))
    { //On a target
        targetGlobalBounds = target->getObject()->getGlobalBounds();
    }
    else if (this->g_anchorTarget == FGE_SCENE_BAD_SID)
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
    switch (this->g_anchorType)
    {
    case Types::ANCHOR_UPLEFT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition();
        break;
    case Types::ANCHOR_UPRIGHT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition() + fge::Vector2f{targetGlobalBounds._width, 0.0f};
        break;
    case Types::ANCHOR_DOWNLEFT_CORNER:
        anchorPosition = targetGlobalBounds.getPosition() + fge::Vector2f{0.0f, targetGlobalBounds._height};
        break;
    case Types::ANCHOR_DOWNRIGHT_CORNER:
        anchorPosition =
                targetGlobalBounds.getPosition() + fge::Vector2f{targetGlobalBounds._width, targetGlobalBounds._height};
        break;
    default:
        anchorPosition = {0.0f, 0.0f};
        break;
    }

    switch (this->g_anchorShift.x)
    {
    case Shifts::SHIFT_NONE:
        break;
    case Shifts::SHIFT_POSITIVE_BOUNDS:
        anchorPosition.x += parentGlobalBounds._width;
        break;
    case Shifts::SHIFT_NEGATIVE_BOUNDS:
        anchorPosition.x -= parentGlobalBounds._width;
        break;
    }
    switch (this->g_anchorShift.y)
    {
    case Shifts::SHIFT_NONE:
        break;
    case Shifts::SHIFT_POSITIVE_BOUNDS:
        anchorPosition.y += parentGlobalBounds._height;
        break;
    case Shifts::SHIFT_NEGATIVE_BOUNDS:
        anchorPosition.y -= parentGlobalBounds._height;
        break;
    }

    fge::Vector2f movePosition = anchorPosition - parentGlobalBounds.getPosition();
    parent->getObject()->move(movePosition);
}

void Anchor::setAnchor(Types type, fge::Vector2<Shifts> const& shift, fge::ObjectSid target)
{
    this->g_anchorType = type;
    this->g_anchorShift = shift;
    this->g_anchorTarget = target;
    this->g_anchorNeedUpdate = true;
}
void Anchor::setAnchorType(Types type)
{
    this->g_anchorType = type;
    this->g_anchorNeedUpdate = true;
}
void Anchor::setAnchorShift(fge::Vector2<Shifts> const& shift)
{
    this->g_anchorShift = shift;
    this->g_anchorNeedUpdate = true;
}
void Anchor::setAnchorTarget(fge::ObjectSid target)
{
    this->g_anchorTarget = target;
    this->g_anchorNeedUpdate = true;
}

fge::Anchor::Types Anchor::getAnchorType() const
{
    return this->g_anchorType;
}
fge::Vector2<Anchor::Shifts> const& Anchor::getAnchorShift() const
{
    return this->g_anchorShift;
}
fge::ObjectSid Anchor::getAnchorTarget() const
{
    return this->g_anchorTarget;
}
fge::Object* Anchor::getAnchorOwner() const
{
    return this->g_anchorOwner;
}

void Anchor::setAnchorSuccessor(fge::ObjectDataWeak successor)
{
    this->g_anchorSuccessor = std::move(successor);
}
fge::ObjectDataWeak Anchor::getAnchorSuccessor() const
{
    return this->g_anchorSuccessor;
}

void Anchor::needAnchorUpdate(bool flag)
{
    this->g_anchorNeedUpdate = flag;
}
bool Anchor::isNeedingAnchorUpdate() const
{
    return this->g_anchorNeedUpdate;
}

} // namespace fge
