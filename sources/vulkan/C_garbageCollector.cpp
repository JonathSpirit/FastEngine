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

#include "FastEngine/vulkan/C_garbageCollector.hpp"

namespace fge::vulkan
{

void GarbageCollector::setCurrentFrame(uint32_t frame)
{
    if (frame < FGE_MAX_FRAMES_IN_FLIGHT)
    {
        this->g_currentFrame = frame;
    }
    this->free();
}
uint32_t GarbageCollector::getCurrentFrame() const
{
    return this->g_currentFrame;
}
void GarbageCollector::push(Garbage garbage) const
{
    if (this->g_enabled)
    {
        this->g_containers[this->g_currentFrame].push_back(std::move(garbage));
    }
}
void GarbageCollector::free()
{
    this->g_containers[this->g_currentFrame].clear();
}
void GarbageCollector::freeAll()
{
    for (unsigned int i=0; i<FGE_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        this->g_containers[i].clear();
    }
}

void GarbageCollector::enable(bool stat)
{
    this->g_enabled = stat;
    if (!stat)
    {
        this->freeAll();
    }
}
bool GarbageCollector::isEnabled() const
{
    return this->g_enabled;
}

}//end fge::vulkan