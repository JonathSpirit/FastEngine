#include "FastEngine/vulkan/C_textureImage.hpp"
#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <stdexcept>

namespace fge::vulkan
{

TextureImage::TextureImage() :
        g_textureImage(VK_NULL_HANDLE),
        g_textureImageMemory(VK_NULL_HANDLE),

        g_textureImageView(VK_NULL_HANDLE),
        g_textureSampler(VK_NULL_HANDLE),

        g_textureSize(0,0),
        g_textureBytesPerPixel(0),

        g_filter(VK_FILTER_LINEAR),

        g_logicalDevice(nullptr),
        g_physicalDevice(nullptr)
{}
TextureImage::TextureImage(TextureImage&& r) noexcept :
        g_textureImage(r.g_textureImage),
        g_textureImageMemory(r.g_textureImageMemory),

        g_textureImageView(r.g_textureImageView),
        g_textureSampler(r.g_textureSampler),

        g_textureSize(r.g_textureSize),
        g_textureBytesPerPixel(r.g_textureBytesPerPixel),

        g_filter(r.g_filter),

        g_logicalDevice(r.g_logicalDevice),
        g_physicalDevice(r.g_physicalDevice)
{
    r.g_textureImage = VK_NULL_HANDLE;
    r.g_textureImageMemory = VK_NULL_HANDLE;

    r.g_textureImageView = VK_NULL_HANDLE;
    r.g_textureSampler = VK_NULL_HANDLE;

    r.g_textureSize = {0,0};
    r.g_textureBytesPerPixel = 0;

    r.g_filter = VK_FILTER_LINEAR;

    r.g_logicalDevice = nullptr;
    r.g_physicalDevice = nullptr;
}
TextureImage::~TextureImage()
{
    this->destroy();
}

void TextureImage::create(const Context& context, const glm::vec<2, int>& size)
{
    this->destroy();

    if (size.x == 0 || size.y == 0)
    {
        return;
    }

    this->g_logicalDevice = &context.getLogicalDevice();
    this->g_physicalDevice = &context.getPhysicalDevice();

    const VkDeviceSize imageSize = static_cast<VkDeviceSize>(size.x) * size.y * 4;

    this->g_textureSize = size;
    this->g_textureBytesPerPixel = 4;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

    CreateBuffer(context.getLogicalDevice(), context.getPhysicalDevice(),
                 imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    std::vector<uint8_t> pixels(imageSize, 0);

    void* data = nullptr;
    vkMapMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels.data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory);

    CreateImage(context.getLogicalDevice(), context.getPhysicalDevice(),
                size.x, size.y,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                this->g_textureImage, this->g_textureImageMemory);

    context.transitionImageLayout(this->g_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    context.copyBufferToImage(stagingBuffer, this->g_textureImage, static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

    context.transitionImageLayout(this->g_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(context.getLogicalDevice().getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory, nullptr);

    this->g_textureImageView = CreateImageView(*this->g_logicalDevice, this->g_textureImage, VK_FORMAT_R8G8B8A8_SRGB);

    this->createTextureSampler(context.getPhysicalDevice());

    this->g_textureDescriptorSet.create(context.getLogicalDevice(),
                                        &context.getDescriptorSetLayout(), 1,
                                        context.getTextureDescriptorPool(), false);

    const DescriptorSet::Descriptor descriptor(*this, 1);
    this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
}
void TextureImage::create(const Context& context, SDL_Surface* surface)
{
    this->destroy();

    if (surface == nullptr)
    {
        return;
    }

    this->g_logicalDevice = &context.getLogicalDevice();
    this->g_physicalDevice = &context.getPhysicalDevice();

    const VkDeviceSize imageSize = static_cast<VkDeviceSize>(surface->w)
            * surface->h
            * surface->format->BytesPerPixel;

    this->g_textureSize = {surface->w, surface->h};
    this->g_textureBytesPerPixel = surface->format->BytesPerPixel;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

    CreateBuffer(context.getLogicalDevice(), context.getPhysicalDevice(),
                 imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void* data = nullptr;
    vkMapMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, surface->pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory);

    CreateImage(context.getLogicalDevice(), context.getPhysicalDevice(),
                surface->w, surface->h,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                this->g_textureImage, this->g_textureImageMemory);

    context.transitionImageLayout(this->g_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    context.copyBufferToImage(stagingBuffer, this->g_textureImage,
                              static_cast<uint32_t>(surface->w),
                              static_cast<uint32_t>(surface->h));

    context.transitionImageLayout(this->g_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(context.getLogicalDevice().getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context.getLogicalDevice().getDevice(), stagingBufferMemory, nullptr);

    this->g_textureImageView = CreateImageView(*this->g_logicalDevice, this->g_textureImage, VK_FORMAT_R8G8B8A8_SRGB);

    this->createTextureSampler(context.getPhysicalDevice());

    this->g_textureDescriptorSet.create(context.getLogicalDevice(),
                                        &context.getDescriptorSetLayout(), 1,
                                        context.getTextureDescriptorPool(), false);

    const DescriptorSet::Descriptor descriptor(*this, 1);
    this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
}
void TextureImage::destroy()
{
    if (this->g_textureImage != VK_NULL_HANDLE)
    {
        this->g_textureDescriptorSet.destroy();

        vkDestroySampler(this->g_logicalDevice->getDevice(), this->g_textureSampler, nullptr);
        vkDestroyImageView(this->g_logicalDevice->getDevice(), this->g_textureImageView, nullptr);

        this->g_textureImageView = VK_NULL_HANDLE;

        vkDestroyImage(this->g_logicalDevice->getDevice(), this->g_textureImage, nullptr);
        vkFreeMemory(this->g_logicalDevice->getDevice(), this->g_textureImageMemory, nullptr);

        this->g_textureImage = VK_NULL_HANDLE;
        this->g_textureImageMemory = VK_NULL_HANDLE;

        this->g_textureSize = {0,0};
        this->g_textureBytesPerPixel = 0;

        this->g_filter = VK_FILTER_LINEAR;

        this->g_logicalDevice = nullptr;
        this->g_physicalDevice = nullptr;
    }
}

const glm::vec<2, int>& TextureImage::getSize() const
{
    return this->g_textureSize;
}
VkExtent2D TextureImage::getExtent() const
{
    return {static_cast<uint32_t>(this->g_textureSize.x),
            static_cast<uint32_t>(this->g_textureSize.y)};
}
int TextureImage::getBytesPerPixel() const
{
    return this->g_textureBytesPerPixel;
}

VkImage TextureImage::getTextureImage() const
{
    return this->g_textureImage;
}
VkDeviceMemory TextureImage::getTextureImageMemory() const
{
    return this->g_textureImageMemory;
}

VkImageView TextureImage::getTextureImageView() const
{
    return this->g_textureImageView;
}
VkSampler TextureImage::getTextureSampler() const
{
    return this->g_textureSampler;
}

void TextureImage::setFilter(VkFilter filter)
{
    if (this->g_filter != filter)
    {
        this->g_filter = filter;
        vkDestroySampler(this->g_logicalDevice->getDevice(), this->g_textureSampler, nullptr);
        this->createTextureSampler(*this->g_physicalDevice);

        const DescriptorSet::Descriptor descriptor(*this, 1);
        this->g_textureDescriptorSet.updateDescriptorSet(&descriptor, 1);
    }
}
VkFilter TextureImage::getFilter() const
{
    return this->g_filter;
}

const LogicalDevice* TextureImage::getLogicalDevice() const
{
    return this->g_logicalDevice;
}

const fge::vulkan::DescriptorSet& TextureImage::getDescriptorSet() const
{
    return this->g_textureDescriptorSet;
}

void TextureImage::createTextureSampler(const PhysicalDevice& physicalDevice)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = this->g_filter;
    samplerInfo.minFilter = this->g_filter;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice.getDevice(), &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(this->g_logicalDevice->getDevice(), &samplerInfo, nullptr, &this->g_textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

}//end fge::vulkan