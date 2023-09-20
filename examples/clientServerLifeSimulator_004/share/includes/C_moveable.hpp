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

#ifndef _EXFGE_C_MOVEABLE_HPP_INCLUDED
#define _EXFGE_C_MOVEABLE_HPP_INCLUDED

#include "FastEngine/object/C_object.hpp"

namespace ls
{

class Moveable
{
public:
    Moveable() = default;
    ~Moveable() = default;

    void setTargetPos(fge::Vector2f const& pos);
    bool updateMoveable(fge::Transformable& transformable, std::chrono::microseconds const& deltaTime);

protected:
    fge::Vector2f _g_targetPos;
    bool _g_finish{true};
};

} // namespace ls

#endif // _EXFGE_C_MOVEABLE_HPP_INCLUDED
