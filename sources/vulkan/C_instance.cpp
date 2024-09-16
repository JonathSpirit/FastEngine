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

#include "FastEngine/vulkan/C_instance.hpp"
#include "FastEngine/C_alloca.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/fge_version.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <iostream>
#include <map>

namespace fge::vulkan
{

Instance::Instance() :
        g_instance(VK_NULL_HANDLE)
{}
Instance::Instance(std::string applicationName, uint16_t versionMajor, uint16_t versionMinor, uint16_t versionPatch) :
        g_instance(VK_NULL_HANDLE)
{
    this->create(std::move(applicationName), versionMajor, versionMinor, versionPatch);
}
Instance::Instance(Instance&& r) noexcept :
        g_instance(r.g_instance),
        g_applicationName(std::move(r.g_applicationName)),
        g_physicalDevices(std::move(r.g_physicalDevices))
{
    r.g_instance = VK_NULL_HANDLE;
}
Instance::~Instance()
{
    this->destroy();
}

void Instance::create(std::string applicationName, uint16_t versionMajor, uint16_t versionMinor, uint16_t versionPatch)
{
    if (this->g_instance != VK_NULL_HANDLE)
    {
        throw fge::Exception{"instance already created !"};
    }

    this->g_applicationName = std::move(applicationName);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = this->g_applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, versionMajor, versionMinor, versionPatch);
    appInfo.pEngineName = "FastEngine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, FGE_VERSION_MAJOR, FGE_VERSION_MINOR, FGE_VERSION_REVISION);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    std::vector<char const*> validInstanceLayers;
    validInstanceLayers.reserve(InstanceLayers.size());

    for (char const* layerName: InstanceLayers)
    {
        if (!CheckInstanceLayerSupport(layerName))
        {
            std::cout << "validation layer \"" << layerName << "\" requested, but not available (will be ignored) !\n";
        }
        else
        {
            validInstanceLayers.push_back(layerName);
        }
    }

    uint32_t enabled_extension_count = 0;
    if (SDL_Vulkan_GetInstanceExtensions(nullptr, &enabled_extension_count, nullptr) == SDL_FALSE)
    {
        throw fge::Exception{"instance: not all required extension was available !"};
    }

    std::vector<char const*> extensions{enabled_extension_count};

    SDL_Vulkan_GetInstanceExtensions(nullptr, &enabled_extension_count, extensions.data());

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = enabled_extension_count;
    createInfo.ppEnabledExtensionNames = reinterpret_cast<char const* const*>(extensions.data());

    createInfo.enabledLayerCount = static_cast<uint32_t>(validInstanceLayers.size());
    createInfo.ppEnabledLayerNames = validInstanceLayers.empty() ? nullptr : validInstanceLayers.data();

    VkResult const result = vkCreateInstance(&createInfo, nullptr, &this->g_instance);

    if (result != VK_SUCCESS)
    {
        throw fge::Exception{"error while creating instance !"};
    }

    volkLoadInstance(this->g_instance);

    this->enumeratePhysicalDevices();
}
void Instance::destroy()
{
    if (this->g_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(this->g_instance, nullptr);
        this->g_instance = VK_NULL_HANDLE;
        this->g_applicationName.clear();
    }
}

std::string const& Instance::getApplicationName() const
{
    return this->g_applicationName;
}

VkInstance Instance::get() const
{
    return this->g_instance;
}

std::vector<PhysicalDevice> const& Instance::getPhysicalDevices() const
{
    return this->g_physicalDevices;
}
std::optional<PhysicalDevice> Instance::pickPhysicalDevice(VkSurfaceKHR surface) const
{
    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<unsigned int, PhysicalDevice const*> candidates;

    for (auto const& device: this->g_physicalDevices)
    {
        auto const score = device.rateDeviceSuitability(surface);
        candidates.insert(std::make_pair(score, &device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0)
    {
        return *candidates.rbegin()->second;
    }
    return std::nullopt;
}

void Instance::enumeratePhysicalDevices()
{
    this->g_physicalDevices.clear();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->g_instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw fge::Exception("failed to find GPUs with Vulkan support !");
    }

    auto* physicalDevices = FGE_ALLOCA_T(VkPhysicalDevice, deviceCount);
    vkEnumeratePhysicalDevices(this->g_instance, &deviceCount, physicalDevices);

    this->g_physicalDevices.reserve(deviceCount);
    for (uint32_t i = 0; i < deviceCount; ++i)
    {
        this->g_physicalDevices.emplace_back(physicalDevices[i]);
    }
}

} // namespace fge::vulkan