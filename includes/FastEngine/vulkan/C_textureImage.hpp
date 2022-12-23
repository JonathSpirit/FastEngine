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

#ifndef _FGE_VULKAN_C_TEXTUREIMAGE_HPP_INCLUDED
#define _FGE_VULKAN_C_TEXTUREIMAGE_HPP_INCLUDED

#include "SDL_vulkan.h"
#include <vector>
#include "volk.h"
#include "glm/glm.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"

namespace fge::vulkan
{

class LogicalDevice;
class PhysicalDevice;
class Context;

class TextureImage
{
public:
    TextureImage();
    TextureImage(const TextureImage& r) = delete;
    TextureImage(TextureImage&& r) noexcept;
    ~TextureImage();

    TextureImage& operator=(const TextureImage& r) = delete;
    TextureImage& operator=(TextureImage&& r) noexcept = delete;

    void create(const Context& context, const glm::vec<2, int>& size);
    void create(const Context& context, SDL_Surface* surface);
    void destroy();

    [[nodiscard]] const glm::vec<2, int>& getSize() const;
    [[nodiscard]] VkExtent2D getExtent() const;
    [[nodiscard]] int getBytesPerPixel() const;

    [[nodiscard]] VkImage getTextureImage() const;
    [[nodiscard]] VkDeviceMemory getTextureImageMemory() const;

    [[nodiscard]] VkImageView getTextureImageView() const;
    [[nodiscard]] VkSampler getTextureSampler() const;

    void setFilter(VkFilter filter);
    [[nodiscard]] VkFilter getFilter() const;

    [[nodiscard]] const LogicalDevice* getLogicalDevice() const;

    [[nodiscard]] const fge::vulkan::DescriptorSet& getDescriptorSet() const;

private:
    void createTextureSampler(const PhysicalDevice& physicalDevice);

    VkImage g_textureImage;
    VkDeviceMemory g_textureImageMemory;

    VkImageView g_textureImageView;
    VkSampler g_textureSampler;

    glm::vec<2, int> g_textureSize;
    int g_textureBytesPerPixel;

    VkFilter g_filter;

    fge::vulkan::DescriptorSet g_textureDescriptorSet;

    const LogicalDevice* g_logicalDevice;
    const PhysicalDevice* g_physicalDevice;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_TEXTUREIMAGE_HPP_INCLUDED
