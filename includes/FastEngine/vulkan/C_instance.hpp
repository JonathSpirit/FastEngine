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

#ifndef _FGE_VULKAN_C_INSTANCE_HPP_INCLUDED
#define _FGE_VULKAN_C_INSTANCE_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"
#include "C_physicalDevice.hpp"
#include "SDL_vulkan.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace fge::vulkan
{

class FGE_API Instance
{
public:
    Instance();
    Instance(const Instance& r) = delete;
    Instance(Instance&& r) noexcept;
    ~Instance();

    Instance& operator=(const Instance& r) = delete;
    Instance& operator=(Instance&& r) noexcept = delete;

    void create(SDL_Window* window,
                std::string applicationName,
                uint16_t versionMajor = 1,
                uint16_t versionMinor = 0,
                uint16_t versionPatch = 0);
    void destroy();

    [[nodiscard]] const std::string& getApplicationName() const;

    [[nodiscard]] VkInstance getInstance() const;
    [[nodiscard]] SDL_Window* getWindow() const;

    [[nodiscard]] glm::vec<2, int> getWindowSize() const;

    [[nodiscard]] const std::vector<PhysicalDevice>& getPhysicalDevices() const;
    [[nodiscard]] PhysicalDevice pickPhysicalDevice(VkSurfaceKHR surface);

    static bool checkValidationLayerSupport();

private:
    void enumeratePhysicalDevices();

    VkInstance g_instance;
    std::string g_applicationName;

    SDL_Window* g_window;

    std::vector<PhysicalDevice> g_physicalDevices;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_INSTANCE_HPP_INCLUDED
