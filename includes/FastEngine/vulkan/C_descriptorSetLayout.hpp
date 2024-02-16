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

#ifndef _FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED
#define _FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include "SDL_vulkan.h"
#include <initializer_list>
#include <vector>

namespace fge::vulkan
{

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
    class Binding
    {
    public:
        constexpr Binding(uint32_t binding,
                          VkDescriptorType type,
                          VkShaderStageFlags stageFlags,
                          uint32_t descriptorCount = 1,
                          VkDescriptorBindingFlagsEXT bindingFlags = 0) :
                g_binding(binding),
                g_descriptorType(type),
                g_descriptorCount(descriptorCount),
                g_stageFlags(stageFlags),
                g_bindingFlags(bindingFlags)
        {}

        constexpr void setBinding(uint32_t binding) { this->g_binding = binding; }
        [[nodiscard]] constexpr uint32_t getBinding() const { return this->g_binding; }

        constexpr void setDescriptorType(VkDescriptorType type) { this->g_descriptorType = type; }
        [[nodiscard]] constexpr VkDescriptorType getDescriptorType() const { return this->g_descriptorType; }

        constexpr void setDescriptorCount(uint32_t descriptorCount) { this->g_descriptorCount = descriptorCount; }
        [[nodiscard]] constexpr uint32_t getDescriptorCount() const { return this->g_descriptorCount; }

        constexpr void setStageFlags(VkShaderStageFlags stageFlags) { this->g_stageFlags = stageFlags; }
        [[nodiscard]] constexpr VkShaderStageFlags getStageFlags() const { return this->g_stageFlags; }

        constexpr void setBindingFlags(VkDescriptorBindingFlagsEXT bindingFlags)
        {
            this->g_bindingFlags = bindingFlags;
        }
        constexpr void clearBindingFlags() { this->g_bindingFlags = 0; }
        [[nodiscard]] constexpr VkDescriptorBindingFlagsEXT getBindingFlags() const { return this->g_bindingFlags; }

        constexpr explicit operator VkDescriptorSetLayoutBinding() const
        {
            return {.binding = this->g_binding,
                    .descriptorType = this->g_descriptorType,
                    .descriptorCount = this->g_descriptorCount,
                    .stageFlags = this->g_stageFlags,
                    .pImmutableSamplers = nullptr};
        }

    private:
        uint32_t g_binding;
        VkDescriptorType g_descriptorType;
        uint32_t g_descriptorCount;
        VkShaderStageFlags g_stageFlags;
        VkDescriptorBindingFlagsEXT g_bindingFlags;
    };

    explicit DescriptorSetLayout(Context const& context);
    DescriptorSetLayout(DescriptorSetLayout const& r);
    DescriptorSetLayout(DescriptorSetLayout&& r) noexcept;
    ~DescriptorSetLayout() override;

    DescriptorSetLayout& operator=(DescriptorSetLayout const& r);
    DescriptorSetLayout& operator=(DescriptorSetLayout&& r) noexcept;

    void create(Binding const* bindings, uint32_t bindingCount);
    inline void create(std::initializer_list<Binding> bindings)
    {
        this->create(bindings.begin(), static_cast<uint32_t>(bindings.size()));
    }
    void destroy() final;

    [[nodiscard]] VkDescriptorSetLayout getLayout() const;
    [[nodiscard]] std::vector<Binding> const& getBindings() const;
    [[nodiscard]] uint32_t getBindingsCount() const;

private:
    VkDescriptorSetLayout g_descriptorSetLayout;
    std::vector<Binding> g_bindings;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_DESCRIPTORSETLAYOUT_HPP_INCLUDED
