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

#ifndef _FGE_C_OBJECTANCHOR_HPP_INCLUDED
#define _FGE_C_OBJECTANCHOR_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_vector.hpp"
#include <cstdint>
#include <limits>
#include <memory>

#define FGE_SCENE_BAD_SID std::numeric_limits<fge::ObjectSid>::max()

namespace fge
{

using ObjectSid = uint32_t;

class ObjectData;
using ObjectDataShared = std::shared_ptr<fge::ObjectData>;
using ObjectDataWeak = std::weak_ptr<fge::ObjectData>;

class Object;

class FGE_API Anchor
{
public:
    enum class Types : uint8_t
    {
        ANCHOR_NONE,
        ANCHOR_UPLEFT_CORNER,
        ANCHOR_UPRIGHT_CORNER,
        ANCHOR_DOWNLEFT_CORNER,
        ANCHOR_DOWNRIGHT_CORNER
    };

    enum class Shifts : uint8_t
    {
        SHIFT_NONE,
        SHIFT_POSITIVE_BOUNDS,
        SHIFT_NEGATIVE_BOUNDS
    };

    explicit Anchor(fge::Object* owner);
    Anchor(fge::Object* owner, Anchor const& anchor);
    Anchor(Anchor const& r) = delete;
    Anchor(Anchor&& r) noexcept = delete;
    virtual ~Anchor();

    Anchor& operator=(Anchor const& r);
    Anchor& operator=(Anchor&& r) noexcept = delete;

    void updateAnchor(fge::Vector2f const& customTargetSize = {0.0f, 0.0f});

    void setAnchor(Types type, fge::Vector2<Shifts> const& shift, fge::ObjectSid target = FGE_SCENE_BAD_SID);

    void setAnchorType(Types type);
    void setAnchorShift(fge::Vector2<Shifts> const& shift);
    void setAnchorTarget(fge::ObjectSid target);

    [[nodiscard]] Types getAnchorType() const;
    [[nodiscard]] fge::Vector2<Shifts> const& getAnchorShift() const;
    [[nodiscard]] fge::ObjectSid getAnchorTarget() const;
    [[nodiscard]] fge::Object* getAnchorOwner() const;

    void setAnchorSuccessor(fge::ObjectDataWeak successor);
    [[nodiscard]] fge::ObjectDataWeak getAnchorSuccessor() const;

    void needAnchorUpdate(bool flag);
    [[nodiscard]] bool isNeedingAnchorUpdate() const;

private:
    Types g_anchorType{Types::ANCHOR_NONE};
    fge::Vector2<Shifts> g_anchorShift{Shifts::SHIFT_NONE, Shifts::SHIFT_NONE};
    fge::ObjectSid g_anchorTarget{FGE_SCENE_BAD_SID};
    bool g_anchorNeedUpdate{true};
    fge::ObjectDataWeak g_anchorSuccessor;
    fge::Object* g_anchorOwner;
};

} // namespace fge

#endif // _FGE_C_OBJECTANCHOR_HPP_INCLUDED
