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

#ifndef _FGE_VULKAN_C_CONTEXT_HPP_INCLUDED
#define _FGE_VULKAN_C_CONTEXT_HPP_INCLUDED

#include "FastEngine/fge_extern.hpp"

#include "FastEngine/vulkan/vulkanGlobal.hpp"
#include <array>
#include <unordered_map>
#include <vector>

#include "FastEngine/vulkan/C_commandBuffer.hpp"
#include "FastEngine/vulkan/C_descriptorPool.hpp"
#include "FastEngine/vulkan/C_descriptorSetLayout.hpp"
#include "FastEngine/vulkan/C_garbageCollector.hpp"
#include "FastEngine/vulkan/C_graphicPipeline.hpp"
#include "FastEngine/vulkan/C_instance.hpp"
#include "FastEngine/vulkan/C_logicalDevice.hpp"
#include "FastEngine/vulkan/C_physicalDevice.hpp"
#include "FastEngine/vulkan/C_surface.hpp"
#include "FastEngine/vulkan/C_textureImage.hpp"
#include "FastEngine/vulkan/C_uniformBuffer.hpp"

///If set to 1, the stage flags of the bindings are checked to be the same as the shader stage flags
///If set to 0, VK_SHADER_STAGE_ALL is used for every binding
#define FGE_VULKAN_USE_STRICT_LAYOUT_BINDINGS_STAGE 0

#define FGE_VULKAN_TEXTURE_BINDING 0
#define FGE_VULKAN_TRANSFORM_BINDING 0
#define FGE_MULTIUSE_POOL_MAX_COMBINED_IMAGE_SAMPLER FGE_SHADER_MAX_BINDING_VARIABLE_DESCRIPTOR_COUNT

#define FGE_CONTEXT_OUTSIDE_RENDER_SCOPE_COMMAND_WAITSTAGE VK_PIPELINE_STAGE_VERTEX_INPUT_BIT

#define FGE_CONTEXT_GLOBALTRANSFORMS_COUNT_START 200

namespace fge
{

struct TransformUboData;
class RenderTarget;

} // namespace fge

namespace fge::vulkan
{

/**
 * \class Context
 * \brief Vulkan context
 * \ingroup vulkan
 *
 * This class is the main starting point for Vulkan usage.
 */
class FGE_API Context
{
public:
    enum class SubmitTypes
    {
        DIRECT_WAIT_EXECUTION, ///< The command buffer is submitted directly to the queue and vkQueueWaitIdle is called
        INDIRECT_EXECUTION ///< The command buffer is transferred to a queue in order to be submitted later and will always be executed before the rendering
    };

    class SubmitableCommandBuffer : public CommandBuffer
    {
    public:
        using CommandBuffer::CommandBuffer;

        [[nodiscard]] inline SubmitTypes getSubmitType() const { return g_submitType; }

    private:
        inline void setSubmitType(SubmitTypes type) { this->g_submitType = type; }

        SubmitTypes g_submitType;

        friend class Context;
    };

    struct GlobalTransform
    {
        GlobalTransform(vulkan::Context const& context);

        void init(vulkan::Context const& context);
        void update();

        vulkan::UniformBuffer _transforms;
        vulkan::DescriptorSet _descriptorSet;
        uint32_t _transformsCount;
        bool _needUpdate;
    };

    Context();
    /**
     * \brief Shortcut to initVulkan(surface)
     *
     * \see initVulkan()
     *
     * \param surface A valid Surface
     */
    explicit Context(Surface const& surface);
    Context(Context const& r) = delete;
    Context(Context&& r) noexcept = delete;
    ~Context();

    Context& operator=(Context const& r) = delete;
    Context& operator=(Context&& r) noexcept = delete;

    void destroy();

    /**
     * \brief Begin commands
     *
     * This return a command buffer that is ready to be used.
     *
     * The DIRECT_WAIT_EXECUTION type is used to execute a command buffer directly, this implies
     * create the buffer,
     * submit the buffer,
     * and waiting for the corresponding queue operations to be finished.
     * This is not ideal for performance.
     *
     * The INDIRECT_EXECUTION type will create a command buffer that will be submitted later and
     * executed with a semaphore that is signaled after every command is done so this assures
     * that every command are finished before graphics commands.
     * This is ideal for performance like copying staging buffers to device local buffers.
     *
     * On certain cases, a reusable command buffer is returned in order to optimize command buffer creation/destruction.
     * Current case is when the command buffer is used outside a render scope and the graphics queue is wanted.
     *
     * \warning This function must be pared with submitCommands().
     * CommandBuffer::begin(), CommandBuffer::end() and CommandBuffer::reset() should not be called.
     *
     * \param type The submit type of the command buffer
     * \param wantedRenderPassScope The wanted render pass scope (for optimization purposes)
     * \param wantedQueue The wanted queue (for optimization purposes)
     * \return A submitable command buffer
     */
    [[nodiscard]] SubmitableCommandBuffer beginCommands(SubmitTypes type,
                                                        CommandBuffer::RenderPassScopes wantedRenderPassScope,
                                                        CommandBuffer::SupportedQueueTypes_t wantedQueue) const;
    /**
     * \brief Submit commands
     *
     * \see beginCommands()
     *
     * \param buffer The command buffer to submit
     */
    bool submitCommands(SubmitableCommandBuffer&& buffer) const;

    /**
     * \brief Retrieve the semaphore that is signaled when all indirect command buffers have finished executing
     *
     * This can return VK_NULL_HANDLE if there is no command buffers to execute.
     *
     * \see submit()
     *
     * \return The semaphore
     */
    [[nodiscard]] VkSemaphore getIndirectSemaphore() const;
    /**
     * \brief Submit Context command buffers
     *
     * Indirect CommandBuffers are submitted with a semaphore that is signaled when the all of them have
     * finished executing.
     *
     * The semaphore should be retrieved with getIndirectSemaphore() and you must wait for it to be
     * signaled before rendering commands as this buffer generally contain some buffer transfer operations.
     *
     * This also increment the internal current frame counter.
     *
     * This is automatically called by a RenderScreen when the RenderScreen::display() method is called.
     */
    void submit() const;

    /**
     * \brief Helper to init SDL, volk and create an Instance
     *
     * This function do that in order :
     * - SDL_Init()
     * - SDL_Vulkan_LoadLibrary()
     * - Context::initVolk()
     * - and create an instance with 'applicationName'
     *
     * \param sdlFlag SDL flag passed to SDL_Init()
     * \param applicationName The name of the application
     * \param versionMajor
     * \param versionMinor
     * \param versionPatch
     * \return A valid Instance or throw on error
     */
    [[nodiscard]] static Instance init(uint32_t sdlFlag,
                                       std::string_view applicationName,
                                       uint16_t versionMajor = 1,
                                       uint16_t versionMinor = 0,
                                       uint16_t versionPatch = 0);

    /**
     * \brief Initialize Volk (Vulkan loader)
     *
     * \warning This function must be called once before any other graphics usage, generally at the start of the program
     */
    static void initVolk();

    /**
     * \brief Initialize Vulkan
     *
     * Once a surface is correctly created, this function must be called to initialize Vulkan.
     * Automatically call SetActiveContext() when no error.
     *
     * \param surface A valid surface
     */
    void initVulkan(Surface const& surface);
    void initVulkanSurfaceless(Instance const& instance);

    /**
     * \brief Enumerate to standard output the available extensions
     *
     * \see retrieveExtensions()
     */
    static void enumerateExtensions();
    /**
     * \brief Retrieve the available extensions
     *
     * \return The available extensions
     */
    [[nodiscard]] static std::vector<std::string> retrieveExtensions();

    /**
     * \brief Wait for the device to be idle
     *
     * This is generally called before any new commands submission.
     * Also when the program is about to exit, this function must be called to make sure that all commands are finished.
     */
    void waitIdle();

    [[nodiscard]] Instance const& getInstance() const;
    [[nodiscard]] Surface const& getSurface() const;
    [[nodiscard]] LogicalDevice const& getLogicalDevice() const;
    [[nodiscard]] PhysicalDevice const& getPhysicalDevice() const;

    /**
     * \brief Retrieve a command pool for graphics commands
     *
     * This command pool is used to create command buffers that will be used to submit commands to the graphics queue.
     *
     * This command pool is created with the following flags:
     * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
     *
     * \return The command pool
     */
    [[nodiscard]] VkCommandPool getGraphicsCommandPool() const;
    /**
     * \brief Allocate graphics command buffers
     *
     * This is a shortcut for vkAllocateCommandBuffers with the graphics command pool.
     *
     * \see getGraphicsCommandPool()
     *
     * \param level The level of the command buffers (primary or secondary)
     * \param commandBuffers An array of VkCommandBuffer structures in which the resulting command buffer objects are returned
     * \param commandBufferCount The number of command buffers to allocate
     */
    void allocateGraphicsCommandBuffers(VkCommandBufferLevel level,
                                        VkCommandBuffer commandBuffers[],
                                        uint32_t commandBufferCount) const;

    /**
     * \brief Retrieve a "multi-usage" descriptor pool
     *
     * This pool was created with the following types:
     * VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
     * VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
     * VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
     * VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
     *
     * \return The descriptor pool
     */
    [[nodiscard]] DescriptorPool const& getMultiUseDescriptorPool() const;

    /**
     * \brief Retrieve a "texture" descriptor set layout
     *
     * This layout is used with default provided shaders.
     *
     * This layout was created with the following:
     * binding: FGE_VULKAN_TEXTURE_BINDING
     * type: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
     * stage: VK_SHADER_STAGE_FRAGMENT_BIT
     *
     * \return The descriptor set layout
     */
    [[nodiscard]] DescriptorSetLayout const& getTextureLayout() const;
    /**
     * \brief Retrieve a "transform" descriptor set layout
     *
     * This layout is used with default provided shaders.
     *
     * This layout was created with the following:
     * binding: FGE_VULKAN_TRANSFORM_BINDING
     * type: VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
     * stage: VK_SHADER_STAGE_VERTEX_BIT
     *
     * \return The descriptor set layout
     */
    [[nodiscard]] DescriptorSetLayout const& getTransformLayout() const;
    /**
     * \brief Retrieve a "texture" descriptor pool
     *
     * This pool can only contain VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER type.
     *
     * \return The descriptor pool
     */
    [[nodiscard]] DescriptorPool const& getTextureDescriptorPool() const;
    /**
     * \brief Retrieve a "transform" descriptor pool
     *
     * This pool can only contain VK_DESCRIPTOR_TYPE_STORAGE_BUFFER type.
     *
     * \return The descriptor pool
     */
    [[nodiscard]] DescriptorPool const& getTransformDescriptorPool() const;

    /**
     * \brief Retrieve the VMA (Vulkan Memory Allocator)
     *
     * \return The allocator
     */
    [[nodiscard]] VmaAllocator getAllocator() const;

    [[nodiscard]] std::optional<BufferInfo> createBuffer(VkDeviceSize size,
                                                         VkBufferUsageFlags usage,
                                                         VmaAllocationCreateFlags flags,
                                                         VkMemoryPropertyFlags requiredProperties = 0) const;
    [[nodiscard]] std::optional<ImageInfo> createImage(uint32_t width,
                                                       uint32_t height,
                                                       VkFormat format,
                                                       VkImageTiling tiling,
                                                       uint32_t mipLevels,
                                                       VkImageUsageFlags usage,
                                                       VmaAllocationCreateFlags flags,
                                                       VkMemoryPropertyFlags requiredProperties = 0) const;

    /**
     * \brief Push a graphics command buffer to a list
     *
     * This is used to keep track of submitable command buffers that will be submitted to the graphics queue.
     * This list must be cleared once the command buffers are submitted. Generally, this is done by a RenderScreen
     * when the RenderScreen::display() method is called.
     *
     * \param commandBuffer The command buffer to push
     */
    void pushGraphicsCommandBuffer(VkCommandBuffer commandBuffer) const;
    /**
     * \brief Retrieve the list of submitable graphics command buffers
     *
     * \see pushGraphicsCommandBuffer()
     *
     * \return The list of executable graphics command buffers
     */
    [[nodiscard]] std::vector<VkCommandBuffer> const& getGraphicsCommandBuffers() const;
    /**
     * \brief Clear the list of submitable graphics command buffers
     *
     * \see pushGraphicsCommandBuffer()
     */
    void clearGraphicsCommandBuffers() const;

    void startMainRenderTarget(RenderTarget& renderTarget) const;
    [[nodiscard]] RenderTarget* getMainRenderTarget() const;
    [[nodiscard]] bool isMainRenderTarget(RenderTarget const& renderTarget) const;
    void endMainRenderTarget(RenderTarget const& renderTarget) const;

    [[nodiscard]] GlobalTransform const& getGlobalTransform() const;
    [[nodiscard]] fge::TransformUboData const* getGlobalTransform(uint32_t index) const;
    [[nodiscard]] std::pair<uint32_t, fge::TransformUboData*> requestGlobalTransform() const;

    void clearLayoutPipelineCache() const;
    /**
     * \brief Retrieve a layout pipeline
     *
     * This function will create a new layout pipeline if it doesn't exist.
     * The layout pipeline is created with the provided shaders with reflection of
     * the SPIR-V code to retrieve the descriptor set layouts and push constant ranges.
     *
     * \param vertexShader The vertex shader, can be \b nullptr
     * \param geometryShader The geometry shader, can be \b nullptr
     * \param fragmentShader The fragment shader, can be \b nullptr
     * \return The layout pipeline
     */
    [[nodiscard]] LayoutPipeline&
    requestLayoutPipeline(Shader const* vertexShader, Shader const* geometryShader, Shader const* fragmentShader) const;
    void clearDescriptorLayoutCache() const;
    /**
     * \brief Retrieve a descriptor set layout
     *
     * This function will create a new descriptor set layout if it doesn't exist.
     * The descriptor set layout is created with the provided shader with reflection of
     * the SPIR-V code to retrieve the descriptor set layouts.
     *
     * \param vertexShader The vertex shader, can be \b nullptr
     * \param geometryShader The geometry shader, can be \b nullptr
     * \param fragmentShader The fragment shader, can be \b nullptr
     * \return The descriptor set layout or \b nullptr if the shader is invalid
     */
    [[nodiscard]] std::vector<DescriptorSetLayout> const* requestDescriptorLayout(Shader const* vertexShader,
                                                                                  Shader const* geometryShader,
                                                                                  Shader const* fragmentShader) const;
    [[nodiscard]] std::vector<DescriptorSetLayout> const* requestDescriptorLayout(Shader const* shader) const;

    /**
     * \brief Helper to request a descriptor set
     *
     * This function will create a new descriptor set with the provided shader and set index.
     * This will also select for you the correct descriptor pool to use.
     *
     * \param shaderName The name of the shader from the manager
     * \param setIndex The index of the wanted set in the shader (not the set number)
     * \param variableElements The number of elements in the descriptor set (for variable descriptor sets)
     * \return The descriptor set or \b std::nullopt if the something went wrong
     */
    [[nodiscard]] std::optional<DescriptorSet>
    createDescriptorSet(std::string_view shaderName, uint32_t setIndex, uint32_t variableElements = 0) const;

    GarbageCollector _garbageCollector;

private:
    void createCommandPool();
    void createMultiUseDescriptorPool();
    void createTextureDescriptorPool();
    void createTransformDescriptorPool();
    void createSyncObjects();

    mutable GlobalTransform g_globalTransform;

    struct ReusableCommandBuffer
    {
        constexpr ReusableCommandBuffer() = default;
        constexpr ReusableCommandBuffer(VkCommandBuffer commandBuffer, bool isRecording) :
                _commandBuffer(commandBuffer),
                _isRecording(isRecording)
        {}

        VkCommandBuffer _commandBuffer{VK_NULL_HANDLE};
        bool _isRecording{false};
    };

    PhysicalDevice g_physicalDevice;
    LogicalDevice g_logicalDevice;
    Surface const* g_surface;
    Instance const* g_instance;

    mutable std::unordered_map<LayoutPipeline::Key,
                               std::vector<DescriptorSetLayout>,
                               LayoutPipeline::Key::Hash,
                               LayoutPipeline::Key::Compare>
            g_cacheDescriptorLayouts;
    mutable std::
            unordered_map<LayoutPipeline::Key, LayoutPipeline, LayoutPipeline::Key::Hash, LayoutPipeline::Key::Compare>
                    g_cachePipelineLayouts;
    DescriptorPool g_multiUseDescriptorPool;

    DescriptorSetLayout g_textureLayout;
    DescriptorSetLayout g_transformLayout;
    DescriptorPool g_textureDescriptorPool;
    DescriptorPool g_transformDescriptorPool;

    mutable RenderTarget* g_mainRenderTarget;

    mutable VmaAllocator g_allocator;

    mutable uint32_t g_currentFrame;

    mutable std::vector<VkCommandBuffer> g_graphicsSubmitableCommandBuffers;

    std::array<VkSemaphore, FGE_MAX_FRAMES_IN_FLIGHT> g_indirectFinishedSemaphores{};
    mutable std::array<std::vector<CommandBuffer>, FGE_MAX_FRAMES_IN_FLIGHT> g_indirectSubmitableCommandBuffers{};
    mutable std::array<ReusableCommandBuffer, FGE_MAX_FRAMES_IN_FLIGHT>
            g_indirectOutsideRenderScopeGraphicsSubmitableCommandBuffers{};

    VkCommandPool g_graphicsCommandPool;
    bool g_isCreated;
};

} // namespace fge::vulkan

#endif //_FGE_VULKAN_C_CONTEXT_HPP_INCLUDED
