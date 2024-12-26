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
        g_textureImageView(VK_NULL_HANDLE),
        g_textureSampler(VK_NULL_HANDLE),

        g_textureSize(0, 0),
        g_textureBytesPerPixel(0),

        g_filter(VK_FILTER_NEAREST),
        g_normalizedCoordinates(true),

        g_mipLevels(0),
        g_modificationCount(0)
{}
TextureImage::TextureImage(TextureImage&& r) noexcept :
        ContextAware(static_cast<ContextAware&&>(r)),
        g_imageInfo(r.g_imageInfo),

        g_textureImageView(r.g_textureImageView),
        g_textureSampler(r.g_textureSampler),

        g_textureSize(r.g_textureSize),
        g_textureBytesPerPixel(r.g_textureBytesPerPixel),

        g_filter(r.g_filter),
        g_normalizedCoordinates(r.g_normalizedCoordinates),

        g_textureDescriptorSet(std::move(r.g_textureDescriptorSet)),

        g_mipLevels(r.g_mipLevels),
        g_modificationCount(r.g_modificationCount)
{
    r.g_imageInfo.clear();

    r.g_textureImageView = VK_NULL_HANDLE;
    r.g_textureSampler = VK_NULL_HANDLE;

    r.g_textureSize = {0, 0};
    r.g_textureBytesPerPixel = 0;

    r.g_filter = VK_FILTER_NEAREST;
    r.g_normalizedCoordinates = false;

    r.g_mipLevels = 0;
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

    this->g_imageInfo = r.g_imageInfo;

    this->g_textureImageView = r.g_textureImageView;
    this->g_textureSampler = r.g_textureSampler;

    this->g_textureSize = r.g_textureSize;
    this->g_textureBytesPerPixel = r.g_textureBytesPerPixel;

    this->g_filter = r.g_filter;
    this->g_normalizedCoordinates = r.g_normalizedCoordinates;

    this->g_textureDescriptorSet = std::move(r.g_textureDescriptorSet);
    this->g_mipLevels = r.g_mipLevels;

    ++this->g_modificationCount;

    r.g_imageInfo.clear();

    r.g_textureImageView = VK_NULL_HANDLE;
    r.g_textureSampler = VK_NULL_HANDLE;

    r.g_textureSize = {0, 0};
    r.g_textureBytesPerPixel = 0;

    r.g_filter = VK_FILTER_NEAREST;
    r.g_normalizedCoordinates = false;

    r.g_mipLevels = 0;
    r.g_modificationCount = 0;

    return *this;
}

bool TextureImage::create(glm::vec<2, int> const& size, uint32_t levels)
{
    this->destroy();

    if (levels == FGE_TEXTURE_IMAGE_MIPMAPS_LEVELS_AUTO)
    {
        levels = static_cast<uint32_t>(std::floor(std::log2(std::max(this->g_textureSize.x, this->g_textureSize.y)))) +
                 1;
    }

    ++this->g_modificationCount;

    if (size.x == 0 || size.y == 0)
    {
        return false;
    }

    auto& context = this->getContext();

    VkDeviceSize const imageSize = static_cast<VkDeviceSize>(size.x) * size.y * 4;

    this->g_textureSize = size;
    this->g_textureBytesPerPixel = 4;
    this->g_mipLevels = levels;

    auto const stagingBufferInfo = *context.createBuffer(
            imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    std::vector<uint8_t> pixels(imageSize, 0);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), stagingBufferInfo._allocation, &data);
    memcpy(data, pixels.data(), static_cast<std::size_t>(imageSize));
    vmaUnmapMemory(context.getAllocator(), stagingBufferInfo._allocation);

    this->g_imageInfo = *context.createImage(size.x, size.y, FGE_VULKAN_TEXTUREIMAGE_FORMAT, VK_IMAGE_TILING_OPTIMAL,
                                             this->g_mipLevels,
                                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    auto commandBuffer =
            context.beginCommands(Context::SubmitTypes::DIRECT_WAIT_EXECUTION, CommandBuffer::RenderPassScopes::OUTSIDE,
                                  CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        this->g_mipLevels);
    commandBuffer.copyBufferToImage(stagingBufferInfo._buffer, this->g_imageInfo._image, static_cast<uint32_t>(size.x),
                                    static_cast<uint32_t>(size.y));

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        this->g_mipLevels);

    context.submitCommands(std::move(commandBuffer));

    vmaDestroyBuffer(context.getAllocator(), stagingBufferInfo._buffer, stagingBufferInfo._allocation);

    this->g_textureImageView = CreateImageView(context.getLogicalDevice(), this->g_imageInfo._image,
                                               FGE_VULKAN_TEXTUREIMAGE_FORMAT, this->g_mipLevels);

    this->createTextureSampler(0.0f, 0.0f, static_cast<float>(this->g_mipLevels));

    this->g_textureDescriptorSet =
            context.getTextureDescriptorPool().allocateDescriptorSet(context.getTextureLayout().getLayout()).value();

    DescriptorSet::Descriptor const descriptor(*this, FGE_VULKAN_TEXTURE_BINDING);
    this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
    return true;
}
bool TextureImage::create(SDL_Surface* surface, uint32_t levels)
{
    this->destroy();

    if (levels == FGE_TEXTURE_IMAGE_MIPMAPS_LEVELS_AUTO)
    {
        levels = static_cast<uint32_t>(std::floor(std::log2(std::max(this->g_textureSize.x, this->g_textureSize.y)))) +
                 1;
    }

    ++this->g_modificationCount;

    if (surface == nullptr)
    {
        return false;
    }

    auto& context = this->getContext();

    VkDeviceSize const imageSize = static_cast<VkDeviceSize>(surface->w) * surface->h * surface->format->BytesPerPixel;

    this->g_textureSize = {surface->w, surface->h};
    this->g_textureBytesPerPixel = surface->format->BytesPerPixel;
    this->g_mipLevels = levels;

    auto const stagingBufferInfo = *context.createBuffer(
            imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), stagingBufferInfo._allocation, &data);
    memcpy(data, surface->pixels, static_cast<std::size_t>(imageSize));
    vmaUnmapMemory(context.getAllocator(), stagingBufferInfo._allocation);

    this->g_imageInfo = *context.createImage(surface->w, surface->h, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                             VK_IMAGE_TILING_OPTIMAL, this->g_mipLevels,
                                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                             0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    auto commandBuffer =
            context.beginCommands(Context::SubmitTypes::DIRECT_WAIT_EXECUTION, CommandBuffer::RenderPassScopes::OUTSIDE,
                                  CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        this->g_mipLevels);
    commandBuffer.copyBufferToImage(stagingBufferInfo._buffer, this->g_imageInfo._image,
                                    static_cast<uint32_t>(surface->w), static_cast<uint32_t>(surface->h));

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        this->g_mipLevels);

    context.submitCommands(std::move(commandBuffer));

    vmaDestroyBuffer(context.getAllocator(), stagingBufferInfo._buffer, stagingBufferInfo._allocation);

    this->g_textureImageView = CreateImageView(context.getLogicalDevice(), this->g_imageInfo._image,
                                               FGE_VULKAN_TEXTUREIMAGE_FORMAT, this->g_mipLevels);

    this->createTextureSampler(0.0f, 0.0f, static_cast<float>(this->g_mipLevels));

    this->g_textureDescriptorSet =
            context.getTextureDescriptorPool().allocateDescriptorSet(context.getTextureLayout().getLayout()).value();

    DescriptorSet::Descriptor const descriptor(*this, FGE_VULKAN_TEXTURE_BINDING);
    this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
    return true;
}
void TextureImage::destroy()
{
    if (this->g_imageInfo.valid())
    {
        this->g_textureDescriptorSet.destroy();

        this->getContext()._garbageCollector.push(
                GarbageSampler(this->g_textureSampler, this->getContext().getLogicalDevice().getDevice()));

        this->getContext()._garbageCollector.push(GarbageImage(this->g_imageInfo._image, this->g_imageInfo._allocation,
                                                               this->g_textureImageView, &this->getContext()));

        this->g_textureImageView = VK_NULL_HANDLE;

        this->g_imageInfo.clear();

        this->g_textureSize = {0, 0};
        this->g_textureBytesPerPixel = 0;

        this->g_filter = VK_FILTER_NEAREST;

        this->g_mipLevels = 0;
        this->g_modificationCount = 0;
    }
}

SDL_Surface* TextureImage::copyToSurface() const
{
    if (!this->g_imageInfo.valid())
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

    auto& context = this->getContext();

    auto const dstBufferInfo = *context.createBuffer(
            imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto commandBuffer =
            context.beginCommands(Context::SubmitTypes::DIRECT_WAIT_EXECUTION, CommandBuffer::RenderPassScopes::OUTSIDE,
                                  CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        this->g_mipLevels);

    commandBuffer.copyImageToBuffer(this->g_imageInfo._image, dstBufferInfo._buffer, this->g_textureSize.x,
                                    this->g_textureSize.y);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        this->g_mipLevels);

    context.submitCommands(std::move(commandBuffer));

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), dstBufferInfo._allocation, &data);
    memcpy(surface->pixels, data, static_cast<std::size_t>(imageSize));
    vmaUnmapMemory(context.getAllocator(), dstBufferInfo._allocation);

    vmaDestroyBuffer(context.getAllocator(), dstBufferInfo._buffer, dstBufferInfo._allocation);

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

    auto commandBuffer =
            context.beginCommands(Context::SubmitTypes::DIRECT_WAIT_EXECUTION, CommandBuffer::RenderPassScopes::OUTSIDE,
                                  CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        this->g_mipLevels);

    VkDeviceSize const imageSize = static_cast<VkDeviceSize>(surface->w) * surface->h * surface->format->BytesPerPixel;

    auto const stagingBufferInfo = *context.createBuffer(
            imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), stagingBufferInfo._allocation, &data);
    memcpy(data, surface->pixels, static_cast<std::size_t>(imageSize));
    vmaUnmapMemory(context.getAllocator(), stagingBufferInfo._allocation);

    commandBuffer.copyBufferToImage(stagingBufferInfo._buffer, this->g_imageInfo._image, surface->w, surface->h,
                                    offset.x, offset.y);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        this->g_mipLevels);

    context.submitCommands(std::move(commandBuffer));

    vmaDestroyBuffer(context.getAllocator(), stagingBufferInfo._buffer, stagingBufferInfo._allocation);
}
void TextureImage::update(TextureImage const& textureImage, glm::vec<2, int> const& offset)
{
    if (textureImage.g_imageInfo._image == VK_NULL_HANDLE)
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

    auto commandBuffer =
            context.beginCommands(Context::SubmitTypes::DIRECT_WAIT_EXECUTION, CommandBuffer::RenderPassScopes::OUTSIDE,
                                  CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        this->g_mipLevels);
    commandBuffer.transitionImageLayout(textureImage.g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        this->g_mipLevels);

    commandBuffer.copyImageToImage(textureImage.g_imageInfo._image, this->g_imageInfo._image,
                                   textureImage.g_textureSize.x, textureImage.g_textureSize.y, offset.x, offset.y);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        this->g_mipLevels);
    commandBuffer.transitionImageLayout(textureImage.g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        this->g_mipLevels);

    context.submitCommands(std::move(commandBuffer));
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

    auto commandBuffer =
            context.beginCommands(Context::SubmitTypes::DIRECT_WAIT_EXECUTION, CommandBuffer::RenderPassScopes::OUTSIDE,
                                  CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        this->g_mipLevels);

    VkDeviceSize const imageSize = bufferSize;

    auto const stagingBufferInfo = *context.createBuffer(
            imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data = nullptr;
    vmaMapMemory(context.getAllocator(), stagingBufferInfo._allocation, &data);
    memcpy(data, buffer, bufferSize);
    vmaUnmapMemory(context.getAllocator(), stagingBufferInfo._allocation);

    commandBuffer.copyBufferToImage(stagingBufferInfo._buffer, this->g_imageInfo._image, size.x, size.y, offset.x,
                                    offset.y);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        this->g_mipLevels);

    context.submitCommands(std::move(commandBuffer));

    vmaDestroyBuffer(context.getAllocator(), stagingBufferInfo._buffer, stagingBufferInfo._allocation);
}

void TextureImage::generateMipmaps(uint32_t levels)
{
    if (levels == FGE_TEXTURE_IMAGE_MIPMAPS_LEVELS_AUTO)
    {
        levels = static_cast<uint32_t>(std::floor(std::log2(std::max(this->g_textureSize.x, this->g_textureSize.y)))) +
                 1;
    }

    if (levels != this->g_mipLevels || (levels == 1 && this->g_mipLevels > 1))
    {
        auto* surface = this->copyToSurface();
        if (surface == nullptr)
        {
            return;
        }
        this->create(surface, levels);
    }
    if (levels == 1)
    {
        return;
    }

    auto const& context = this->getContext();

    auto commandBuffer =
            context.beginCommands(Context::SubmitTypes::DIRECT_WAIT_EXECUTION, CommandBuffer::RenderPassScopes::OUTSIDE,
                                  CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);

    commandBuffer.transitionImageLayout(this->g_imageInfo._image, FGE_VULKAN_TEXTUREIMAGE_FORMAT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        this->g_mipLevels);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = this->g_imageInfo._image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = this->g_textureSize.x;
    int32_t mipHeight = this->g_textureSize.y;

    for (uint32_t i = 1; i < this->g_mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer.get(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer.get(), this->g_imageInfo._image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       this->g_imageInfo._image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer.get(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1)
        {
            mipWidth /= 2;
        }
        if (mipHeight > 1)
        {
            mipHeight /= 2;
        }
    }

    barrier.subresourceRange.baseMipLevel = this->g_mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer.get(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    context.submitCommands(std::move(commandBuffer));
}
uint32_t TextureImage::getMipLevels() const
{
    return this->g_mipLevels;
}
void TextureImage::forceMipLod(float mipLodBias, float mipLodMin, float mipLodMax)
{
    this->getContext()._garbageCollector.push(
            fge::vulkan::GarbageSampler(this->g_textureSampler, this->getContext().getLogicalDevice().getDevice()));
    this->createTextureSampler(mipLodBias, mipLodMin, mipLodMax);

    DescriptorSet::Descriptor const descriptor(*this, FGE_VULKAN_TEXTURE_BINDING);
    this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
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
    return this->g_imageInfo._image;
}
VmaAllocation TextureImage::getTextureImageAllocation() const
{
    return this->g_imageInfo._allocation;
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
        this->createTextureSampler(0.0f, 0.0f, static_cast<float>(this->g_mipLevels));

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
        this->createTextureSampler(0.0f, 0.0f, static_cast<float>(this->g_mipLevels));

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

void TextureImage::createTextureSampler(float mipLodBias, float mipLodMin, float mipLodMax)
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
    samplerInfo.mipLodBias = mipLodBias;
    samplerInfo.minLod = mipLodMin;
    samplerInfo.maxLod =
            mipLodMax > static_cast<float>(this->g_mipLevels) ? static_cast<float>(this->g_mipLevels) : mipLodMax;

    if (vkCreateSampler(this->getContext().getLogicalDevice().getDevice(), &samplerInfo, nullptr,
                        &this->g_textureSampler) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create texture sampler!");
    }
}

} // namespace fge::vulkan