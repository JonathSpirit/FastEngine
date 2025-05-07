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

#include "C_moveable.hpp"
#include "FastEngine/extra/extra_function.hpp"

namespace ls
{

void Moveable::setTargetPos(fge::Vector2f const& pos)
{
    this->_g_targetPos = pos;
    this->_g_finish = false;
}
bool Moveable::updateMoveable(fge::Transformable& transformable, fge::DeltaTime const& deltaTime)
{
    if (this->_g_finish)
    {
        return true;
    }
    float deltaTimeFloat = fge::DurationToSecondFloat(deltaTime);
    transformable.setPosition(fge::ReachVector(transformable.getPosition(), this->_g_targetPos, 60.0f, deltaTimeFloat));
    if (transformable.getPosition() == this->_g_targetPos)
    {
        this->_g_finish = true;
        return true;
    }
    return false;
}

} // namespace ls
