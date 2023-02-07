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
#include "FastEngine/vulkan/C_context.hpp"
#include <stdexcept>

namespace fge::vulkan
{

DescriptorSetLayout::DescriptorSetLayout() :
        g_descriptorSetLayout(VK_NULL_HANDLE),
        g_context(nullptr)
{}
DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayout& r) :
        g_descriptorSetLayout(r.g_descriptorSetLayout),
        g_layouts(r.g_layouts),
        g_context(r.g_context)
{
    if (this->g_layouts.empty())
    {
        return;
    }

    this->createDescriptorSetLayout();
}
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& r) noexcept :
        g_descriptorSetLayout(r.g_descriptorSetLayout),
        g_layouts(std::move(r.g_layouts)),
        g_context(r.g_context)
{
    r.g_descriptorSetLayout = VK_NULL_HANDLE;
    r.g_context = nullptr;
}
DescriptorSetLayout::~DescriptorSetLayout()
{
    this->destroy();
}

DescriptorSetLayout& DescriptorSetLayout::operator=(const DescriptorSetLayout& r)
{
    this->destroy();

    if (r.g_layouts.empty())
    {
        return *this;
    }

    this->g_layouts = r.g_layouts;
    this->g_context = r.g_context;

    this->createDescriptorSetLayout();

    return *this;
}
DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& r) noexcept
{
    this->destroy();

    this->g_layouts = std::move(r.g_layouts);
    this->g_context = r.g_context;
    this->g_descriptorSetLayout = r.g_descriptorSetLayout;

    r.g_descriptorSetLayout = VK_NULL_HANDLE;
    r.g_context = nullptr;

    return *this;
}

void DescriptorSetLayout::create(const Context& context, std::initializer_list<VkDescriptorSetLayoutBinding> layouts)
{
    this->destroy();

    if (layouts.size() == 0)
    {
        return;
    }

    this->g_layouts = layouts;
    this->g_context = &context;

    this->createDescriptorSetLayout();
}
void DescriptorSetLayout::destroy()
{
    if (this->g_descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(this->g_context->getLogicalDevice().getDevice(), this->g_descriptorSetLayout, nullptr);
        this->g_descriptorSetLayout = VK_NULL_HANDLE;
        this->g_layouts.clear();
        this->g_context = nullptr;
    }
}

VkDescriptorSetLayout DescriptorSetLayout::getLayout() const
{
    return this->g_descriptorSetLayout;
}
const std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayout::getLayouts() const
{
    return this->g_layouts;
}
std::size_t DescriptorSetLayout::getLayoutSize() const
{
    return this->g_layouts.size();
}
const Context* DescriptorSetLayout::getContext() const
{
    return this->g_context;
}

void DescriptorSetLayout::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(this->g_layouts.size());
    layoutInfo.pBindings = this->g_layouts.data();

    if (vkCreateDescriptorSetLayout(this->g_context->getLogicalDevice().getDevice(), &layoutInfo, nullptr, &this->g_descriptorSetLayout) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

} // namespace fge::vulkan