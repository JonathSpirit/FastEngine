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

#include "FastEngine/fastengine_extern.hpp"
#include "SDL_vulkan.h"
#include <vector>
#include "volk.h"
#include "glm/glm.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/graphic/C_rect.hpp"
#include "FastEngine/graphic/C_vector.hpp"

namespace fge::vulkan
{

class Context;
class PhysicalDevice;

class FGE_API TextureImage
{
public:
    TextureImage();
    TextureImage(const TextureImage& r) = delete;
    TextureImage(TextureImage&& r) noexcept;
    ~TextureImage();

    TextureImage& operator=(const TextureImage& r) = delete;
    TextureImage& operator=(TextureImage&& r) noexcept;

    bool create(const Context& context, const glm::vec<2, int>& size);
    bool create(const Context& context, SDL_Surface* surface);
    void destroy();

    [[nodiscard]] SDL_Surface* copyToSurface() const;

    void update(SDL_Surface* surface, const glm::vec<2, int>& offset);
    void update(const TextureImage& textureImage, const glm::vec<2, int>& offset);
    void update(void* buffer, std::size_t bufferSize, const glm::vec<2, int>& size, const glm::vec<2, int>& offset);

    [[nodiscard]] const glm::vec<2, int>& getSize() const;
    [[nodiscard]] VkExtent2D getExtent() const;
    [[nodiscard]] int getBytesPerPixel() const;

    [[nodiscard]] VkImage getTextureImage() const;
    [[nodiscard]] VkDeviceMemory getTextureImageMemory() const;

    [[nodiscard]] VkImageView getTextureImageView() const;
    [[nodiscard]] VkSampler getTextureSampler() const;

    void setNormalizedCoordinates(bool normalized);
    [[nodiscard]] bool getNormalizedCoordinates() const;

    void setFilter(VkFilter filter);
    [[nodiscard]] VkFilter getFilter() const;

    [[nodiscard]] const Context* getContext() const;

    [[nodiscard]] const fge::vulkan::DescriptorSet& getDescriptorSet() const;

    [[nodiscard]] fge::Vector2f normalizeTextureCoords(const fge::Vector2i& coords) const;
    [[nodiscard]] fge::RectFloat normalizeTextureRect(const fge::RectInt& rect) const;

    [[nodiscard]] uint32_t getModificationCount() const;

private:
    void createTextureSampler(const PhysicalDevice& physicalDevice);

    VkImage g_textureImage;
    VkDeviceMemory g_textureImageMemory;

    VkImageView g_textureImageView;
    VkSampler g_textureSampler;

    glm::vec<2, int> g_textureSize;
    int g_textureBytesPerPixel;

    VkFilter g_filter;
    bool g_normalizedCoordinates;

    fge::vulkan::DescriptorSet g_textureDescriptorSet;

    uint32_t g_modificationCount;

    const Context* g_context;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_TEXTUREIMAGE_HPP_INCLUDED
