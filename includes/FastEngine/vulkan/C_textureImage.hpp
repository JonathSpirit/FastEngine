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

#ifndef _FGE_VULKAN_C_TEXTUREIMAGE_HPP_INCLUDED
#define _FGE_VULKAN_C_TEXTUREIMAGE_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "FastEngine/C_rect.hpp"
#include "FastEngine/C_vector.hpp"
#include "FastEngine/vulkan/C_contextAware.hpp"
#include "FastEngine/vulkan/C_descriptorSet.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include "SDL_surface.h"

#define FGE_TEXTURE_IMAGE_MIPMAPS_LEVELS_AUTO 0

namespace fge::vulkan
{

class FGE_API TextureImage : public ContextAware
{
public:
    explicit TextureImage(Context const& context);
    TextureImage(TextureImage const& r) = delete;
    TextureImage(TextureImage&& r) noexcept;
    ~TextureImage() override;

    TextureImage& operator=(TextureImage const& r) = delete;
    TextureImage& operator=(TextureImage&& r) noexcept;

    bool create(glm::vec<2, int> const& size, uint32_t levels = 1);
    bool create(SDL_Surface* surface, uint32_t levels = 1);
    void destroy() final;

    [[nodiscard]] SDL_Surface* copyToSurface() const;

    void update(SDL_Surface* surface, glm::vec<2, int> const& offset);
    void update(TextureImage const& textureImage, glm::vec<2, int> const& offset);
    void update(void* buffer, std::size_t bufferSize, glm::vec<2, int> const& size, glm::vec<2, int> const& offset);

    void generateMipmaps(uint32_t levels = FGE_TEXTURE_IMAGE_MIPMAPS_LEVELS_AUTO);
    [[nodiscard]] uint32_t getMipLevels() const;
    void forceMipLod(float mipLodBias, float mipLodMin, float mipLodMax);

    [[nodiscard]] glm::vec<2, int> const& getSize() const;
    [[nodiscard]] VkExtent2D getExtent() const;
    [[nodiscard]] int getBytesPerPixel() const;

    [[nodiscard]] VkImage getTextureImage() const;
    [[nodiscard]] VmaAllocation getTextureImageAllocation() const;

    [[nodiscard]] VkImageView getTextureImageView() const;
    [[nodiscard]] VkSampler getTextureSampler() const;

    void setNormalizedCoordinates(bool normalized);
    [[nodiscard]] bool getNormalizedCoordinates() const;

    void setFilter(VkFilter filter);
    [[nodiscard]] VkFilter getFilter() const;

    [[nodiscard]] fge::vulkan::DescriptorSet const& getDescriptorSet() const;

    [[nodiscard]] fge::Vector2f normalizeTextureCoords(fge::Vector2i const& coords) const;
    [[nodiscard]] fge::RectFloat normalizeTextureRect(fge::RectInt const& rect) const;

    [[nodiscard]] uint32_t getModificationCount() const;

private:
    void createTextureSampler(float mipLodBias, float mipLodMin, float mipLodMax);

    ImageInfo g_imageInfo;

    VkImageView g_textureImageView;
    VkSampler g_textureSampler;

    glm::vec<2, int> g_textureSize;
    int g_textureBytesPerPixel;

    VkFilter g_filter;
    bool g_normalizedCoordinates;

    fge::vulkan::DescriptorSet g_textureDescriptorSet;

    uint32_t g_mipLevels;
    uint32_t g_modificationCount;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_TEXTUREIMAGE_HPP_INCLUDED
