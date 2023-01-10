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

#ifndef _FGE_VULKAN_C_GARBAGECOLLECTOR_HPP_INCLUDED
#define _FGE_VULKAN_C_GARBAGECOLLECTOR_HPP_INCLUDED

#include "FastEngine/fastengine_extern.hpp"
#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <array>
#include <deque>
#include "volk.h"

namespace fge::vulkan
{

class FGE_API GarbageCollector
{
public:
    enum class Type
    {
        GARBAGE_EMPTY,
        GARBAGE_DESCRIPTOR_SET,
        GARBAGE_VERTEX_BUFFER,
        GARBAGE_GRAPHIC_PIPELINE,
        GARBAGE_COMMAND_POOL,
        GARBAGE_FRAMEBUFFER,
        GARBAGE_RENDERPASS,
        GARBAGE_SAMPLER,
        GARBAGE_IMAGE
    };

    struct GarbageType
    {
        Type _type;
    };
    struct GarbageDescriptorSet
    {
        GarbageDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorPool descriptorPool, VkDevice logicalDevice) :
                _type(Type::GARBAGE_DESCRIPTOR_SET),
                _descriptorSet(descriptorSet),
                _descriptorPool(descriptorPool),
                _logicalDevice(logicalDevice)
        {}

        Type _type;
        VkDescriptorSet _descriptorSet;
        VkDescriptorPool _descriptorPool;
        VkDevice _logicalDevice;
    };
    struct GarbageBuffer
    {
        GarbageBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory, VkDevice logicalDevice) :
                _type(Type::GARBAGE_VERTEX_BUFFER),
                _buffer(buffer),
                _bufferMemory(bufferMemory),
                _logicalDevice(logicalDevice)
        {}

        Type _type;
        VkBuffer _buffer;
        VkDeviceMemory _bufferMemory;
        VkDevice _logicalDevice;
    };
    struct GarbageGraphicPipeline
    {
        GarbageGraphicPipeline(VkPipelineLayout pipelineLayout, VkPipeline pipeline, VkDevice logicalDevice) :
                _type(Type::GARBAGE_GRAPHIC_PIPELINE),
                _pipelineLayout(pipelineLayout),
                _pipeline(pipeline),
                _logicalDevice(logicalDevice)
        {}

        Type _type;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;
        VkDevice _logicalDevice;
    };
    struct GarbageCommandPool
    {
        GarbageCommandPool(VkCommandPool commandPool, VkDevice logicalDevice) :
                _type(Type::GARBAGE_COMMAND_POOL),
                _commandPool(commandPool),
                _logicalDevice(logicalDevice)
        {}

        Type _type;
        VkCommandPool _commandPool;
        VkDevice _logicalDevice;
    };
    struct GarbageFramebuffer
    {
        GarbageFramebuffer(VkFramebuffer framebuffer, VkDevice logicalDevice) :
                _type(Type::GARBAGE_FRAMEBUFFER),
                _framebuffer(framebuffer),
                _logicalDevice(logicalDevice)
        {}

        Type _type;
        VkFramebuffer _framebuffer;
        VkDevice _logicalDevice;
    };
    struct GarbageRenderPass
    {
        GarbageRenderPass(VkRenderPass renderPass, VkDevice logicalDevice) :
                _type(Type::GARBAGE_RENDERPASS),
                _renderPass(renderPass),
                _logicalDevice(logicalDevice)
        {}

        Type _type;
        VkRenderPass _renderPass;
        VkDevice _logicalDevice;
    };
    struct GarbageSampler
    {
        GarbageSampler(VkSampler sampler, VkDevice logicalDevice) :
                _type(Type::GARBAGE_SAMPLER),
                _sampler(sampler),
                _logicalDevice(logicalDevice)
        {}

        Type _type;
        VkSampler _sampler;
        VkDevice _logicalDevice;
    };
    struct GarbageImage
    {
        GarbageImage(VkImage image, VkDeviceMemory deviceMemory, VkImageView imageView, VkDevice logicalDevice) :
                _type(Type::GARBAGE_IMAGE),
                _image(image),
                _deviceMemory(deviceMemory),
                _imageView(imageView),
                _logicalDevice(logicalDevice)
        {}

        Type _type;
        VkImage _image;
        VkDeviceMemory _deviceMemory;
        VkImageView _imageView;
        VkDevice _logicalDevice;
    };

    class Garbage final
    {
    private:
        Garbage() :
                g_data(Type::GARBAGE_EMPTY)
        {}

    public:
        Garbage(const GarbageDescriptorSet& garbage) :
                g_data(garbage)
        {}
        Garbage(const GarbageBuffer& garbage) :
                g_data(garbage)
        {}
        Garbage(const GarbageGraphicPipeline& garbage) :
                g_data(garbage)
        {}
        Garbage(const GarbageCommandPool& garbage) :
                g_data(garbage)
        {}
        Garbage(const GarbageFramebuffer& garbage) :
                g_data(garbage)
        {}
        Garbage(const GarbageRenderPass& garbage) :
                g_data(garbage)
        {}
        Garbage(const GarbageSampler& garbage) :
                g_data(garbage)
        {}
        Garbage(const GarbageImage& garbage) :
                g_data(garbage)
        {}
        Garbage(const Garbage& r) = delete;
        Garbage(Garbage&& r) noexcept :
                g_data(r.g_data)
        {
            r.g_data._type._type = Type::GARBAGE_EMPTY;
        }
        ~Garbage()
        {
            switch (this->g_data._type._type)
            {
            case Type::GARBAGE_DESCRIPTOR_SET:
                vkFreeDescriptorSets(this->g_data._descriptorSet._logicalDevice,
                                     this->g_data._descriptorSet._descriptorPool,
                                     1, &this->g_data._descriptorSet._descriptorSet);
                break;
            case Type::GARBAGE_VERTEX_BUFFER:
                vkDestroyBuffer(this->g_data._buffer._logicalDevice, this->g_data._buffer._buffer, nullptr);
                vkFreeMemory(this->g_data._buffer._logicalDevice, this->g_data._buffer._bufferMemory, nullptr);
                break;
            case Type::GARBAGE_GRAPHIC_PIPELINE:
                vkDestroyPipeline(this->g_data._graphicPipeline._logicalDevice, this->g_data._graphicPipeline._pipeline, nullptr);
                vkDestroyPipelineLayout(this->g_data._graphicPipeline._logicalDevice, this->g_data._graphicPipeline._pipelineLayout, nullptr);
                break;
            case Type::GARBAGE_COMMAND_POOL:
                vkDestroyCommandPool(this->g_data._commandPool._logicalDevice, this->g_data._commandPool._commandPool, nullptr);
                break;
            case Type::GARBAGE_FRAMEBUFFER:
                vkDestroyFramebuffer(this->g_data._framebuffer._logicalDevice, this->g_data._framebuffer._framebuffer, nullptr);
                break;
            case Type::GARBAGE_RENDERPASS:
                vkDestroyRenderPass(this->g_data._renderPass._logicalDevice, this->g_data._renderPass._renderPass, nullptr);
                break;
            case Type::GARBAGE_SAMPLER:
                vkDestroySampler(this->g_data._sampler._logicalDevice, this->g_data._sampler._sampler, nullptr);
                break;
            case Type::GARBAGE_IMAGE:
                vkDestroyImageView(this->g_data._image._logicalDevice, this->g_data._image._imageView, nullptr);
                vkDestroyImage(this->g_data._image._logicalDevice, this->g_data._image._image, nullptr);
                vkFreeMemory(this->g_data._image._logicalDevice, this->g_data._image._deviceMemory, nullptr);
                break;
            default:
                break;
            }
        }

    private:
        union Data
        {
            explicit Data(Type type) : _type{type} {}
            explicit Data(const GarbageDescriptorSet& data) : _descriptorSet{data} {}
            explicit Data(const GarbageBuffer& data) : _buffer{data} {}
            explicit Data(const GarbageGraphicPipeline& data) : _graphicPipeline{data} {}
            explicit Data(const GarbageCommandPool& data) : _commandPool{data} {}
            explicit Data(const GarbageFramebuffer& data) : _framebuffer{data} {}
            explicit Data(const GarbageRenderPass& data) : _renderPass{data} {}
            explicit Data(const GarbageSampler& data) : _sampler{data} {}
            explicit Data(const GarbageImage& data) : _image{data} {}

            GarbageType _type;
            GarbageDescriptorSet _descriptorSet;
            GarbageBuffer _buffer;
            GarbageGraphicPipeline _graphicPipeline;
            GarbageCommandPool _commandPool;
            GarbageFramebuffer _framebuffer;
            GarbageRenderPass _renderPass;
            GarbageSampler _sampler;
            GarbageImage _image;
        };

        Data g_data;
    };

    using ContainerType = std::vector<Garbage>;

    GarbageCollector() = default;
    GarbageCollector(const GarbageCollector& r) = delete;
    GarbageCollector(GarbageCollector&& r) noexcept = delete;
    ~GarbageCollector() = default;

    GarbageCollector& operator=(const GarbageCollector& r) = delete;
    GarbageCollector& operator=(GarbageCollector&& r) noexcept = delete;

    void setCurrentFrame(uint32_t frame);
    [[nodiscard]] uint32_t getCurrentFrame() const;
    void push(Garbage garbage) const;
    void free();
    void freeAll();

    void enable(bool stat);
    [[nodiscard]] bool isEnabled() const;

private:
    mutable std::array<ContainerType, FGE_MAX_FRAMES_IN_FLIGHT> g_containers;
    uint32_t g_currentFrame = 0;
    bool g_enabled = false;
};

}//end fge::vulkan

#endif //_FGE_VULKAN_C_GARBAGECOLLECTOR_HPP_INCLUDED
