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

#include "FastEngine/vulkan/C_viewport.hpp"

namespace fge::vulkan
{

Viewport::Viewport() :
        g_viewport{.x=0.0f, .y=0.0f, .width=0.0f, .height=0.0f, .minDepth=0.0f, .maxDepth=1.0f}
{}
Viewport::Viewport(float x, float y, float width, float height) :
        g_viewport{.x=x, .y=y, .width=width, .height=height, .minDepth=0.0f, .maxDepth=1.0f}
{}

void Viewport::setPosition(float x, float y)
{
    this->g_viewport.x = x;
    this->g_viewport.y = y;
}
void Viewport::setSize(float width, float height)
{
    this->g_viewport.width = width;
    this->g_viewport.height = height;
}

float Viewport::getPositionX() const
{
    return this->g_viewport.x;
}
float Viewport::getPositionY() const
{
    return this->g_viewport.y;
}

float Viewport::getWidth() const
{
    return this->g_viewport.width;
}
float Viewport::getHeight() const
{
    return this->g_viewport.height;
}

const VkViewport& Viewport::getViewport() const
{
    return this->g_viewport;
}

}//end fge::vulkan