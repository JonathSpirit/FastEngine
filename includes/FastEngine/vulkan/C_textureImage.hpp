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
