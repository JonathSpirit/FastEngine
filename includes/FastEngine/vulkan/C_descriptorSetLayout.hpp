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

#ifndef _FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"
#include "SDL_vulkan.h"
#include <initializer_list>

namespace fge::vulkan
{

class LogicalDevice;

class FGE_API DescriptorSetLayout
{
public:
    struct Layout
    {
        VkDescriptorType _type;
        uint32_t _binding;
        VkShaderStageFlags _stage;
    };

    DescriptorSetLayout();
    DescriptorSetLayout(const DescriptorSetLayout& r) = delete;
    DescriptorSetLayout(DescriptorSetLayout&& r) noexcept;
    ~DescriptorSetLayout();

    DescriptorSetLayout& operator=(const DescriptorSetLayout& r) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& r) noexcept = delete;

    void create(const LogicalDevice& logicalDevice, std::initializer_list<Layout> layouts);
    void destroy();

    [[nodiscard]] VkDescriptorSetLayout getLayout() const;
    [[nodiscard]] std::size_t getLayoutSize() const;
    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

private:
    VkDescriptorSetLayout g_descriptorSetLayout;
    std::size_t g_layoutSize;

    const LogicalDevice* g_logicalDevice;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED
