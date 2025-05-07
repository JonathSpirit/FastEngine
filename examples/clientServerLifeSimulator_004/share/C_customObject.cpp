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

#include "C_customObject.hpp"
#include "FastEngine/C_random.hpp"
#include "FastEngine/extra/extra_function.hpp"
#include "definition.hpp"

namespace ls
{

bool CustomObject::worldTick()
{
    return false;
}

fge::Vector2f ClampToMapLimit(fge::Vector2f const& position)
{
    return {std::clamp<float>(position.x, LIFESIM_MAP_SIZE_MINX, LIFESIM_MAP_SIZE_MAXX),
            std::clamp<float>(position.y, LIFESIM_MAP_SIZE_MINY, LIFESIM_MAP_SIZE_MAXY)};
}
fge::Vector2f GetRandomPositionFromCenter(fge::Vector2f const& center, float maxDistance)
{
    fge::Vector2f forwardVector = fge::GetForwardVector(fge::_random.range<float>(0.0f, 360.0f));
    forwardVector *= fge::_random.range<float>(0.0f, maxDistance);

    return ls::ClampToMapLimit(forwardVector + center);
}
fge::Vector2f GetRandomPosition()
{
    fge::Vector2f position(fge::_random.rangeVec2(LIFESIM_MAP_SIZE_MINX, LIFESIM_MAP_SIZE_MAXX, LIFESIM_MAP_SIZE_MINY,
                                                  LIFESIM_MAP_SIZE_MAXY));

    return position;
}

} // namespace ls
