#include "pch.hpp"

#define VMA_IMPLEMENTATION
#define VMA_DEBUG_LOG_FORMAT
#include <vk_mem_alloc.h>

#include "render_device/bufferDescriptor.h"
#include "render_device/renderPipeline.h"

#include "vulkan/utils.h"
#include "vulkan/vulkanDevice.h"
#include "vulkan/vulkanPipeline.h"

namespace RED
{
namespace vulkan
{
VulkanDevice::VulkanDevice(void* deviceInfo, void* queues, void* callbacks)
{
    assert(deviceInfo);
    assert(queues);

    auto vulkanDeviceInfo = reinterpret_cast<DeviceInfo*>(deviceInfo);
    auto vulkanQueues     = reinterpret_cast<QueueInfo*>(queues);
    auto vulkanCallbacks  = reinterpret_cast<DeviceCallbacks*>(callbacks);

    m_device         = vulkanDeviceInfo->device;
    m_physicalDevice = vulkanDeviceInfo->physicalDevice;

    m_rendererContext      = vulkanCallbacks->context;
    m_beginFrame_fn        = vulkanCallbacks->BeginFrame_fn;
    m_endFrame_fn          = vulkanCallbacks->EndFrame_fn;
    m_getSwapchainState_fn = vulkanCallbacks->GetSwapchainState_fn;

    /*
      Queues
      1 Graphics
      2 Present - used by renderer, not here
      3 Compute
    */
    m_queues[static_cast<u32>(QueueType::GRAPHICS)] = vulkanQueues[0];
    m_queues[static_cast<u32>(QueueType::COMPUTE)]  = vulkanQueues[2];

    VmaAllocatorCreateInfo allocator_info{
      .flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = m_physicalDevice,
      .device         = m_device,
      .instance       = vulkanDeviceInfo->instance,
    };

    vmaCreateAllocator(&allocator_info, &m_allocator);

    m_immediateCommandPool =
      CreateCommandPool(m_queues[static_cast<u32>(QueueType::GRAPHICS)].index);

    for (auto& commandBufferSet : m_commandBufferSets)
    {
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            commandBufferSet.commandPools.at(i) =
              CreateCommandPool(m_queues[static_cast<u32>(QueueType::GRAPHICS)].index);
            commandBufferSet.commandBufferIndex.at(i) = -1;
        }
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_frameCompletedFences.at(i)      = VK_NULL_HANDLE;
        m_imagesAvailableSemaphores.at(i) = CreateSemaphore();
    }

    // TODO Temporal
    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;
    assert(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler) == VK_SUCCESS);
}

VulkanDevice::~VulkanDevice()
{
    BeginCommandRecording(QueueType::GRAPHICS);
    UpdateResources();
    EndCommandRecording(false);

    vkDeviceWaitIdle(m_device);

    DeleteResources();

    for (auto& pipeline : m_pipelines)
        pipeline.second.Destroy(m_device);

    for (auto& buffer : m_buffers)
        vmaDestroyBuffer(m_allocator, buffer.second.m_buffer, buffer.second.m_allocation);

    for (auto& texture : m_textures)
        vmaDestroyImage(m_allocator, texture.second.image, texture.second.allocation);

    m_pipelines.clear();
    m_buffers.clear();
    m_textures.clear();

    vkDestroySampler(m_device, m_sampler, nullptr);

    m_descriptorSetManager.OnDestroy(m_device);

    for (auto& stagingBuffer : m_stagingBuffers)
    {
        vmaDestroyBuffer(m_allocator, stagingBuffer.buffer, stagingBuffer.allocation);
    }

    m_stagingBuffers.clear();

    for (auto& commandBufferSet : m_commandBufferSets)
    {
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroyCommandPool(m_device, commandBufferSet.commandPools.at(i), nullptr);
            auto& commandBuffers = commandBufferSet.commandBuffers.at(i);

            for (auto& cmd : commandBuffers)
            {
                vkDestroySemaphore(m_device, cmd->signalSemaphore, nullptr);
            }
            commandBuffers.clear();
        }
    }

    vkDestroyCommandPool(m_device, m_immediateCommandPool, nullptr);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroyFence(m_device, m_frameCompletedFences[i], nullptr);
        vkDestroySemaphore(m_device, m_imagesAvailableSemaphores[i], nullptr);
    }

    vmaDestroyAllocator(m_allocator);
    m_allocator = nullptr;

    if (m_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    m_physicalDevice  = VK_NULL_HANDLE;
    m_rendererContext = nullptr;
}

void* VulkanDevice::GetCurrentCommandBuffer() const
{
    return m_currentCommandBuffer->commandBuffer;
}

void VulkanDevice::BeginFrame()
{
    m_currentIndexFrame = ++m_currentIndexFrame % MAX_FRAMES_IN_FLIGHT;

    if (m_frameCompletedFences.at(m_currentIndexFrame) == VK_NULL_HANDLE)
    {
        m_frameCompletedFences.at(m_currentIndexFrame) = CreateFence();
    }
    else
    {
        auto ok = vkWaitForFences(m_device,
                                  1,
                                  &m_frameCompletedFences.at(m_currentIndexFrame),
                                  VK_TRUE,
                                  UINT64_MAX);
        assert(ok == VK_SUCCESS);
    }

    for (auto& commandBufferSet : m_commandBufferSets)
    {
        commandBufferSet.commandBufferIndex.at(m_currentIndexFrame) = -1;
    }

    for (auto& stagingBuffer : m_stagingBuffers)
    {
        stagingBuffer.memory = 0; // Reset staging buffers memory to 0
    }

    m_beginFrame_fn(m_rendererContext, m_imagesAvailableSemaphores.at(m_currentIndexFrame));

    auto ok = vkResetFences(m_device, 1, &m_frameCompletedFences.at(m_currentIndexFrame));
    assert(ok == VK_SUCCESS);

    vkResetCommandPool(m_device,
                       m_commandBufferSets.at(static_cast<u32>(QueueType::GRAPHICS))
                         .commandPools.at(m_currentIndexFrame),
                       0x0);
    vkResetCommandPool(m_device,
                       m_commandBufferSets.at(static_cast<u32>(QueueType::COMPUTE))
                         .commandPools.at(m_currentIndexFrame),
                       0x0);

    const auto swapchainState = m_getSwapchainState_fn();

    BeginCommandRecording(QueueType::GRAPHICS);

    UpdateResources(); // TODO Make sure resources in use are not destroyed.

    TransitionImageLayout(swapchainState.swapchainImage,
                          0,
                          VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
}

void VulkanDevice::EndFrame()
{
    const auto swapchainState = m_getSwapchainState_fn();

    TransitionImageLayout(swapchainState.swapchainImage,
                          VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                          0,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    EndCommandRecording(true, true);

    if (m_lastCommandBuffer && m_lastCommandBuffer->queueType != QueueType::COUNT)
    {
        m_endFrame_fn(m_rendererContext, m_lastCommandBuffer->signalSemaphore);
    }

    m_descriptorSetManager.Update();

    DeleteResources();

    m_currentCommandBuffer          = nullptr;
    m_lastCommandBuffer             = nullptr;
    m_currentRenderPipeline         = nullptr;
    m_currentRenderPipelineInternal = nullptr;
}

void VulkanDevice::EnableRendering(const VkRect2D&                 renderArea,
                                   const std::vector<FrameBuffer>& colorAttachments,
                                   FrameBuffer*                    depthAttachment)
{
    UpdateResources();

    auto fillAttachmentInfo = [this](const FrameBuffer& frameBuffer, bool isDepth = false)
    {
        const auto vulkanTexture = this->GetVulkanTexture(frameBuffer.textureView.GetID());

        const auto loadOp = frameBuffer.loadOperation == FrameBufferLoadOperation::CLEAR ?
                              VK_ATTACHMENT_LOAD_OP_CLEAR :
                            frameBuffer.loadOperation == FrameBufferLoadOperation::LOAD ?
                              VK_ATTACHMENT_LOAD_OP_LOAD :
                              VK_ATTACHMENT_LOAD_OP_DONT_CARE;

        const auto storeOp = frameBuffer.storeOperation == FrameBufferStoreOperation::STORE ?
                               VK_ATTACHMENT_STORE_OP_STORE :
                               VK_ATTACHMENT_STORE_OP_DONT_CARE;

        auto clear = isDepth ? VkClearValue{frameBuffer.clearColor[0], frameBuffer.clearColor[1]} :
                               VkClearValue{frameBuffer.clearColor[0],
                                            frameBuffer.clearColor[1],
                                            frameBuffer.clearColor[2],
                                            frameBuffer.clearColor[3]};

        VkRenderingAttachmentInfo info{
          .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .pNext       = nullptr,
          .imageView   = vulkanTexture.imageView,
          .imageLayout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
                                   VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
          .loadOp      = loadOp,
          .storeOp     = storeOp,
          .clearValue  = clear,
        };

        return info;
    };

    VkRenderingAttachmentInfo              vulkanDepthAttachment{};
    std::vector<VkRenderingAttachmentInfo> attachments;
    attachments.reserve(colorAttachments.size());

    // Enable present to swapchain image.
    if (colorAttachments.empty() && !depthAttachment)
    {
        assert(depthAttachment == nullptr);

        const auto& swapchainState = m_getSwapchainState_fn();

        VkRenderingAttachmentInfo attachment{
          .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .pNext       = nullptr,
          .imageView   = swapchainState.swapchainImageView,
          .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
          .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
          .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
          .clearValue  = {.0f, .0f, .0f, .0f},
        };

        attachments.emplace_back(attachment);
    }
    else
    {
        for (const auto& attachment : colorAttachments)
        {
            attachments.push_back(fillAttachmentInfo(attachment));
        }

        if (depthAttachment)
        {
            vulkanDepthAttachment = fillAttachmentInfo(*depthAttachment, true);
        }
    }

    VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
    info.colorAttachmentCount = static_cast<u32>(attachments.size());
    info.pColorAttachments    = attachments.data();
    info.pDepthAttachment     = depthAttachment ? &vulkanDepthAttachment : nullptr;
    info.renderArea           = renderArea;
    info.layerCount           = 1;

    const auto& cmd = m_currentCommandBuffer->commandBuffer;
    vkCmdBeginRendering(cmd, &info);
}

void VulkanDevice::DisableRendering()
{
    const auto& cmd = m_currentCommandBuffer->commandBuffer;
    vkCmdEndRendering(cmd);
}

void VulkanDevice::SetGraphicsState(GraphicsState* state)
{
    m_currentGraphicsState = *state;
    auto& cmd              = m_currentCommandBuffer->commandBuffer;

    VkViewport viewport = {static_cast<f32>(m_currentGraphicsState.viewport.x),
                           static_cast<f32>(m_currentGraphicsState.viewport.height),
                           static_cast<f32>(m_currentGraphicsState.viewport.width),
                           static_cast<f32>(-m_currentGraphicsState.viewport.height),
                           0.0f,
                           1.0f};
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissors = {
      .offset = {0, 0},
      .extent = {static_cast<u32>(m_currentGraphicsState.viewport.width),
                 static_cast<u32>(m_currentGraphicsState.viewport.height)},
    };
    vkCmdSetScissor(cmd, 0, 1, &scissors);
}

void VulkanDevice::SetRenderPipeline(const RenderPipeline* pipeline)
{
    m_currentRenderPipeline = pipeline;
}

void VulkanDevice::SubmitDrawCalls(const std::vector<DrawCall>& drawCalls)
{
    for (const auto& dc : drawCalls)
    {
        SubmitDrawCall(dc);
    }
}

GPUBuffer VulkanDevice::CreateBuffer(const BufferDescriptor& descriptor, void* data)
{
    const auto id = m_resourceGenerator.GenerateBufferResource();

    auto  bufferCreationInfoPtr = new BufferCreationInfo();
    auto& bufferCreationInfo    = *bufferCreationInfoPtr;
    bufferCreationInfo.data     = std::move(static_cast<u8*>(data));
    bufferCreationInfo.bufferId = id;

    auto& buffer        = bufferCreationInfo.buffer;
    buffer.m_descriptor = descriptor;

    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 GetVulkanBufferUsage(descriptor.usage);
    info.size = descriptor.size;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    auto ok = vmaCreateBuffer(m_allocator,
                              &info,
                              &allocInfo,
                              &buffer.m_buffer,
                              &buffer.m_allocation,
                              nullptr);

    assert(ok == VK_SUCCESS);

    m_bufferCreateList.push(bufferCreationInfoPtr);

    return {id, info.size, this};
}

void VulkanDevice::UpdateBuffer(BufferResource bufferId, void* data)
{
    CreateBuffersInternal();

    if (!data)
        return;

    const auto vulkanBuffer = GetVulkanBuffer(bufferId);

    if (vulkanBuffer.m_buffer == VK_NULL_HANDLE)
        return;

    UpdateBufferInternal(bufferId, 0, static_cast<u8*>(data), 0);
}

void VulkanDevice::DestroyBuffer(BufferResource resource)
{
    if (auto buffer = m_buffers.find(resource); buffer != m_buffers.end())
    {
        m_buffersToDestroy.at(m_currentIndexFrame).push_back(resource);
    }
}

GPUTexture VulkanDevice::CreateTexture(const TextureDescriptor& descriptor, void* data)
{
    const auto id = m_resourceGenerator.GenerateTextureResource();

    auto  textureCreationInfoPtr  = new TextureCreationInfo();
    auto& textureCreationInfo     = *textureCreationInfoPtr;
    textureCreationInfo.data      = static_cast<u8*>(data);
    textureCreationInfo.textureId = id;

    VulkanTexture texture;
    texture.descriptor = descriptor;

    const auto vulkanFormat = FormatToVulkanFormatMap[descriptor.format];

    VkImageCreateInfo info{};
    info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext         = nullptr;
    info.imageType     = VK_IMAGE_TYPE_2D;
    info.format        = vulkanFormat;
    info.extent        = {descriptor.width, descriptor.height, descriptor.depth};
    info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | descriptor.usage;
    info.samples       = VK_SAMPLE_COUNT_1_BIT;
    info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.mipLevels     = 1;
    info.arrayLayers   = 1;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VmaAllocationInfo allocInfo{};

    auto ok = vmaCreateImage(m_allocator,
                             &info,
                             &allocCreateInfo,
                             &texture.image,
                             &texture.allocation,
                             &allocInfo);
    assert(ok == VK_SUCCESS);

    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.pNext                           = nullptr;
    viewInfo.image                           = texture.image;
    viewInfo.format                          = vulkanFormat;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;
    viewInfo.subresourceRange.aspectMask =
      info.format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    ok = vkCreateImageView(m_device, &viewInfo, nullptr, &texture.imageView);
    assert(ok == VK_SUCCESS);

    textureCreationInfo.texture = std::move(texture);

    m_textureCreateList.emplace(textureCreationInfoPtr);

    m_textures.insert({id, texture});

    return {id, this};
}

void VulkanDevice::UpdateTexture(const TextureResource& textureId, void* data)
{
    CreateTexturesInternal();

    if (data == nullptr)
        return;

    const auto texture = GetVulkanTexture(textureId);
    if (texture.image == VK_NULL_HANDLE || texture.imageView == VK_NULL_HANDLE)
        return;

    UpdateTextureInternal(textureId, 0, static_cast<u8*>(data), 0);
}

void VulkanDevice::DestroyTexture(TextureResource resource)
{
    if (auto texture = m_textures.find(resource); texture != m_textures.end())
    {
        auto t = texture->second;
        vmaDestroyImage(m_allocator, t.image, t.allocation);
        vkDestroyImageView(m_device, t.imageView, nullptr);

        m_textures.erase(texture);
    }
}

bool VulkanDevice::IsResourceValid(const GPUResource& resource) const
{
    return resource.id == GPU_RESOURCE_INVALID ? false : true;
}

void VulkanDevice::WaitIdle() const { assert(vkDeviceWaitIdle(m_device) == VK_SUCCESS); }

UniformDescriptorSetInfos VulkanDevice::GetUniformDescriptorSetInfos(
  const std::vector<UniformDescriptor>& uniformDescriptors)
{
    UniformDescriptorSetInfos uniformDescriptorSetInfos;
    for (auto& uniformDescriptor : uniformDescriptorSetInfos)
    {
        uniformDescriptor.resources.reserve(MAX_UNIFORM_SLOTS);
    }

    for (const auto& uniform : uniformDescriptors)
    {
        if (const auto& bufferViews =
              std::get_if<std::vector<GPUBufferView>>(&uniform.uniformViews))
        {
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            bufferInfos.reserve(bufferViews->size());
            for (const auto& bufferView : *bufferViews)
            {
                if (const auto& bufferId = bufferView.GetID();
                    bufferId.id != GPU_RESOURCE_INVALID && bufferId.type == ResourceType::BUFFER)
                {
                    const auto             buffer = GetVulkanBuffer(bufferId);
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = buffer.m_buffer;
                    bufferInfo.offset = 0;
                    bufferInfo.range  = buffer.m_descriptor.size;

                    bufferInfos.emplace_back(bufferInfo);
                }
            }
            uniformDescriptorSetInfos.at(uniform.set)
              .resources.emplace_back(std::move(bufferInfos),
                                      uniform.binding,
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }
        else if (const auto& textureViews =
                   std::get_if<std::vector<GPUTextureView>>(&uniform.uniformViews))
        {
            std::vector<VkDescriptorImageInfo> imageInfos;
            imageInfos.reserve(textureViews->size());
            for (const auto& textureView : *textureViews)
            {
                if (const auto& textureId = textureView.GetID();
                    textureId.id != GPU_RESOURCE_INVALID && textureId.type == ResourceType::TEXTURE)
                {
                    const auto            texture = GetVulkanTexture(textureId);
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView   = texture.imageView;
                    imageInfo.sampler     = m_sampler;

                    imageInfos.emplace_back(std::move(imageInfo));
                }
            }
            uniformDescriptorSetInfos.at(uniform.set)
              .resources.emplace_back(std::move(imageInfos),
                                      uniform.binding,
                                      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }
    }

    return uniformDescriptorSetInfos;
}

void VulkanDevice::BeginCommandRecording(QueueType type)
{
    if (!m_currentCommandBuffer || m_currentCommandBuffer->queueType != type)
    {
        if (m_currentCommandBuffer && m_currentCommandBuffer->queueType != QueueType::COUNT)
        {
            EndCommandRecording();
        }

        auto& commandBufferIndex =
          m_commandBufferSets.at(static_cast<u32>(type)).commandBufferIndex.at(m_currentIndexFrame);
        auto& commandBuffers =
          m_commandBufferSets.at(static_cast<u32>(type)).commandBuffers.at(m_currentIndexFrame);
        auto& commandPool =
          m_commandBufferSets.at(static_cast<u32>(type)).commandPools.at(m_currentIndexFrame);
        ++commandBufferIndex;

        if ((commandBufferIndex + 1) > static_cast<i32>(commandBuffers.size()))
        {
            auto cmd = new CommandBuffer(type, CreateCommandBuffer(commandPool), CreateSemaphore());
            commandBuffers.emplace_back(std::move(cmd));
        }

        auto& commandBufferInfo = commandBuffers.at(commandBufferIndex);

        VkCommandBufferBeginInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        auto                     ok = vkBeginCommandBuffer(commandBufferInfo->commandBuffer, &info);
        if (ok == VK_SUCCESS)
        {
            m_currentCommandBuffer = commandBufferInfo;
        }
        else
        {
            m_currentCommandBuffer = nullptr;
        }
    }
}

void VulkanDevice::EndCommandRecording(bool waitForImage, bool signalFence)
{
    if (!m_currentCommandBuffer || m_currentCommandBuffer->queueType == QueueType::COUNT)
        return;

    const auto& commandQueue = m_queues.at(static_cast<u32>(m_currentCommandBuffer->queueType));

    auto ok = vkEndCommandBuffer(m_currentCommandBuffer->commandBuffer);
    assert(ok == VK_SUCCESS);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &m_currentCommandBuffer->commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &m_currentCommandBuffer->signalSemaphore;

    if (!m_lastCommandBuffer || m_lastCommandBuffer->queueType == QueueType::COUNT)
    {
        if (waitForImage)
        {
            constexpr VkPipelineStageFlags waitStages[] = {
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            submitInfo.pWaitSemaphores    = &m_imagesAvailableSemaphores.at(m_currentIndexFrame);
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitDstStageMask  = waitStages;
        }
    }
    else
    {
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT};
        submitInfo.pWaitDstStageMask      = waitStages;
        submitInfo.waitSemaphoreCount     = 1;
        submitInfo.pWaitSemaphores        = &m_lastCommandBuffer->signalSemaphore;
    }

    ok =
      vkQueueSubmit(commandQueue.queue,
                    1,
                    &submitInfo,
                    signalFence ? m_frameCompletedFences.at(m_currentIndexFrame) : VK_NULL_HANDLE);
    assert(ok == VK_SUCCESS);

    m_lastCommandBuffer    = m_currentCommandBuffer;
    m_currentCommandBuffer = nullptr;
}

VkFence VulkanDevice::CreateFence(VkFenceCreateFlags flags) const
{
    VkFence           fence{VK_NULL_HANDLE};
    VkFenceCreateInfo info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, VK_NULL_HANDLE, flags};
    auto              ok = vkCreateFence(m_device, &info, nullptr, &fence);
    assert(ok == VK_SUCCESS);
    return fence;
}

VkSemaphore VulkanDevice::CreateSemaphore(VkSemaphoreCreateFlags flags) const
{
    VkSemaphore           semaphore{VK_NULL_HANDLE};
    VkSemaphoreCreateInfo info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, VK_NULL_HANDLE, flags};
    vkCreateSemaphore(m_device, &info, nullptr, &semaphore);

    return semaphore;
}

VkCommandPool VulkanDevice::CreateCommandPool(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
    VkCommandPool           commandPool;
    VkCommandPoolCreateInfo info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                 VK_NULL_HANDLE,
                                 flags,
                                 queueFamilyIndex};
    auto                    ok = vkCreateCommandPool(m_device, &info, nullptr, &commandPool);
    assert(ok == VK_SUCCESS);
    return commandPool;
}

VkCommandBuffer VulkanDevice::CreateCommandBuffer(const VkCommandPool& commandPool)
{
    VkCommandBuffer cmd;

    VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                     VK_NULL_HANDLE,
                                     commandPool,
                                     VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                     1};
    auto                        ok = vkAllocateCommandBuffers(m_device, &info, &cmd);
    assert(ok == VK_SUCCESS);

    return cmd;
}

VulkanPipeline* VulkanDevice::GetRenderPipeline(const RenderPipeline* pipeline,
                                                const GraphicsState&  graphicsState)
{
    auto key = PipelineKey{pipeline, graphicsState};

    auto it = m_pipelines.find(key);
    if (it != m_pipelines.end())
        return &it->second;

    VulkanPipeline renderPipeline = VulkanPipeline::Create(m_device, *pipeline, graphicsState);

    m_pipelines.insert({key, renderPipeline});
    auto p = m_pipelines.find(key);
    if (p != m_pipelines.end())
        return &p->second;

    return nullptr;
}

VulkanBuffer VulkanDevice::GetVulkanBuffer(const BufferResource& bufferResource)
{
    auto buffer = m_buffers.find(bufferResource);
    if (buffer != m_buffers.end())
        return buffer->second;

    return {};
}

VulkanTexture VulkanDevice::GetVulkanTexture(const TextureResource& textureResource)
{
    auto texture = m_textures.find(textureResource);
    if (texture != m_textures.end())
        return texture->second;

    return {};
}

void VulkanDevice::SubmitDrawCall(const DrawCall& drawCall)
{
    auto        pipeline = GetRenderPipeline(m_currentRenderPipeline, m_currentGraphicsState);
    const auto& cmd      = m_currentCommandBuffer->commandBuffer;

    if (m_currentRenderPipelineInternal != pipeline)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
        m_currentRenderPipelineInternal = pipeline;
    }

    // Uniforms
    if (!drawCall.uniforms.empty())
    {
        const std::array<Shader, 2> shaders{m_currentRenderPipeline->vertexShader,
                                            m_currentRenderPipeline->fragmentShader};

        const auto& uniformDescriptorSetInfos = GetUniformDescriptorSetInfos(drawCall.uniforms);
        const auto& sets =
          m_descriptorSetManager.GetDescriptorSet(m_device, uniformDescriptorSetInfos, pipeline);

        vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline->GetPipelineLayout(),
                                0,
                                static_cast<u32>(sets.size()),
                                sets.data(),
                                0,
                                nullptr);
    }

    // Bind vertex buffers
    const auto& bufferIt = m_buffers.find(drawCall.vertexBuffer.GetID());
    assert(bufferIt != m_buffers.end());

    const auto& buffer = bufferIt->second;

    VkDeviceSize offset{0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer.m_buffer, &offset);

    if (drawCall.indexBuffer.GetID().id != GPU_RESOURCE_INVALID)
    {
        VkDeviceSize indexOffset{0};
        const auto&  indexBuffer = GetVulkanBuffer(drawCall.indexBuffer.GetID());
        vkCmdBindIndexBuffer(cmd,
                             indexBuffer.m_buffer,
                             0,
                             indexBuffer.m_descriptor.elementSize == 2 ? VK_INDEX_TYPE_UINT16 :
                                                                         VK_INDEX_TYPE_UINT32);

        const u32 indexCount =
          static_cast<u32>(indexBuffer.m_descriptor.size / indexBuffer.m_descriptor.elementSize);
        vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
    }
    else
    {
        const u32 vertexCount =
          static_cast<u32>(buffer.m_descriptor.size / buffer.m_descriptor.elementSize);
        vkCmdDraw(cmd, vertexCount, 1, 0, 0);
    }
}

void VulkanDevice::CreateBuffersInternal()
{
    while (!m_bufferCreateList.empty())
    {
        auto& [bufferId, buffer, data] = *m_bufferCreateList.front();
        m_buffers.emplace(bufferId, buffer);

        if (data != nullptr)
        {
            UpdateBufferInternal(bufferId, 0, data, 0);
        }

        m_bufferCreateList.pop();
    }
}

void VulkanDevice::UpdateBuffersInternal()
{
    const auto cmd = m_currentCommandBuffer->commandBuffer;

    for (auto bufferUpdateInfo : m_bufferUpdateList)
    {
        auto& [buffer, stagingBuffer, region] = bufferUpdateInfo;
        const auto size                       = buffer.m_descriptor.size;

        vkCmdCopyBuffer(cmd, stagingBuffer.buffer, buffer.m_buffer, 1, &region);

        VkBufferMemoryBarrier2 barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2};
        barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
        barrier.buffer        = buffer.m_buffer;
        barrier.size          = size;

        VkDependencyInfo dependency{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        dependency.bufferMemoryBarrierCount = 1;
        dependency.pBufferMemoryBarriers    = &barrier;

        vkCmdPipelineBarrier2(cmd, &dependency);
    }

    m_bufferUpdateList.clear();
}

void VulkanDevice::DeleteBufferInternal(BufferResource bufferId)
{
    if (auto it = m_buffers.find(bufferId); it != m_buffers.end())
    {
        vmaDestroyBuffer(m_allocator, it->second.m_buffer, it->second.m_allocation);
    }

    m_buffers.erase(bufferId);
}

void VulkanDevice::UpdateBufferInternal(BufferResource bufferId,
                                        size_t         bufferOffset,
                                        const u8*      data,
                                        size_t         dataOffset)
{
    auto buffer = m_buffers.find(bufferId);
    if (buffer == m_buffers.end())
    {
        return;
    }

    const auto dataSize            = buffer->second.m_descriptor.size;
    const auto stagingBuffer       = GetStagingBuffer(dataSize);
    const auto stagingBufferOffset = stagingBuffer.memory - dataSize;
    vmaCopyMemoryToAllocation(m_allocator,
                              data + dataOffset,
                              stagingBuffer.allocation,
                              stagingBufferOffset,
                              dataSize);

    const auto region = VkBufferCopy{stagingBufferOffset, bufferOffset, dataSize};

    m_bufferUpdateList.emplace_back(buffer->second, stagingBuffer, region);
}

void VulkanDevice::CreateTexturesInternal()
{
    while (!m_textureCreateList.empty())
    {
        auto& [textureId, texture, data] = *m_textureCreateList.front();
        m_textures.emplace(textureId, texture);

        if (data)
        {
            UpdateTextureInternal(textureId, 0, data, 0);
        }

        m_textureCreateList.pop();
    }
}

void VulkanDevice::UpdateTexturesInternal()
{
    const auto cmd = m_currentCommandBuffer->commandBuffer;

    for (auto updateInfo : m_textureUpdateList)
    {
        auto& [texture, stagingBuffer, region] = updateInfo;
        const auto size   = texture.descriptor.width * texture.descriptor.height * 4;
        const auto format = FormatToVulkanFormatMap[texture.descriptor.format];

        VkImageAspectFlags imageAspect =
          format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageSubresourceRange subresourceRange{
          .aspectMask     = imageAspect,
          .baseMipLevel   = 0,
          .levelCount     = 1,
          .baseArrayLayer = 0,
          .layerCount     = 1,
        };

        TransitionImageLayout(texture.image,
                              0,
                              VK_ACCESS_2_MEMORY_WRITE_BIT,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                              subresourceRange);

        const auto cmd = m_currentCommandBuffer->commandBuffer;
        vkCmdCopyBufferToImage(cmd,
                               stagingBuffer.buffer,
                               texture.image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);

        // Transition image again from transfer_dst to desired (generally shader_read_only)
        TransitionImageLayout(texture.image,
                              VK_ACCESS_2_TRANSFER_WRITE_BIT,
                              VK_ACCESS_2_SHADER_READ_BIT,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              texture.descriptor.layout,
                              VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                              VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                              subresourceRange);
    }

    m_textureUpdateList.clear();
}

void VulkanDevice::DeleteTextureInternal(TextureResource textureId)
{
    if (auto it = m_textures.find(textureId); it != m_textures.end())
    {
        if (it->second.imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(m_device, it->second.imageView, nullptr);
        }
        vmaDestroyImage(m_allocator, it->second.image, it->second.allocation);
    }

    m_textures.erase(textureId);
}

void VulkanDevice::UpdateTextureInternal(TextureResource textureId,
                                         size_t          textureOffset,
                                         const u8*       data,
                                         size_t          dataOffset)
{
    auto textureIt = m_textures.find(textureId);
    if (textureIt == m_textures.end())
    {
        return;
    }

    auto       texture = textureIt->second;
    const auto dataSize =
      texture.descriptor.width * texture.descriptor.height * 4; // TODO 4 is hardcoded so far.

    const auto stagingBuffer       = GetStagingBuffer(dataSize);
    const auto stagingBufferOffset = stagingBuffer.memory - dataSize;
    const auto region              = VkBufferImageCopy{
      stagingBufferOffset,
      0,
      0,
                   {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                   {0, 0, 0},
                   {texture.descriptor.width, texture.descriptor.height, texture.descriptor.depth}};

    vmaCopyMemoryToAllocation(m_allocator,
                              data,
                              stagingBuffer.allocation,
                              stagingBufferOffset,
                              dataSize);
    m_textureUpdateList.emplace_back(texture, stagingBuffer, region);
}

void VulkanDevice::DeleteResources()
{
    auto& buffersToDestroy = m_buffersToDestroy.at(m_currentIndexFrame);
    while (!buffersToDestroy.empty())
    {
        DeleteBufferInternal(buffersToDestroy.front());
        buffersToDestroy.pop_front();
    }

    auto& textureDestroyList = m_texturesToDestroy.at(m_currentIndexFrame);
    while (!textureDestroyList.empty())
    {
        DeleteTextureInternal(textureDestroyList.front());
        textureDestroyList.pop_front();
    }
}

void VulkanDevice::UpdateResources()
{
    CreateBuffersInternal();
    UpdateBuffersInternal();
    CreateTexturesInternal();
    UpdateTexturesInternal();
}

VkCommandBuffer VulkanDevice::BeginImmediateCommandBuffer(VkCommandPool commandPool)
{
    VkCommandBuffer             cmd;
    VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                     VK_NULL_HANDLE,
                                     commandPool,
                                     VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                     1};
    vkAllocateCommandBuffers(m_device, &info, &cmd);

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                       VK_NULL_HANDLE,
                                       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                       VK_NULL_HANDLE};
    vkBeginCommandBuffer(cmd, &beginInfo);

    return cmd;
}

void VulkanDevice::FlushImmediateCommandBuffer(VkCommandBuffer cmd,
                                               VkQueue         queue,
                                               VkFence         fence) const
{
    vkEndCommandBuffer(cmd);

    VkSubmitInfo info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    info.commandBufferCount = 1;
    info.pCommandBuffers    = &cmd;

    auto ok = vkQueueSubmit(queue, 1, &info, fence);
    assert(ok == VK_SUCCESS);
    vkQueueWaitIdle(queue);
}

void VulkanDevice::TransitionImageLayout(TextureResource         image,
                                         VkAccessFlags           srcAccessFlags,
                                         VkAccessFlags           dstAccessFlags,
                                         VkImageLayout           currentLayout,
                                         VkImageLayout           targetLayout,
                                         VkPipelineStageFlags    srcStageFlags,
                                         VkPipelineStageFlags    dstStageFlags,
                                         VkImageSubresourceRange subresourceRange,
                                         bool                    immediate)
{
    const auto vulkanTexture = GetVulkanTexture(image);

    TransitionImageLayout(vulkanTexture.image,
                          srcAccessFlags,
                          dstAccessFlags,
                          currentLayout,
                          targetLayout,
                          srcStageFlags,
                          dstStageFlags,
                          subresourceRange,
                          immediate);
}

VulkanDevice::StagingBuffer VulkanDevice::GetStagingBuffer(u64 size)
{
    VulkanBuffer staging;
    staging.m_descriptor.elementSize = size;
    staging.m_descriptor.size        = size;

    for (auto& stagingBuffer : m_stagingBuffers)
    {
        const auto freeSize = stagingBuffer.size - stagingBuffer.memory;
        if (size <= freeSize)
        {
            stagingBuffer.memory += size;
            return stagingBuffer;
        }
    }

    u64           stagingBufferSize = size < MiB(1) ?
                                        MiB(1) :
                                        static_cast<u64>(std::ceil(static_cast<f64>(size) / MiB(1))) * MiB(1);
    StagingBuffer stagingBuffer;
    stagingBuffer.size   = stagingBufferSize;
    stagingBuffer.memory = size;

    VkBufferCreateInfo stagingBufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    stagingBufferInfo.size        = stagingBufferSize;
    stagingBufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo stagingAllocInfo{};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    auto ok = vmaCreateBuffer(m_allocator,
                              &stagingBufferInfo,
                              &stagingAllocInfo,
                              &stagingBuffer.buffer,
                              &stagingBuffer.allocation,
                              nullptr);

    assert(ok == VK_SUCCESS);

    m_stagingBuffers.emplace_back(stagingBuffer);
    return m_stagingBuffers.back();
}

void VulkanDevice::TransitionImageLayout(VkImage                 image,
                                         VkAccessFlags           srcAccessFlags,
                                         VkAccessFlags           dstAccessFlags,
                                         VkImageLayout           currentLayout,
                                         VkImageLayout           targetLayout,
                                         VkPipelineStageFlags    srcStageFlags,
                                         VkPipelineStageFlags    dstStageFlags,
                                         VkImageSubresourceRange subresourceRange,
                                         bool                    immediate)
{
    auto cmd = immediate ? BeginImmediateCommandBuffer(m_immediateCommandPool) :
                           m_currentCommandBuffer->commandBuffer;

    VkImageMemoryBarrier2 barrier{
      .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask     = srcStageFlags,
      .srcAccessMask    = srcAccessFlags,
      .dstStageMask     = dstStageFlags,
      .dstAccessMask    = dstAccessFlags,
      .oldLayout        = currentLayout,
      .newLayout        = targetLayout,
      .image            = image,
      .subresourceRange = subresourceRange,
    };

    VkDependencyInfo dependency{
      .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers    = &barrier,
    };

    vkCmdPipelineBarrier2(cmd, &dependency);

    if (immediate)
        FlushImmediateCommandBuffer(cmd, m_queues[static_cast<u32>(QueueType::GRAPHICS)].queue);
}
} // namespace vulkan
} // namespace RED
