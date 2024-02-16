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

#include "FastEngine/vulkan/C_descriptorSetLayout.hpp"
#include "FastEngine/C_alloca.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_context.hpp"

namespace fge::vulkan
{

DescriptorSetLayout::DescriptorSetLayout(Context const& context) :
        ContextAware(context),
        g_descriptorSetLayout(VK_NULL_HANDLE)
{}
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout const& r) :
        ContextAware(r),
        g_descriptorSetLayout(VK_NULL_HANDLE)
{
    this->create(r.g_bindings.data(), static_cast<uint32_t>(r.g_bindings.size()));
}
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_descriptorSetLayout(r.g_descriptorSetLayout),
        g_bindings(std::move(r.g_bindings))
{
    r.g_descriptorSetLayout = VK_NULL_HANDLE;
}
DescriptorSetLayout::~DescriptorSetLayout()
{
    this->destroy();
}

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout const& r)
{
    this->verifyContext(r);

    this->create(r.g_bindings.data(), static_cast<uint32_t>(r.g_bindings.size()));

    return *this;
}
DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& r) noexcept
{
    this->verifyContext(r);

    this->destroy();

    this->g_bindings = std::move(r.g_bindings);
    this->g_descriptorSetLayout = r.g_descriptorSetLayout;

    r.g_descriptorSetLayout = VK_NULL_HANDLE;

    return *this;
}

void DescriptorSetLayout::create(Binding const* bindings, uint32_t bindingCount)
{
    this->destroy();

    if (bindings == nullptr || bindingCount == 0)
    {
        return;
    }

    this->g_bindings.assign(bindings, bindings + bindingCount);

    auto* layoutBindings = FGE_ALLOCA_T(VkDescriptorSetLayoutBinding, bindingCount);
    auto* layoutBindingFlags = FGE_ALLOCA_T(VkDescriptorBindingFlags, bindingCount);

    bool haveBindingFlags = false;
    for (uint32_t i = 0; i < bindingCount; ++i)
    {
        layoutBindings[i] = static_cast<VkDescriptorSetLayoutBinding>(this->g_bindings[i]);
        layoutBindingFlags[i] = this->g_bindings[i].getBindingFlags();
        haveBindingFlags |= layoutBindingFlags[i] != 0;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(this->g_bindings.size());
    layoutInfo.pBindings = layoutBindings;
    layoutInfo.pNext = nullptr;

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
    if (haveBindingFlags)
    {
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        bindingFlagsInfo.bindingCount = static_cast<uint32_t>(this->g_bindings.size());
        bindingFlagsInfo.pBindingFlags = layoutBindingFlags;
        layoutInfo.pNext = &bindingFlagsInfo;
    }

    if (vkCreateDescriptorSetLayout(this->getContext().getLogicalDevice().getDevice(), &layoutInfo, nullptr,
                                    &this->g_descriptorSetLayout) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create descriptor set layout!");
    }
}
void DescriptorSetLayout::destroy()
{
    if (this->g_descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(this->getContext().getLogicalDevice().getDevice(), this->g_descriptorSetLayout,
                                     nullptr);
        this->g_descriptorSetLayout = VK_NULL_HANDLE;
        this->g_bindings.clear();
    }
}

VkDescriptorSetLayout DescriptorSetLayout::getLayout() const
{
    return this->g_descriptorSetLayout;
}
std::vector<DescriptorSetLayout::Binding> const& DescriptorSetLayout::getBindings() const
{
    return this->g_bindings;
}
uint32_t DescriptorSetLayout::getBindingsCount() const
{
    return static_cast<uint32_t>(this->g_bindings.size());
}

} // namespace fge::vulkan