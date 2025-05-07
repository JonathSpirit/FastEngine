/*
 * Copyright 2025 Guillaume Guillet
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

#ifndef _FGE_VULKAN_C_GARBAGECOLLECTOR_HPP_INCLUDED
#define _FGE_VULKAN_C_GARBAGECOLLECTOR_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"
#include "volk.h"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <array>
#include <vector>

namespace fge::vulkan
{

class Context;

enum class GarbageType
{
    GARBAGE_EMPTY,

    GARBAGE_DESCRIPTOR_SET,
    GARBAGE_VERTEX_BUFFER,
    GARBAGE_GRAPHIC_PIPELINE,
    GARBAGE_PIPELINE_LAYOUT,
    GARBAGE_COMMAND_POOL,
    GARBAGE_COMMAND_BUFFER,
    GARBAGE_FRAMEBUFFER,
    GARBAGE_RENDERPASS,
    GARBAGE_SAMPLER,
    GARBAGE_IMAGE
};

struct GarbageGeneric
{
    GarbageType _type;
};
struct GarbageDescriptorSet
{
    constexpr GarbageDescriptorSet(VkDescriptorSet descriptorSet,
                                   VkDescriptorPool descriptorPool,
                                   VkDevice logicalDevice) :
            _type(GarbageType::GARBAGE_DESCRIPTOR_SET),
            _descriptorSet(descriptorSet),
            _descriptorPool(descriptorPool),
            _logicalDevice(logicalDevice)
    {}

    GarbageType _type;
    VkDescriptorSet _descriptorSet;
    VkDescriptorPool _descriptorPool;
    VkDevice _logicalDevice;
};
struct GarbageBuffer
{
    constexpr GarbageBuffer(BufferInfo bufferInfo, VmaAllocator allocator) :
            _type(GarbageType::GARBAGE_VERTEX_BUFFER),
            _bufferInfo(bufferInfo),
            _allocator(allocator)
    {}

    GarbageType _type;
    BufferInfo _bufferInfo;
    VmaAllocator _allocator;
};
struct GarbageGraphicPipeline
{
    constexpr GarbageGraphicPipeline(VkPipeline pipeline, VkDevice logicalDevice) :
            _type(GarbageType::GARBAGE_GRAPHIC_PIPELINE),
            _pipeline(pipeline),
            _logicalDevice(logicalDevice)
    {}

    GarbageType _type;
    VkPipeline _pipeline;
    VkDevice _logicalDevice;
};
struct GarbagePipelineLayout
{
    constexpr GarbagePipelineLayout(VkPipelineLayout pipelineLayout, VkDevice logicalDevice) :
            _type(GarbageType::GARBAGE_PIPELINE_LAYOUT),
            _pipelineLayout(pipelineLayout),
            _logicalDevice(logicalDevice)
    {}

    GarbageType _type;
    VkPipelineLayout _pipelineLayout;
    VkDevice _logicalDevice;
};
struct GarbageCommandPool
{
    constexpr GarbageCommandPool(VkCommandPool commandPool, VkDevice logicalDevice) :
            _type(GarbageType::GARBAGE_COMMAND_POOL),
            _commandPool(commandPool),
            _logicalDevice(logicalDevice)
    {}

    GarbageType _type;
    VkCommandPool _commandPool;
    VkDevice _logicalDevice;
};
struct GarbageCommandBuffer
{
    constexpr GarbageCommandBuffer(VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkDevice logicalDevice) :
            _type(GarbageType::GARBAGE_COMMAND_BUFFER),
            _commandPool(commandPool),
            _commandBuffer(commandBuffer),
            _logicalDevice(logicalDevice)
    {}

    GarbageType _type;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
    VkDevice _logicalDevice;
};
struct GarbageFramebuffer
{
    constexpr GarbageFramebuffer(VkFramebuffer framebuffer, VkDevice logicalDevice) :
            _type(GarbageType::GARBAGE_FRAMEBUFFER),
            _framebuffer(framebuffer),
            _logicalDevice(logicalDevice)
    {}

    GarbageType _type;
    VkFramebuffer _framebuffer;
    VkDevice _logicalDevice;
};
struct GarbageRenderPass
{
    constexpr GarbageRenderPass(VkRenderPass renderPass, VkDevice logicalDevice) :
            _type(GarbageType::GARBAGE_RENDERPASS),
            _renderPass(renderPass),
            _logicalDevice(logicalDevice)
    {}

    GarbageType _type;
    VkRenderPass _renderPass;
    VkDevice _logicalDevice;
};
struct GarbageSampler
{
    constexpr GarbageSampler(VkSampler sampler, VkDevice logicalDevice) :
            _type(GarbageType::GARBAGE_SAMPLER),
            _sampler(sampler),
            _logicalDevice(logicalDevice)
    {}

    GarbageType _type;
    VkSampler _sampler;
    VkDevice _logicalDevice;
};
struct GarbageImage
{
    constexpr GarbageImage(VkImage image,
                           VmaAllocation bufferAllocation,
                           VkImageView imageView,
                           Context const* context) :
            _type(GarbageType::GARBAGE_IMAGE),
            _image(image),
            _allocation(bufferAllocation),
            _imageView(imageView),
            _context(context)
    {}

    GarbageType _type;
    VkImage _image;
    VmaAllocation _allocation;
    VkImageView _imageView;
    Context const* _context;
};

/**
 * \class Garbage
 * \ingroup vulkan
 * \brief A class that holds a garbage object
 *
 * \see GarbageCollector
 */
class FGE_API Garbage final
{
private:
    constexpr Garbage() :
            g_data(GarbageType::GARBAGE_EMPTY)
    {}

public:
    constexpr Garbage(GarbageDescriptorSet const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbageBuffer const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbageGraphicPipeline const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbagePipelineLayout const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbageCommandPool const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbageCommandBuffer const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbageFramebuffer const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbageRenderPass const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbageSampler const& garbage) :
            g_data(garbage)
    {}
    constexpr Garbage(GarbageImage const& garbage) :
            g_data(garbage)
    {}
    Garbage(Garbage const& r) = delete;
    Garbage(Garbage&& r) noexcept :
            g_data(r.g_data)
    {
        r.g_data._generic._type = GarbageType::GARBAGE_EMPTY;
    }
    ~Garbage();

    Garbage& operator=(Garbage const& r) = delete;
    Garbage& operator=(Garbage&& r) noexcept = delete;

private:
    union Data
    {
        explicit constexpr Data(GarbageType type) :
                _generic{type}
        {}
        explicit constexpr Data(GarbageDescriptorSet const& data) :
                _descriptorSet{data}
        {}
        explicit constexpr Data(GarbageBuffer const& data) :
                _buffer{data}
        {}
        explicit constexpr Data(GarbageGraphicPipeline const& data) :
                _graphicPipeline{data}
        {}
        explicit constexpr Data(GarbagePipelineLayout const& data) :
                _pipelineLayout{data}
        {}
        explicit constexpr Data(GarbageCommandPool const& data) :
                _commandPool{data}
        {}
        explicit constexpr Data(GarbageCommandBuffer const& data) :
                _commandBuffer{data}
        {}
        explicit constexpr Data(GarbageFramebuffer const& data) :
                _framebuffer{data}
        {}
        explicit constexpr Data(GarbageRenderPass const& data) :
                _renderPass{data}
        {}
        explicit constexpr Data(GarbageSampler const& data) :
                _sampler{data}
        {}
        explicit constexpr Data(GarbageImage const& data) :
                _image{data}
        {}

        GarbageGeneric _generic;
        GarbageDescriptorSet _descriptorSet;
        GarbageBuffer _buffer;
        GarbageGraphicPipeline _graphicPipeline;
        GarbagePipelineLayout _pipelineLayout;
        GarbageCommandPool _commandPool;
        GarbageCommandBuffer _commandBuffer;
        GarbageFramebuffer _framebuffer;
        GarbageRenderPass _renderPass;
        GarbageSampler _sampler;
        GarbageImage _image;
    };

    Data g_data;
};

/**
 * \class GarbageCollector
 * \ingroup vulkan
 * \brief A garbage collector for Vulkan objects
 *
 * In Vulkan when recording a command buffer, you can't destroy a resource that is used by it.
 * In order to address this problem, a garbage collector is used. It will collect all the unused
 * resources and free them when the command buffer is done.
 */
class FGE_API GarbageCollector
{
public:
    using ContainerType = std::vector<Garbage>;

    GarbageCollector() = default;
    GarbageCollector(GarbageCollector const& r) = delete;
    GarbageCollector(GarbageCollector&& r) noexcept;
    ~GarbageCollector() = default;

    GarbageCollector& operator=(GarbageCollector const& r) = delete;
    GarbageCollector& operator=(GarbageCollector&& r) noexcept = delete;

    /**
     * \brief Set the current frame
     *
     * Set the current frame respecting the maximum number of frames in flight.
     * When switching to a new frame, the garbage collector will free all the
     * resources that were collected previously.
     *
     * \param frame The current frame
     */
    void setCurrentFrame(uint32_t frame);
    [[nodiscard]] uint32_t getCurrentFrame() const;
    /**
     * \brief Push a garbage object
     *
     * Push a garbage object associated with the current frame.
     * The garbage will be freed when switching to a new frame.
     *
     * \param garbage The garbage object
     */
    void push(Garbage garbage) const;
    /**
     * \brief Free all the garbage objects in the current frame
     *
     * This method should not be called manually for normal use.
     */
    void free();
    /**
     * \brief Free all the garbage objects
     *
     * This method should be only called when the application is closing
     * just after the scene has been destroyed.
     */
    void freeAll();

    /**
     * \brief Enable or disable the garbage collector
     *
     * When the garbage collector is disabled, all the garbage objects
     * will be freed immediately.
     *
     * By default, the garbage collector is disabled.
     *
     * When the user disable the garbage collector, the freeAll() method
     * is called automatically.
     *
     * \param stat \b false to disable the garbage collector, \b true to enable it
     */
    void enable(bool stat);
    [[nodiscard]] bool isEnabled() const;

private:
    mutable std::array<ContainerType, FGE_MAX_FRAMES_IN_FLIGHT> g_containers;
    uint32_t g_currentFrame = 0;
    bool g_enabled = false;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_GARBAGECOLLECTOR_HPP_INCLUDED
