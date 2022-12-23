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

#include "FastEngine/vulkan/C_descriptorSetLayout.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include <stdexcept>
#include <vector>

namespace fge::vulkan
{

DescriptorSetLayout::DescriptorSetLayout() :
        g_descriptorSetLayout(VK_NULL_HANDLE),
        g_layoutSize(0),
        g_logicalDevice(nullptr)
{}
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& r) noexcept :
        g_descriptorSetLayout(r.g_descriptorSetLayout),
        g_layoutSize(r.g_layoutSize),
        g_logicalDevice(r.g_logicalDevice)
{
    r.g_descriptorSetLayout = VK_NULL_HANDLE;
    r.g_layoutSize = 0;
    r.g_logicalDevice = nullptr;
}
DescriptorSetLayout::~DescriptorSetLayout()
{
    this->destroy();
}

void DescriptorSetLayout::create(const LogicalDevice& logicalDevice, std::initializer_list<Layout> layouts)
{
    this->destroy();

    if (layouts.size() == 0)
    {
        return;
    }

    this->g_layoutSize = layouts.size();
    this->g_logicalDevice = &logicalDevice;

    std::vector<VkDescriptorSetLayoutBinding> bindings(layouts.size());
    std::size_t index = 0;

    for (const auto& layout : layouts)
    {
        bindings[index].binding = layout._binding;
        bindings[index].descriptorType = layout._type;
        bindings[index].descriptorCount = 1;

        bindings[index].stageFlags = layout._stage;
        bindings[index].pImmutableSamplers = nullptr; // Optional

        ++index;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice.getDevice(), &layoutInfo, nullptr, &this->g_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}
void DescriptorSetLayout::destroy()
{
    if (this->g_descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(this->g_logicalDevice->getDevice(), this->g_descriptorSetLayout, nullptr);
        this->g_descriptorSetLayout = VK_NULL_HANDLE;
        this->g_layoutSize = 0;
        this->g_logicalDevice = nullptr;
    }
}

VkDescriptorSetLayout DescriptorSetLayout::getLayout() const
{
    return this->g_descriptorSetLayout;
}
std::size_t DescriptorSetLayout::getLayoutSize() const
{
    return this->g_layoutSize;
}
const LogicalDevice* DescriptorSetLayout::getLogicalDevice() const
{
    return this->g_logicalDevice;
}

}//end fge::vulkan