/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include <cstring>

extern "C" {
#include "volk.c" //Including the volk implementation
}

namespace fge::vulkan
{

#ifdef FGE_ENABLE_VALIDATION_LAYERS
std::vector<char const*> InstanceLayers = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"};
#else
std::vector<char const*> InstanceLayers = {};
#endif

std::vector<char const*> DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                                             VK_EXT_ROBUSTNESS_2_EXTENSION_NAME};

std::vector<char const*> InstanceExtensions = {};

namespace
{

Context* gActiveContext{nullptr};

} // namespace

Context& GetActiveContext()
{
#if defined(FGE_DEF_DEBUG) && !defined(FGE_DEF_SERVER)
    if (gActiveContext == nullptr)
    {
        throw fge::Exception("No active context !");
    }
#endif

    return *gActiveContext;
}
void SetActiveContext(Context& context)
{
    gActiveContext = &context;
}

bool CheckInstanceLayerSupport(char const* layerName)
{
    static std::vector<VkLayerProperties> availableLayers;

    if (availableLayers.empty())
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        availableLayers.resize(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    }

    for (auto const& layerProperties: availableLayers)
    {
        if (std::strcmp(layerName, layerProperties.layerName) == 0)
        {
            return true;
        }
    }
    return false;
}

} // namespace fge::vulkan