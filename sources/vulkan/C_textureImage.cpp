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

#include "FastEngine/vulkan/C_textureImage.hpp"
#include "FastEngine/fge_except.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <cstring>

#define FGE_VULKAN_TEXTUREIMAGE_FORMAT VK_FORMAT_R8G8B8A8_UNORM

namespace fge::vulkan
{

TextureImage::TextureImage(Context const& context) :
        ContextAware(context),
        g_textureImage(VK_NULL_HANDLE),
        g_textureImageAllocation(VK_NULL_HANDLE),

        g_textureImageView(VK_NULL_HANDLE),
        g_textureSampler(VK_NULL_HANDLE),

        g_textureSize(0, 0),
        g_textureBytesPerPixel(0),

        g_filter(VK_FILTER_NEAREST),
        g_normalizedCoordinates(true),

        g_modificationCount(0)
{}
TextureImage::TextureImage(TextureImage&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_textureImage(r.g_textureImage),
        g_textureImageAllocation(r.g_textureImageAllocation),

        g_textureImageView(r.g_textureImageView),
        g_textureSampler(r.g_textureSampler),

        g_textureSize(r.g_textureSize),
        g_textureBytesPerPixel(r.g_textureBytesPerPixel),

        g_filter(r.g_filter),
        g_normalizedCoordinates(r.g_normalizedCoordinates),

        g_textureDescriptorSet(std::move(r.g_textureDescriptorSet)),

        g_modificationCount(r.g_modificationCount)
{
    r.g_textureImage = VK_NULL_HANDLE;
    r.g_textureImageAllocation = VK_NULL_HANDLE;

    r.g_textureImageView = VK_NULL_HANDLE;
    r.g_textureSampler = VK_NULL_HANDLE;

    r.g_textureSize = {0, 0};
    r.g_textureBytesPerPixel = 0;

    r.g_filter = VK_FILTER_NEAREST;
    r.g_normalizedCoordinates = false;

    r.g_modificationCount = 0;
}
TextureImage::~TextureImage()
{
    this->destroy();
}

TextureImage& TextureImage::operator=(TextureImage&& r) noexcept
{
    this->verifyContext(r);

    this->destroy();

    this->g_textureImage = r.g_textureImage;
    this->g_textureImageAllocation = r.g_textureImageAllocation;

    this->g_textureImageView = r.g_textureImageView;
    this->g_textureSampler = r.g_textureSampler;

    this->g_textureSize = r.g_textureSize;
    this->g_textureBytesPerPixel = r.g_textureBytesPerPixel;

    this->g_filter = r.g_filter;
    this->g_normalizedCoordinates = r.g_normalizedCoordinates;

    this->g_textureDescriptorSet = std::move(r.g_textureDescriptorSet);

    ++this->g_modificationCount;

    r.g_textureImage = VK_NULL_HANDLE;
    r.g_textureImageAllocation = VK_NULL_HANDLE;

    r.g_textureImageView = VK_NULL_HANDLE;
    r.g_textureSampler = VK_NULL_HANDLE;

    r.g_textureSize = {0, 0};
    r.g_textureBytesPerPixel = 0;

    r.g_filter = VK_FILTER_NEAREST;
    r.g_normalizedCoordinates = false;

    r.g_modificationCount = 0;

    return *this;
}

bool TextureImage::create(glm::vec<2, int> const& size)
{
    this->destroy();

    ++this->g_modificationCount;

    if (size.x == 0 || size.y == 0)
    {
        return false;
    }

    auto& context = this->getContext();

    VkDeviceSize const imageSize = static_cast<VkDeviceSize>(size.x) * size.y * 4;

    this->g_textureSize = size;
    this->g_textureBytesPerPixel = 4;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

    CreateBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferAllocation);

    std::vector<uint8_t> pixels(imageSize, 0);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), stagingBufferAllocation, &data);
    memcpy(data, pixels.data(), static_cast<size_t>(imageSize));
    vmaUnmapMemory(context.getAllocator(), stagingBufferAllocation);

    CreateImage(context, size.x, size.y, FGE_VULKAN_TEXTUREIMAGE_FORMAT, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->g_textureImage, this->g_textureImageAllocation);

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    context.copyBufferToImage(stagingBuffer, this->g_textureImage, static_cast<uint32_t>(size.x),
                              static_cast<uint32_t>(size.y));

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(context.getAllocator(), stagingBuffer, stagingBufferAllocation);

    this->g_textureImageView =
            CreateImageView(context.getLogicalDevice(), this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT);

    this->createTextureSampler();

    this->g_textureDescriptorSet =
            context.getTextureDescriptorPool().allocateDescriptorSet(context.getTextureLayout().getLayout()).value();

    DescriptorSet::Descriptor const descriptor(*this, FGE_VULKAN_TEXTURE_BINDING);
    this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
    return true;
}
bool TextureImage::create(SDL_Surface* surface)
{
    this->destroy();

    ++this->g_modificationCount;

    if (surface == nullptr)
    {
        return false;
    }

    auto& context = this->getContext();

    VkDeviceSize const imageSize = static_cast<VkDeviceSize>(surface->w) * surface->h * surface->format->BytesPerPixel;

    this->g_textureSize = {surface->w, surface->h};
    this->g_textureBytesPerPixel = surface->format->BytesPerPixel;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

    CreateBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferAllocation);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), stagingBufferAllocation, &data);
    memcpy(data, surface->pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(context.getAllocator(), stagingBufferAllocation);

    CreateImage(context, surface->w, surface->h, FGE_VULKAN_TEXTUREIMAGE_FORMAT, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->g_textureImage, this->g_textureImageAllocation);

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    context.copyBufferToImage(stagingBuffer, this->g_textureImage, static_cast<uint32_t>(surface->w),
                              static_cast<uint32_t>(surface->h));

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(context.getAllocator(), stagingBuffer, stagingBufferAllocation);

    this->g_textureImageView =
            CreateImageView(context.getLogicalDevice(), this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT);

    this->createTextureSampler();

    this->g_textureDescriptorSet =
            context.getTextureDescriptorPool().allocateDescriptorSet(context.getTextureLayout().getLayout()).value();

    DescriptorSet::Descriptor const descriptor(*this, FGE_VULKAN_TEXTURE_BINDING);
    this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
    return true;
}
void TextureImage::destroy()
{
    if (this->g_textureImage != VK_NULL_HANDLE)
    {
        this->g_textureDescriptorSet.destroy();

        this->getContext()._garbageCollector.push(
                fge::vulkan::GarbageSampler(this->g_textureSampler, this->getContext().getLogicalDevice().getDevice()));

        this->getContext()._garbageCollector.push(fge::vulkan::GarbageImage(
                this->g_textureImage, this->g_textureImageAllocation, this->g_textureImageView, &this->getContext()));

        this->g_textureImageView = VK_NULL_HANDLE;

        this->g_textureImage = VK_NULL_HANDLE;
        this->g_textureImageAllocation = VK_NULL_HANDLE;

        this->g_textureSize = {0, 0};
        this->g_textureBytesPerPixel = 0;

        this->g_filter = VK_FILTER_NEAREST;

        this->g_modificationCount = 0;
    }
}

SDL_Surface* TextureImage::copyToSurface() const
{
    if (this->g_textureImage == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    SDL_Surface* surface =
            SDL_CreateRGBSurfaceWithFormat(0, this->g_textureSize.x, this->g_textureSize.y, 32, SDL_PIXELFORMAT_RGBA32);
    if (surface == nullptr)
    {
        return nullptr;
    }

    VkDeviceSize const imageSize =
            static_cast<VkDeviceSize>(this->g_textureSize.x) * this->g_textureSize.y * this->g_textureBytesPerPixel;

    VkBuffer dstBuffer = VK_NULL_HANDLE;
    VmaAllocation dstBufferAllocation = VK_NULL_HANDLE;

    auto& context = this->getContext();

    CreateBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, dstBuffer,
                 dstBufferAllocation);

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    context.copyImageToBuffer(this->g_textureImage, dstBuffer, this->g_textureSize.x, this->g_textureSize.y);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), dstBufferAllocation, &data);
    memcpy(surface->pixels, data, static_cast<size_t>(imageSize));
    vmaUnmapMemory(context.getAllocator(), dstBufferAllocation);

    vmaDestroyBuffer(context.getAllocator(), dstBuffer, dstBufferAllocation);

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return surface;
}

void TextureImage::update(SDL_Surface* surface, glm::vec<2, int> const& offset)
{
    if (surface == nullptr)
    {
        return;
    }
    if ((surface->w + offset.x > this->g_textureSize.x) || (surface->h + offset.y > this->g_textureSize.y))
    {
        return;
    }

    ++this->g_modificationCount;

    auto& context = this->getContext();

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkDeviceSize const imageSize = static_cast<VkDeviceSize>(surface->w) * surface->h * surface->format->BytesPerPixel;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

    CreateBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferAllocation);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), stagingBufferAllocation, &data);
    memcpy(data, surface->pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(context.getAllocator(), stagingBufferAllocation);

    context.copyBufferToImage(stagingBuffer, this->g_textureImage, surface->w, surface->h, offset.x, offset.y);

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(context.getAllocator(), stagingBuffer, stagingBufferAllocation);
}
void TextureImage::update(TextureImage const& textureImage, glm::vec<2, int> const& offset)
{
    if (textureImage.g_textureImage == VK_NULL_HANDLE)
    {
        return;
    }
    if ((textureImage.g_textureSize.x + offset.x > this->g_textureSize.x) ||
        (textureImage.g_textureSize.y + offset.y > this->g_textureSize.y))
    {
        return;
    }

    ++this->g_modificationCount;

    auto& context = this->getContext();

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    context.transitionImageLayout(textureImage.g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    context.copyImageToImage(textureImage.g_textureImage, this->g_textureImage, textureImage.g_textureSize.x,
                             textureImage.g_textureSize.y, offset.x, offset.y);

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    context.transitionImageLayout(textureImage.g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
void TextureImage::update(void* buffer,
                          std::size_t bufferSize,
                          glm::vec<2, int> const& size,
                          glm::vec<2, int> const& offset)
{
    if (buffer == nullptr || bufferSize == 0)
    {
        return;
    }
    if ((size.x + offset.x > this->g_textureSize.x) || (size.y + offset.y > this->g_textureSize.y))
    {
        return;
    }

    ++this->g_modificationCount;

    auto& context = this->getContext();

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkDeviceSize const imageSize = bufferSize;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

    CreateBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferAllocation);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), stagingBufferAllocation, &data);
    memcpy(data, buffer, bufferSize);
    vmaUnmapMemory(context.getAllocator(), stagingBufferAllocation);

    context.copyBufferToImage(stagingBuffer, this->g_textureImage, size.x, size.y, offset.x, offset.y);

    context.transitionImageLayout(this->g_textureImage, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(context.getAllocator(), stagingBuffer, stagingBufferAllocation);
}

glm::vec<2, int> const& TextureImage::getSize() const
{
    return this->g_textureSize;
}
VkExtent2D TextureImage::getExtent() const
{
    return {static_cast<uint32_t>(this->g_textureSize.x), static_cast<uint32_t>(this->g_textureSize.y)};
}
int TextureImage::getBytesPerPixel() const
{
    return this->g_textureBytesPerPixel;
}

VkImage TextureImage::getTextureImage() const
{
    return this->g_textureImage;
}
VmaAllocation TextureImage::getTextureImageAllocation() const
{
    return this->g_textureImageAllocation;
}

VkImageView TextureImage::getTextureImageView() const
{
    return this->g_textureImageView;
}
VkSampler TextureImage::getTextureSampler() const
{
    return this->g_textureSampler;
}

void TextureImage::setNormalizedCoordinates(bool normalized)
{
    if (this->g_normalizedCoordinates != normalized)
    {
        this->g_normalizedCoordinates = normalized;
        this->getContext()._garbageCollector.push(
                fge::vulkan::GarbageSampler(this->g_textureSampler, this->getContext().getLogicalDevice().getDevice()));
        this->createTextureSampler();

        DescriptorSet::Descriptor const descriptor(*this, FGE_VULKAN_TEXTURE_BINDING);
        this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
    }
}
bool TextureImage::getNormalizedCoordinates() const
{
    return this->g_normalizedCoordinates;
}

void TextureImage::setFilter(VkFilter filter)
{
    if (this->g_filter != filter)
    {
        this->g_filter = filter;
        this->getContext()._garbageCollector.push(
                fge::vulkan::GarbageSampler(this->g_textureSampler, this->getContext().getLogicalDevice().getDevice()));
        this->createTextureSampler();

        DescriptorSet::Descriptor const descriptor(*this, FGE_VULKAN_TEXTURE_BINDING);
        this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
    }
}
VkFilter TextureImage::getFilter() const
{
    return this->g_filter;
}

fge::vulkan::DescriptorSet const& TextureImage::getDescriptorSet() const
{
    return this->g_textureDescriptorSet;
}

fge::Vector2f TextureImage::normalizeTextureCoords(fge::Vector2i const& coords) const
{
    if (this->g_textureSize.x == 0 || this->g_textureSize.y == 0)
    {
        return {0.0f, 0.0f};
    }
    return {static_cast<float>(coords.x) / static_cast<float>(this->g_textureSize.x),
            static_cast<float>(coords.y) / static_cast<float>(this->g_textureSize.y)};
}
fge::RectFloat TextureImage::normalizeTextureRect(fge::RectInt const& rect) const
{
    if (this->g_textureSize.x == 0 || this->g_textureSize.y == 0)
    {
        return {{0.0f, 0.0f}, {0.0f, 0.0f}};
    }

    return {{static_cast<float>(rect._x) / static_cast<float>(this->g_textureSize.x),
             static_cast<float>(rect._y) / static_cast<float>(this->g_textureSize.y)},
            {static_cast<float>(rect._width) / static_cast<float>(this->g_textureSize.x),
             static_cast<float>(rect._height) / static_cast<float>(this->g_textureSize.y)}};
}

uint32_t TextureImage::getModificationCount() const
{
    return this->g_modificationCount;
}

void TextureImage::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = this->g_filter;
    samplerInfo.minFilter = this->g_filter;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(this->getContext().getPhysicalDevice().getDevice(), &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = this->g_normalizedCoordinates ? VK_FALSE : VK_TRUE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(this->getContext().getLogicalDevice().getDevice(), &samplerInfo, nullptr,
                        &this->g_textureSampler) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create texture sampler!");
    }
}

} // namespace fge::vulkan