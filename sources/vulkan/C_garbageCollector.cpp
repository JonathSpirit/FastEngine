/*
 * Copyright 2026 Guillaume Guillet
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

#include "FastEngine/vulkan/C_garbageCollector.hpp"
#include "FastEngine/vulkan/C_context.hpp"

namespace fge::vulkan
{

//Garbage

Garbage::~Garbage()
{
    switch (this->g_data._generic._type)
    {
    case GarbageType::GARBAGE_DESCRIPTOR_SET:
        vkFreeDescriptorSets(this->g_data._descriptorSet._logicalDevice, this->g_data._descriptorSet._descriptorPool, 1,
                             &this->g_data._descriptorSet._descriptorSet);
        break;
    case GarbageType::GARBAGE_VERTEX_BUFFER:
        vmaDestroyBuffer(this->g_data._buffer._allocator, this->g_data._buffer._bufferInfo._buffer,
                         this->g_data._buffer._bufferInfo._allocation);
        break;
    case GarbageType::GARBAGE_GRAPHIC_PIPELINE:
        vkDestroyPipeline(this->g_data._graphicPipeline._logicalDevice, this->g_data._graphicPipeline._pipeline,
                          nullptr);
        break;
    case GarbageType::GARBAGE_PIPELINE_LAYOUT:
        vkDestroyPipelineLayout(this->g_data._pipelineLayout._logicalDevice,
                                this->g_data._pipelineLayout._pipelineLayout, nullptr);
        break;
    case GarbageType::GARBAGE_COMMAND_POOL:
        vkDestroyCommandPool(this->g_data._commandPool._logicalDevice, this->g_data._commandPool._commandPool, nullptr);
        break;
    case GarbageType::GARBAGE_COMMAND_BUFFER:
        vkFreeCommandBuffers(this->g_data._commandBuffer._logicalDevice, this->g_data._commandBuffer._commandPool, 1,
                             &this->g_data._commandBuffer._commandBuffer);
        break;
    case GarbageType::GARBAGE_FRAMEBUFFER:
        vkDestroyFramebuffer(this->g_data._framebuffer._logicalDevice, this->g_data._framebuffer._framebuffer, nullptr);
        break;
    case GarbageType::GARBAGE_RENDERPASS:
        vkDestroyRenderPass(this->g_data._renderPass._logicalDevice, this->g_data._renderPass._renderPass, nullptr);
        break;
    case GarbageType::GARBAGE_SAMPLER:
        vkDestroySampler(this->g_data._sampler._logicalDevice, this->g_data._sampler._sampler, nullptr);
        break;
    case GarbageType::GARBAGE_IMAGE:
        vkDestroyImageView(this->g_data._image._context->getLogicalDevice().getDevice(), this->g_data._image._imageView,
                           nullptr);
        vmaDestroyImage(this->g_data._image._context->getAllocator(), this->g_data._image._image,
                        this->g_data._image._allocation);
        break;
    default:
        break;
    }
}

//GarbageCollector

GarbageCollector::GarbageCollector(GarbageCollector&& r) noexcept :
        g_containers(std::move(r.g_containers)),
        g_currentFrame(r.g_currentFrame),
        g_enabled(r.g_enabled)
{
    r.g_currentFrame = 0;
    r.g_enabled = false;
}

void GarbageCollector::setCurrentFrame(uint32_t frame)
{
    if (frame < FGE_MAX_FRAMES_IN_FLIGHT)
    {
        this->g_currentFrame = frame;
    }
    this->free();
}
uint32_t GarbageCollector::getCurrentFrame() const
{
    return this->g_currentFrame;
}
void GarbageCollector::push(Garbage garbage) const
{
    if (this->g_enabled)
    {
        this->g_containers[this->g_currentFrame].push_back(std::move(garbage));
    }
}
void GarbageCollector::free()
{
    this->g_containers[this->g_currentFrame].clear();
}
void GarbageCollector::freeAll()
{
    for (unsigned int i = 0; i < FGE_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        this->g_containers[i].clear();
    }
}

void GarbageCollector::enable(bool stat)
{
    this->g_enabled = stat;
    if (!stat)
    {
        this->freeAll();
    }
}
bool GarbageCollector::isEnabled() const
{
    return this->g_enabled;
}

} // namespace fge::vulkan