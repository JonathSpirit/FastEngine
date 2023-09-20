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

#ifndef _FGE_VULKAN_C_INSTANCE_HPP_INCLUDED
#define _FGE_VULKAN_C_INSTANCE_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"
#include "C_physicalDevice.hpp"
#include "FastEngine/C_vector.hpp"
#include "SDL_vulkan.h"
#include <string>
#include <vector>

namespace fge::vulkan
{

/**
 * \class Instance
 * \ingroup vulkan
 * \brief Vulkan instance abstraction
 *
 * This class is used to create a Vulkan instance and get the physical devices.
 */
class FGE_API Instance
{
public:
    Instance();
    Instance(Instance const& r) = delete;
    Instance(Instance&& r) noexcept;
    ~Instance();

    Instance& operator=(Instance const& r) = delete;
    Instance& operator=(Instance&& r) noexcept = delete;

    /**
     * \brief Create the instance
     *
     * \param window The SDL window
     * \param applicationName The application name
     * \param versionMajor The application version major
     * \param versionMinor The application version minor
     * \param versionPatch The application version patch
     */
    void create(SDL_Window* window,
                std::string applicationName,
                uint16_t versionMajor = 1,
                uint16_t versionMinor = 0,
                uint16_t versionPatch = 0);
    void destroy();

    [[nodiscard]] std::string const& getApplicationName() const;

    [[nodiscard]] VkInstance getInstance() const;
    [[nodiscard]] SDL_Window* getWindow() const;

    /**
     * \brief Helper to get the SDL window size
     *
     * \return The window size
     */
    [[nodiscard]] glm::vec<2, int> getWindowSize() const;

    /**
     * \brief Get a list of physical devices
     *
     * You have to create the instance before calling this function.
     *
     * \return The list of physical devices
     */
    [[nodiscard]] std::vector<PhysicalDevice> const& getPhysicalDevices() const;
    /**
     * \brief Pick a physical device
     *
     * Pick a physical device that support the surface based on a score.
     * It will prioritize discrete GPU over integrated GPU.
     *
     * \param surface The surface
     * \return The physical device or VK_NULL_HANDLE if no physical device is found
     */
    [[nodiscard]] PhysicalDevice pickPhysicalDevice(VkSurfaceKHR surface);

private:
    void enumeratePhysicalDevices();

    VkInstance g_instance;
    std::string g_applicationName;

    SDL_Window* g_window;

    std::vector<PhysicalDevice> g_physicalDevices;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_INSTANCE_HPP_INCLUDED
