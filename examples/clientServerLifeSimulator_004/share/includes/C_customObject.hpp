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

#ifndef _EXFGE_CUSTOMOBJECT_HPP_INCLUDED
#define _EXFGE_CUSTOMOBJECT_HPP_INCLUDED

#include "C_moveable.hpp"
#include "FastEngine/object/C_object.hpp"

namespace ls
{

class CustomObject : public fge::Object, public ls::Moveable
{
public:
    CustomObject() = default;
    ~CustomObject() override = default;

    virtual bool worldTick();
};

fge::Vector2f ClampToMapLimit(fge::Vector2f const& position);
fge::Vector2f GetRandomPositionFromCenter(fge::Vector2f const& center, float maxDistance);
fge::Vector2f GetRandomPosition();

} // namespace ls

#endif // _EXFGE_CUSTOMOBJECT_HPP_INCLUDED
