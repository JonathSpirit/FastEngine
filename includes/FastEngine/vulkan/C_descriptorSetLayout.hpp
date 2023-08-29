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

#ifndef _FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "volk.h"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include "SDL_vulkan.h"
#include <initializer_list>
#include <vector>

namespace fge::vulkan
{

/**
 * \ingroup vulkan
 * \brief Function to create a simple VkDescriptorSetLayoutBinding.
 *
 * \param binding The binding number that this descriptor uses in the shader
 * \param type The type of descriptor
 * \param stageFlags Which shader stage(s) will use this descriptor
 */
constexpr VkDescriptorSetLayoutBinding
CreateSimpleLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags)
{
    return {.binding = binding,
            .descriptorType = type,
            .descriptorCount = 1,
            .stageFlags = stageFlags,
            .pImmutableSamplers = nullptr};
}

/**
 * \class DescriptorSetLayout
 * \ingroup vulkan
 * \brief This class abstract the vulkan descriptor set layout for easier use
 *
 * Essentially, this class abstract creation and destruction of the descriptor set layout.
 * It also enable copy and move semantics.
 */
class FGE_API DescriptorSetLayout : public ContextAware
{
public:
    explicit DescriptorSetLayout(Context const& context);
    DescriptorSetLayout(const DescriptorSetLayout& r) = delete; ///TODO
    DescriptorSetLayout(DescriptorSetLayout&& r) noexcept;
    ~DescriptorSetLayout();

    DescriptorSetLayout& operator=(const DescriptorSetLayout& r) = delete; ///TODO
    DescriptorSetLayout& operator=(DescriptorSetLayout&& r) noexcept;

    void create(std::initializer_list<VkDescriptorSetLayoutBinding> bindings,
                VkDescriptorBindingFlagsEXT const* bindingFlags = nullptr);
    void destroy();

    [[nodiscard]] VkDescriptorSetLayout getLayout() const;
    [[nodiscard]] const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const;
    [[nodiscard]] std::size_t getBindingsCount() const;

private:
    VkDescriptorSetLayout g_descriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding> g_bindings;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED
