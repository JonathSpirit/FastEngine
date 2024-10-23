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

#include "FastEngine/vulkan/C_context.hpp"
#include "FastEngine/C_alloca.hpp"
#include "FastEngine/graphic/C_transform.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"
#include <iostream>
#include <optional>
#include <vector>

//https://docs.tizen.org/application/native/guides/graphics/vulkan/

//https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules

namespace fge::vulkan
{

Context::Context() :
        g_globalTransform(*this),
        g_surface(nullptr),
        g_multiUseDescriptorPool(*this),
        g_textureLayout(*this),
        g_transformLayout(*this),
        g_textureDescriptorPool(*this),
        g_transformDescriptorPool(*this),
        g_mainRenderTarget(nullptr),
        g_allocator(),
        g_currentFrame(0),
        g_graphicsCommandPool(VK_NULL_HANDLE),
        g_isCreated(false)
{
    this->g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers.fill({VK_NULL_HANDLE, false});
    this->g_indirectFinishedSemaphores.fill(VK_NULL_HANDLE);
}
Context::Context(Surface const& surface) :
        Context()
{
    this->initVulkan(surface);
}
Context::~Context()
{
    this->destroy();
}

void Context::destroy()
{
    if (this->g_isCreated)
    {
        this->g_globalTransform._transforms.destroy();
        this->g_globalTransform._descriptorSet.destroy();

        this->_garbageCollector.enable(false);

        for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(this->g_logicalDevice.getDevice(), this->g_indirectFinishedSemaphores[i], nullptr);
        }

        this->g_cacheLayouts.clear();
        this->g_textureLayout.destroy();
        this->g_transformLayout.destroy();

        this->g_multiUseDescriptorPool.destroy();
        this->g_textureDescriptorPool.destroy();
        this->g_transformDescriptorPool.destroy();

        vkDestroyCommandPool(this->g_logicalDevice.getDevice(), this->g_graphicsCommandPool, nullptr);

        vmaDestroyAllocator(this->g_allocator);

        this->g_surface = nullptr;
        this->g_logicalDevice.destroy();

        this->g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers.fill({VK_NULL_HANDLE, false});
        this->g_indirectFinishedSemaphores.fill(VK_NULL_HANDLE);
        this->g_currentFrame = 0;

        this->g_graphicsCommandPool = VK_NULL_HANDLE;
        this->g_isCreated = false;
    }
}

Context::SubmitableCommandBuffer Context::beginCommands(SubmitTypes type,
                                                        CommandBuffer::RenderPassScopes wantedRenderPassScope,
                                                        CommandBuffer::SupportedQueueTypes_t wantedQueue) const
{
    if (wantedRenderPassScope == CommandBuffer::RenderPassScopes::OUTSIDE &&
        (wantedQueue & CommandBuffer::SUPPORTED_QUEUE_GRAPHICS) > 0 && type == SubmitTypes::INDIRECT_EXECUTION)
    {
        auto& buffer = this->g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers[this->g_currentFrame];

        if (!buffer._isRecording)
        {
            vkResetCommandBuffer(buffer._commandBuffer, 0);
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(buffer._commandBuffer, &beginInfo);

            buffer._isRecording = true;
        }

        SubmitableCommandBuffer submitBuffer{*this, VK_COMMAND_BUFFER_LEVEL_PRIMARY, buffer._commandBuffer,
                                             VK_NULL_HANDLE};
        submitBuffer.setSubmitType(type);
        submitBuffer.forceSupportedQueues(CommandBuffer::SUPPORTED_QUEUE_GRAPHICS);
        submitBuffer.forceRenderPassScope(CommandBuffer::RenderPassScopes::OUTSIDE);
        return submitBuffer;
    }

    SubmitableCommandBuffer submitBuffer{*this, VK_COMMAND_BUFFER_LEVEL_PRIMARY, this->g_graphicsCommandPool};
    submitBuffer.setSubmitType(type);
    submitBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    return submitBuffer;
}
bool Context::submitCommands(SubmitableCommandBuffer&& buffer) const
{
    if (buffer.get() == VK_NULL_HANDLE)
    {
        return false;
    }

    switch (buffer.getSubmitType())
    {
    case SubmitTypes::DIRECT_WAIT_EXECUTION:
    {
        if (!buffer.isEnded())
        {
            buffer.end();
        }

        VkQueue queue = VK_NULL_HANDLE;
        if ((buffer.getSupportedQueues() & CommandBuffer::SUPPORTED_QUEUE_GRAPHICS) > 0)
        {
            queue = this->g_logicalDevice.getGraphicQueue();
        }
        else if ((buffer.getSupportedQueues() & CommandBuffer::SUPPORTED_QUEUE_COMPUTE) > 0)
        {
            queue = this->g_logicalDevice.getComputeQueue();
        }
        else if ((buffer.getSupportedQueues() & CommandBuffer::SUPPORTED_QUEUE_TRANSFER) > 0)
        {
            queue = this->g_logicalDevice.getTransferQueue();
        }

        if (queue == VK_NULL_HANDLE)
        {
            break;
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffer.getPtr();

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
    }
    break;
    case SubmitTypes::INDIRECT_EXECUTION:
        if (buffer.get() ==
            this->g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers[this->g_currentFrame]._commandBuffer)
        {
            break;
        }
        if (!buffer.isEnded())
        {
            buffer.end();
        }
        this->g_indirectSubmitableCommandBuffers[this->g_currentFrame].push_back(std::move(buffer));
        break;
    }

    return true;
}

VkSemaphore Context::getIndirectSemaphore() const
{
    return this->g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers[this->g_currentFrame]._isRecording ||
                           !this->g_indirectSubmitableCommandBuffers[this->g_currentFrame].empty()
                   ? this->g_indirectFinishedSemaphores[this->g_currentFrame]
                   : VK_NULL_HANDLE;
}
void Context::submit() const
{
    auto& indirectSubmitableCommandBuffer = this->g_indirectSubmitableCommandBuffers[this->g_currentFrame];
    auto& indirectOutsideRenderScopeGraphicsSubmitableCommandBuffer =
            this->g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers[this->g_currentFrame];

    if (indirectSubmitableCommandBuffer.empty() &&
        !indirectOutsideRenderScopeGraphicsSubmitableCommandBuffer._isRecording)
    {
        return;
    }

    VkCommandBuffer* commandBuffers = FGE_ALLOCA_T(VkCommandBuffer, indirectSubmitableCommandBuffer.size() + 1);
    uint32_t commandBufferCount = 0;

    if (indirectOutsideRenderScopeGraphicsSubmitableCommandBuffer._isRecording)
    {
        if (vkEndCommandBuffer(indirectOutsideRenderScopeGraphicsSubmitableCommandBuffer._commandBuffer) != VK_SUCCESS)
        {
            throw fge::Exception("failed to record command buffer!");
        }

        commandBuffers[commandBufferCount++] = indirectOutsideRenderScopeGraphicsSubmitableCommandBuffer._commandBuffer;
        indirectOutsideRenderScopeGraphicsSubmitableCommandBuffer._isRecording = false;
    }

    for (auto const& buffer: indirectSubmitableCommandBuffer)
    {
        if ((buffer.getSupportedQueues() & CommandBuffer::SUPPORTED_QUEUE_GRAPHICS) == 0)
        { ///TODO: Only supporting graphics queue for now
            continue;
        }
        commandBuffers[commandBufferCount++] = buffer.get();
    }

    if (commandBufferCount == 0)
    {
        indirectSubmitableCommandBuffer.clear();
        return;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;

    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = commandBuffers;

    VkSemaphore signalSemaphores[] = {this->g_indirectFinishedSemaphores[this->g_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(this->g_logicalDevice.getGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw fge::Exception("failed to submit indirect command buffers!");
    }

    indirectSubmitableCommandBuffer.clear();

    this->g_currentFrame = (this->g_currentFrame + 1) % FGE_MAX_FRAMES_IN_FLIGHT;
}

Instance Context::init(uint32_t sdlFlag,
                       std::string_view applicationName,
                       uint16_t versionMajor,
                       uint16_t versionMinor,
                       uint16_t versionPatch)
{
    if (SDL_Init(sdlFlag) != 0)
    {
        throw fge::Exception("failed to initialize SDL!");
    }
    if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
    {
        throw fge::Exception("failed to load sdl Vulkan library!");
    }

    Context::initVolk();

    return Instance(applicationName, versionMajor, versionMinor, versionPatch);
}

void Context::initVolk()
{
    auto result = volkInitialize();
    if (result != VK_SUCCESS)
    {
        throw fge::Exception{"Can't init volk!"};
    }
}

void Context::initVulkan(Surface const& surface)
{
    this->g_surface = &surface;
    this->g_instance = &this->g_surface->getInstance();
    auto physicalDevice = this->g_instance->pickPhysicalDevice(this->g_surface->get());
    if (!physicalDevice.has_value())
    {
        throw fge::Exception("failed to find a suitable GPU!");
    }
    this->g_physicalDevice = std::move(physicalDevice.value());
    this->g_logicalDevice.create(this->g_physicalDevice, this->g_surface->get());

    VmaVulkanFunctions vulkanFunctions{vkGetInstanceProcAddr,
                                       vkGetDeviceProcAddr,
                                       vkGetPhysicalDeviceProperties,
                                       vkGetPhysicalDeviceMemoryProperties,
                                       vkAllocateMemory,
                                       vkFreeMemory,
                                       vkMapMemory,
                                       vkUnmapMemory,
                                       vkFlushMappedMemoryRanges,
                                       vkInvalidateMappedMemoryRanges,
                                       vkBindBufferMemory,
                                       vkBindImageMemory,
                                       vkGetBufferMemoryRequirements,
                                       vkGetImageMemoryRequirements,
                                       vkCreateBuffer,
                                       vkDestroyBuffer,
                                       vkCreateImage,
                                       vkDestroyImage,
                                       vkCmdCopyBuffer,
                                       vkGetBufferMemoryRequirements2,
                                       vkGetImageMemoryRequirements2,
                                       vkBindBufferMemory2,
                                       vkBindImageMemory2,
                                       vkGetPhysicalDeviceMemoryProperties2,
                                       vkGetDeviceBufferMemoryRequirements,
                                       vkGetDeviceImageMemoryRequirements};

    VmaAllocatorCreateInfo allocatorCreateInfo{0,
                                               this->g_physicalDevice.getDevice(),
                                               this->g_logicalDevice.getDevice(),
                                               0,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               &vulkanFunctions,
                                               this->g_instance->get(),
                                               VK_API_VERSION_1_1,
                                               nullptr};

    if (vmaCreateAllocator(&allocatorCreateInfo, &this->g_allocator) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create allocator!");
    }

    this->createCommandPool();
    this->createMultiUseDescriptorPool();
    this->createTextureDescriptorPool();
    this->createTransformDescriptorPool();
    this->createSyncObjects();

    for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        this->allocateGraphicsCommandBuffers(
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                &this->g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers[i]._commandBuffer, 1);
    }

    this->g_textureLayout.create({DescriptorSetLayout::Binding(
            FGE_VULKAN_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)});
    this->g_transformLayout.create({DescriptorSetLayout::Binding(
            FGE_VULKAN_TRANSFORM_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)});

    this->g_globalTransform.init(*this);
    this->g_isCreated = true;

    SetActiveContext(*this);
}
void Context::initVulkanSurfaceless(Instance const& instance)
{
    this->g_surface = nullptr;
    this->g_instance = &instance;
    auto physicalDevice = this->g_instance->pickPhysicalDevice(VK_NULL_HANDLE);
    if (!physicalDevice.has_value())
    {
        throw fge::Exception("failed to find a suitable GPU!");
    }
    this->g_physicalDevice = std::move(physicalDevice.value());
    this->g_logicalDevice.create(this->g_physicalDevice, VK_NULL_HANDLE);

    VmaVulkanFunctions vulkanFunctions{vkGetInstanceProcAddr,
                                       vkGetDeviceProcAddr,
                                       vkGetPhysicalDeviceProperties,
                                       vkGetPhysicalDeviceMemoryProperties,
                                       vkAllocateMemory,
                                       vkFreeMemory,
                                       vkMapMemory,
                                       vkUnmapMemory,
                                       vkFlushMappedMemoryRanges,
                                       vkInvalidateMappedMemoryRanges,
                                       vkBindBufferMemory,
                                       vkBindImageMemory,
                                       vkGetBufferMemoryRequirements,
                                       vkGetImageMemoryRequirements,
                                       vkCreateBuffer,
                                       vkDestroyBuffer,
                                       vkCreateImage,
                                       vkDestroyImage,
                                       vkCmdCopyBuffer,
                                       vkGetBufferMemoryRequirements2,
                                       vkGetImageMemoryRequirements2,
                                       vkBindBufferMemory2,
                                       vkBindImageMemory2,
                                       vkGetPhysicalDeviceMemoryProperties2,
                                       vkGetDeviceBufferMemoryRequirements,
                                       vkGetDeviceImageMemoryRequirements};

    VmaAllocatorCreateInfo allocatorCreateInfo{0,
                                               this->g_physicalDevice.getDevice(),
                                               this->g_logicalDevice.getDevice(),
                                               0,
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               &vulkanFunctions,
                                               this->g_instance->get(),
                                               VK_API_VERSION_1_1,
                                               nullptr};

    if (vmaCreateAllocator(&allocatorCreateInfo, &this->g_allocator) != VK_SUCCESS)
    {
        throw fge::Exception("failed to create allocator!");
    }

    this->createCommandPool();
    this->createMultiUseDescriptorPool();
    this->createTextureDescriptorPool();
    this->createTransformDescriptorPool();
    this->createSyncObjects();

    for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        this->allocateGraphicsCommandBuffers(
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                &this->g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers[i]._commandBuffer, 1);
    }

    this->g_textureLayout.create({DescriptorSetLayout::Binding(
            FGE_VULKAN_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)});
    this->g_transformLayout.create({DescriptorSetLayout::Binding(
            FGE_VULKAN_TRANSFORM_BINDING, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)});

    this->g_globalTransform.init(*this);
    this->g_isCreated = true;

    SetActiveContext(*this);
}
void Context::enumerateExtensions()
{
    auto extensions = Context::retrieveExtensions();

    std::cout << "available extensions:\n";

    for (auto const& extension: extensions)
    {
        std::cout << '\t' << extension << '\n';
    }
    std::cout << std::flush;
}
std::vector<std::string> Context::retrieveExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::vector<std::string> result;
    result.reserve(extensionCount);

    for (auto const& extension: extensions)
    {
        result.emplace_back(extension.extensionName);
    }

    return result;
}

void Context::waitIdle()
{
    vkDeviceWaitIdle(this->g_logicalDevice.getDevice());
}

Instance const& Context::getInstance() const
{
    return *this->g_instance;
}
Surface const& Context::getSurface() const
{
    return *this->g_surface;
}
LogicalDevice const& Context::getLogicalDevice() const
{
    return this->g_logicalDevice;
}
PhysicalDevice const& Context::getPhysicalDevice() const
{
    return this->g_physicalDevice;
}

VkCommandPool Context::getGraphicsCommandPool() const
{
    return this->g_graphicsCommandPool;
}
void Context::allocateGraphicsCommandBuffers(VkCommandBufferLevel level,
                                             VkCommandBuffer commandBuffers[],
                                             uint32_t commandBufferCount) const
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = this->g_graphicsCommandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = commandBufferCount;

    if (vkAllocateCommandBuffers(this->g_logicalDevice.getDevice(), &allocInfo, commandBuffers) != VK_SUCCESS)
    {
        throw fge::Exception("failed to allocate command buffers!");
    }
}

fge::vulkan::DescriptorSetLayout& Context::getCacheLayout(std::string_view key) const
{
    auto it = this->g_cacheLayouts.find(key);
    if (it != this->g_cacheLayouts.end())
    {
        return it->second;
    }
    return this->g_cacheLayouts
            .emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(*this))
            .first->second;
}
DescriptorPool const& Context::getMultiUseDescriptorPool() const
{
    return this->g_multiUseDescriptorPool;
}

fge::vulkan::DescriptorSetLayout const& Context::getTextureLayout() const
{
    return this->g_textureLayout;
}
fge::vulkan::DescriptorSetLayout const& Context::getTransformLayout() const
{
    return this->g_transformLayout;
}
DescriptorPool const& Context::getTextureDescriptorPool() const
{
    return this->g_textureDescriptorPool;
}
DescriptorPool const& Context::getTransformDescriptorPool() const
{
    return this->g_transformDescriptorPool;
}

VmaAllocator Context::getAllocator() const
{
    return this->g_allocator;
}

void Context::pushGraphicsCommandBuffer(VkCommandBuffer commandBuffer) const
{
    this->g_graphicsSubmitableCommandBuffers.push_back(commandBuffer);
}
std::vector<VkCommandBuffer> const& Context::getGraphicsCommandBuffers() const
{
    return this->g_graphicsSubmitableCommandBuffers;
}
void Context::clearGraphicsCommandBuffers() const
{
    this->g_graphicsSubmitableCommandBuffers.clear();
}

void Context::startMainRenderTarget(RenderTarget& renderTarget) const
{
    if (this->g_mainRenderTarget == nullptr)
    {
        this->g_mainRenderTarget = &renderTarget;
        this->g_globalTransform._transformsCount = 0;
        this->g_globalTransform.update();
    }
}
RenderTarget* Context::getMainRenderTarget() const
{
    return this->g_mainRenderTarget;
}
bool Context::isMainRenderTarget(RenderTarget const& renderTarget) const
{
    return this->g_mainRenderTarget == &renderTarget;
}
void Context::endMainRenderTarget(RenderTarget const& renderTarget) const
{
    if (this->g_mainRenderTarget == &renderTarget)
    {
        this->g_mainRenderTarget = nullptr;
    }
}

Context::GlobalTransform const& Context::getGlobalTransform() const
{
    return this->g_globalTransform;
}
fge::TransformUboData const* Context::getGlobalTransform(uint32_t index) const
{
    if (index >= this->g_globalTransform._transformsCount)
    {
        return nullptr;
    }
    return static_cast<fge::TransformUboData*>(this->g_globalTransform._transforms.getBufferMapped()) + index;
}
std::pair<uint32_t, fge::TransformUboData*> Context::requestGlobalTransform() const
{
    auto const index = this->g_globalTransform._transformsCount++;

    auto const maxSize = this->g_globalTransform._transforms.getBufferSize() / fge::TransformUboData::uboSize;
    if (this->g_globalTransform._transformsCount >= maxSize)
    {
        this->g_globalTransform._transforms.resize(fge::TransformUboData::uboSize * maxSize * 2);
        this->g_globalTransform._needUpdate = true;
    }

    return {index, static_cast<fge::TransformUboData*>(this->g_globalTransform._transforms.getBufferMapped()) + index};
}

Context::GlobalTransform::GlobalTransform(vulkan::Context const& context) :
        _transforms(context, vulkan::UniformBuffer::Types::STORAGE_BUFFER),
        _transformsCount(0),
        _needUpdate(false)
{}

void Context::GlobalTransform::init(vulkan::Context const& context)
{
    this->_transformsCount = 0;
    this->_needUpdate = false;

    this->_transforms.create(FGE_CONTEXT_GLOBALTRANSFORMS_COUNT_START * fge::TransformUboData::uboSize,
                             vulkan::UniformBuffer::Types::STORAGE_BUFFER);

    this->_descriptorSet = context.getTransformDescriptorPool()
                                   .allocateDescriptorSet(context.getTransformLayout().getLayout())
                                   .value();

    vulkan::DescriptorSet::Descriptor const descriptor(this->_transforms, FGE_VULKAN_TRANSFORM_BINDING,
                                                       vulkan::DescriptorSet::Descriptor::BufferTypes::STORAGE,
                                                       VK_WHOLE_SIZE);
    this->_descriptorSet.updateDescriptorSet(&descriptor, 1);
}
void Context::GlobalTransform::update()
{
    if (this->_needUpdate)
    {
        this->_needUpdate = false;

        //TODO
        //Can cause flickering as descriptor set can't be updated while in use (whitout extension)
        auto const* context = this->_descriptorSet.getContext();
        this->_descriptorSet = context->getTransformDescriptorPool()
                                       .allocateDescriptorSet(context->getTransformLayout().getLayout())
                                       .value();

        vulkan::DescriptorSet::Descriptor const descriptor(this->_transforms, FGE_VULKAN_TRANSFORM_BINDING,
                                                           vulkan::DescriptorSet::Descriptor::BufferTypes::STORAGE,
                                                           VK_WHOLE_SIZE);
        this->_descriptorSet.updateDescriptorSet(&descriptor, 1);
    }
}

void Context::createCommandPool()
{
    auto queueFamilyIndices = this->g_physicalDevice.findQueueFamilies(
            this->g_surface == nullptr
                    ? VK_NULL_HANDLE
                    : this->g_surface->get()); //TODO: do not do that, it's already done in logicalDevice

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices._graphicsFamily.value();

    if (vkCreateCommandPool(this->g_logicalDevice.getDevice(), &poolInfo, nullptr, &this->g_graphicsCommandPool) !=
        VK_SUCCESS)
    {
        throw fge::Exception("failed to create command pool!");
    }
}
void Context::createMultiUseDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(4);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = FGE_MULTIUSE_POOL_MAX_COMBINED_IMAGE_SAMPLER;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 1;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = 1;

    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[3].descriptorCount = 1;

    this->g_multiUseDescriptorPool.create(std::move(poolSizes), 128, false, true);
}
void Context::createTextureDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(1);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 1;

    this->g_textureDescriptorPool.create(std::move(poolSizes), 128, false, true);
}
void Context::createTransformDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(1);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 1;

    this->g_transformDescriptorPool.create(std::move(poolSizes), 128, false, true);
}
void Context::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (std::size_t i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(this->g_logicalDevice.getDevice(), &semaphoreInfo, nullptr,
                              &this->g_indirectFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw fge::Exception("failed to create semaphores!");
        }
    }
}

} // namespace fge::vulkan