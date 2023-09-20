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

#ifndef _FGE_VULKAN_C_VIEWPORT_HPP_INCLUDED
#define _FGE_VULKAN_C_VIEWPORT_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"

namespace fge::vulkan
{

class FGE_API Viewport
{
public:
    Viewport();
    Viewport(float x, float y, float width, float height);
    ~Viewport() = default;

    [[nodiscard]] bool operator==(Viewport const& r) const
    {
        return this->g_viewport.x == r.g_viewport.x && this->g_viewport.y == r.g_viewport.y &&
               this->g_viewport.width == r.g_viewport.width && this->g_viewport.height == r.g_viewport.height;
    }
    [[nodiscard]] bool operator!=(Viewport const& r) const { return !this->operator==(r); }

    void setPosition(float x, float y);
    void setSize(float width, float height);

    [[nodiscard]] float getPositionX() const;
    [[nodiscard]] float getPositionY() const;

    [[nodiscard]] float getWidth() const;
    [[nodiscard]] float getHeight() const;

    [[nodiscard]] VkViewport const& getViewport() const;

    void cmdSetViewport(VkCommandBuffer commandBuffer) const;

private:
    VkViewport g_viewport;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_VIEWPORT_HPP_INCLUDED
