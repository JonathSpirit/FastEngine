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

#include "FastEngine/vulkan/C_instance.hpp"
#include "FastEngine/fastengine_version.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <iostream>
#include <map>
#include <stdexcept>

namespace fge::vulkan
{

Instance::Instance() :
        g_instance(VK_NULL_HANDLE),
        g_window(nullptr)
{}
Instance::Instance(Instance&& r) noexcept :
        g_instance(r.g_instance),
        g_applicationName(std::move(r.g_applicationName)),
        g_window(r.g_window),
        g_physicalDevices(std::move(r.g_physicalDevices))
{
    r.g_instance = VK_NULL_HANDLE;
    r.g_window = nullptr;
}
Instance::~Instance()
{
    this->destroy();
}

void Instance::create(SDL_Window* window,
                      std::string applicationName,
                      uint16_t versionMajor,
                      uint16_t versionMinor,
                      uint16_t versionPatch)
{
    if (this->g_instance != VK_NULL_HANDLE)
    {
        throw std::runtime_error{"instance already created !"};
    }

    this->g_applicationName = std::move(applicationName);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = this->g_applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, versionMajor, versionMinor, versionPatch);
    appInfo.pEngineName = "FastEngine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, FGE_VERSION_MAJOR, FGE_VERSION_MINOR, FGE_VERSION_REVISION);
    appInfo.apiVersion = VK_API_VERSION_1_1;

#ifdef FGE_ENABLE_VALIDATION_LAYERS
    std::vector<const char*> validValidationLayers;
    validValidationLayers.reserve(ValidationLayers.size());

    for (const char* layerName: ValidationLayers)
    {
        if (!CheckValidationLayerSupport(layerName))
        {
            std::cout << "validation layer \"" << layerName << "\" requested, but not available (will be ignored) !\n";
        }
        else
        {
            validValidationLayers.push_back(layerName);
        }
    }
#endif

    uint32_t enabled_extension_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &enabled_extension_count, nullptr);

    std::vector<const char*> extensions{enabled_extension_count};

    SDL_Vulkan_GetInstanceExtensions(window, &enabled_extension_count, extensions.data());

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = enabled_extension_count;
    createInfo.ppEnabledExtensionNames = reinterpret_cast<const char* const*>(extensions.data());

#ifndef FGE_ENABLE_VALIDATION_LAYERS
    createInfo.enabledLayerCount = 0;
#else
    createInfo.enabledLayerCount = static_cast<uint32_t>(validValidationLayers.size());
    createInfo.ppEnabledLayerNames = validValidationLayers.data();
#endif

    const VkResult result = vkCreateInstance(&createInfo, nullptr, &this->g_instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error{"error while creating instance !"};
    }

    volkLoadInstance(this->g_instance);

    this->g_window = window;

    this->enumeratePhysicalDevices();
}
void Instance::destroy()
{
    if (this->g_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(this->g_instance, nullptr);
        this->g_instance = VK_NULL_HANDLE;
        this->g_window = nullptr;
        this->g_applicationName.clear();
    }
}

const std::string& Instance::getApplicationName() const
{
    return this->g_applicationName;
}

VkInstance Instance::getInstance() const
{
    return this->g_instance;
}
SDL_Window* Instance::getWindow() const
{
    return this->g_window;
}

glm::vec<2, int> Instance::getWindowSize() const
{
    glm::vec<2, int> size{0};
    SDL_GetWindowSize(this->g_window, &size.x, &size.y);
    return size;
}

const std::vector<PhysicalDevice>& Instance::getPhysicalDevices() const
{
    return this->g_physicalDevices;
}
PhysicalDevice Instance::pickPhysicalDevice(VkSurfaceKHR surface)
{
    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, PhysicalDevice> candidates;

    for (const auto& device: this->g_physicalDevices)
    {
        const auto score = device.rateDeviceSuitability(surface);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0)
    {
        return candidates.rbegin()->second;
    }
    return PhysicalDevice(VK_NULL_HANDLE);
}

void Instance::enumeratePhysicalDevices()
{
    this->g_physicalDevices.clear();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->g_instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support !");
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(this->g_instance, &deviceCount, physicalDevices.data());

    this->g_physicalDevices.resize(deviceCount);
    for (std::size_t i = 0; i < deviceCount; ++i)
    {
        this->g_physicalDevices[i] = PhysicalDevice(physicalDevices[i]);
    }
}

} // namespace fge::vulkan