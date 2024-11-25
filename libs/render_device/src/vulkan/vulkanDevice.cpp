#include "pch.hpp"

#define VMA_IMPLEMENTATION
#define VMA_DEBUG_LOG_FORMAT
#include <external/VulkanMemoryAllocator/include/vk_mem_alloc.h>

#include "render_device/bufferDescriptor.h"
#include "render_device/renderPipeline.h"
#include "src/vulkan/utils.h"
#include "src/vulkan/vulkanDevice.h"
#include "src/vulkan/vulkanPipeline.h"
#include "vulkanDevice.h"

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

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_globalUniformBuffers.at(i) =
          CreateInternalBuffer(UNIFORM_BUFFER_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
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
    vkDeviceWaitIdle(m_device);

    for (auto& pipeline : m_pipelines)
        pipeline.second.Destroy(m_device);

    m_pipelines.clear();

    for (auto& buffer : m_buffers)
        vmaDestroyBuffer(m_allocator, buffer.second.m_buffer, buffer.second.m_allocation);

    m_buffers.clear();

    for (auto& texture : m_textures)
        vmaDestroyImage(m_allocator, texture.second.image, texture.second.allocation);

    m_textures.clear();

    vkDestroySampler(m_device, m_sampler, nullptr);

    m_descriptorSetManager.OnDestroy(m_device);

    for (auto& uniformBuffers : m_globalUniformBuffers)
        vmaDestroyBuffer(m_allocator, uniformBuffers.m_buffer, uniformBuffers.m_allocation);

    for (auto& stagingBuffer : m_stagingBuffers)
        vmaDestroyBuffer(m_allocator,
                         stagingBuffer.second.m_buffer,
                         stagingBuffer.second.m_allocation);

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

    m_currentCommandBuffer          = nullptr;
    m_lastCommandBuffer             = nullptr;
    m_currentRenderPipeline         = nullptr;
    m_currentRenderPipelineInternal = nullptr;
}

void VulkanDevice::EnableRendering(const VkRect2D&                               renderArea,
                                   const std::vector<VkRenderingAttachmentInfo>& colorAttachments,
                                   VkRenderingAttachmentInfo*                    depthAttachment)
{
    VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
    info.colorAttachmentCount = static_cast<u32>(colorAttachments.size());
    info.pColorAttachments    = colorAttachments.data();
    info.pDepthAttachment     = depthAttachment ? depthAttachment : nullptr;
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

void VulkanDevice::SetRenderPipeline(RenderPipeline* pipeline)
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

    VulkanBuffer buffer;
    buffer.m_descriptor = descriptor;

    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.usage =
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | descriptor.usage;
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

    if (data != nullptr)
    {
        VulkanBuffer staging;

        VkBufferCreateInfo stagingBufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        stagingBufferInfo.size        = info.size;
        stagingBufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo{};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        ok = vmaCreateBuffer(m_allocator,
                             &stagingBufferInfo,
                             &stagingAllocInfo,
                             &staging.m_buffer,
                             &staging.m_allocation,
                             nullptr);
        assert(ok == VK_SUCCESS);

        vmaCopyMemoryToAllocation(m_allocator, data, staging.m_allocation, 0, descriptor.size);

        VkBufferCopy region{};
        region.size = info.size;

        const auto cmd = BeginImmediateCommandBuffer(m_immediateCommandPool);
        vkCmdCopyBuffer(cmd, staging.m_buffer, buffer.m_buffer, 1, &region);
        FlushImmediateCommandBuffer(cmd, m_queues.at(static_cast<u32>(QueueType::GRAPHICS)).queue);

        vmaDestroyBuffer(m_allocator, staging.m_buffer, staging.m_allocation);
    }

    m_buffers.insert({id, buffer});

    return {id, info.size, this};
}

void VulkanDevice::UpdateBuffer(BufferResource bufferId, void* data)
{
    if (!data)
        return;

    auto       vulkanBuffer = GetVulkanBuffer(bufferId);
    const auto size         = vulkanBuffer.m_descriptor.size;

    VulkanBuffer staging = GetStagingBuffer(size);

    vmaCopyMemoryToAllocation(m_allocator, data, staging.m_allocation, 0, size);

    VkBufferCopy region{0, 0, size};

    const auto cmd = m_currentCommandBuffer->commandBuffer;

    vkCmdCopyBuffer(cmd, staging.m_buffer, vulkanBuffer.m_buffer, 1, &region);

    VkBufferMemoryBarrier2 barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2};
    barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask  = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
    barrier.buffer        = vulkanBuffer.m_buffer;
    barrier.size          = size;

    VkDependencyInfo dependency{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
    dependency.bufferMemoryBarrierCount = 1;
    dependency.pBufferMemoryBarriers    = &barrier;

    vkCmdPipelineBarrier2(cmd, &dependency);
}

void VulkanDevice::DestroyBuffer(BufferResource resource)
{
    if (auto buffer = m_buffers.find(resource); buffer != m_buffers.end())
    {
        auto b = buffer->second;
        vmaDestroyBuffer(m_allocator, b.m_buffer, b.m_allocation);

        m_buffers.erase(buffer);
    }
}

GPUTexture VulkanDevice::CreateTexture(const TextureDescriptor& descriptor, const void* data)
{
    const auto id = m_resourceGenerator.GenerateTextureResource();

    VulkanTexture texture;
    texture.descriptor = descriptor;

    VkImageCreateInfo info{};
    info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext         = nullptr;
    info.imageType     = VK_IMAGE_TYPE_2D;
    info.format        = descriptor.format;
    info.extent        = descriptor.extent;
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
    viewInfo.format                          = descriptor.format;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;
    viewInfo.subresourceRange.aspectMask =
      info.format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    ok = vkCreateImageView(m_device, &viewInfo, nullptr, &texture.imageView);
    assert(ok == VK_SUCCESS);

    m_textures.insert({id, texture});

    if (data != nullptr)
    {
        // Get staging buffer and copy data to it.
        const auto   stagingBufferSize = info.extent.height * info.extent.width * 4;
        VulkanBuffer staging           = GetStagingBuffer(stagingBufferSize);

        vmaCopyMemoryToAllocation(m_allocator, data, staging.m_allocation, 0, stagingBufferSize);

        // Transition the image from undefined to transfer_dst
        TransitionImageLayout(texture.image,
                              0,
                              VK_ACCESS_2_MEMORY_WRITE_BIT,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                              viewInfo.subresourceRange);

        // Copy buffer to image
        VkBufferImageCopy region{};
        region.imageExtent                     = descriptor.extent;
        region.bufferOffset                    = 0;
        region.imageOffset                     = {0, 0, 0};
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageSubresource.mipLevel       = 0;

        const auto cmd = m_currentCommandBuffer->commandBuffer;
        vkCmdCopyBufferToImage(cmd,
                               staging.m_buffer,
                               texture.image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);

        // Transition image again from transfer_dst to desired (generally shader_read_only)
        TransitionImageLayout(texture.image,
                              VK_ACCESS_2_TRANSFER_WRITE_BIT,
                              VK_ACCESS_2_SHADER_READ_BIT,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              descriptor.layout,
                              VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                              VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                              viewInfo.subresourceRange);
    }
    else
    {
        TransitionImageLayout(texture.image,
                              VK_ACCESS_2_MEMORY_WRITE_BIT,
                              VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              descriptor.layout,
                              VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                              VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                              viewInfo.subresourceRange);
    }

    return {id, this};
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

VkRenderingAttachmentInfo VulkanDevice::GetAttachment(const GPUTextureView& textureView,
                                                      VkImageLayout         layout,
                                                      VkAttachmentLoadOp    loadOp,
                                                      VkAttachmentStoreOp   storeOp,
                                                      VkClearValue          clearValue)
{
    assert(!textureView.IsEmpty());

    const auto& texture = GetVulkanTexture(textureView.GetID());

    return VkRenderingAttachmentInfo{
      .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView   = texture.imageView,
      .imageLayout = layout,
      .loadOp      = loadOp,
      .storeOp     = storeOp,
      .clearValue  = clearValue,
    };
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
        if (const auto& bufferId = uniform.bufferView.GetID();
            bufferId.id != GPU_RESOURCE_INVALID && bufferId.type == ResourceType::BUFFER)
        {
            const auto             buffer = GetVulkanBuffer(bufferId);
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer.m_buffer;
            bufferInfo.offset = 0;
            bufferInfo.range  = buffer.m_descriptor.size;

            uniformDescriptorSetInfos.at(uniform.set)
              .resources.emplace_back(std::move(bufferInfo),
                                      uniform.binding,
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }
        else if (const auto& textureId = uniform.textureView.GetID();
                 textureId.id != GPU_RESOURCE_INVALID && textureId.type == ResourceType::TEXTURE)
        {
            const auto            texture = GetVulkanTexture(textureId);
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView   = texture.imageView;
            imageInfo.sampler     = m_sampler;

            uniformDescriptorSetInfos.at(uniform.set)
              .resources.emplace_back(std::move(imageInfo),
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

VulkanBuffer VulkanDevice::CreateInternalBuffer(u32 size, VkBufferUsageFlagBits usage)
{
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
    info.size        = size;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VulkanBuffer buffer;
    auto         ok = vmaCreateBuffer(m_allocator,
                              &info,
                              &allocInfo,
                              &buffer.m_buffer,
                              &buffer.m_allocation,
                              nullptr);

    return buffer;
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

VulkanBuffer VulkanDevice::GetStagingBuffer(u64 size)
{
    VulkanBuffer staging;
    if (auto stagingBufferIt = m_stagingBuffers.find(size);
        stagingBufferIt == m_stagingBuffers.end())
    {
        VkBufferCreateInfo stagingBufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        stagingBufferInfo.size        = size;
        stagingBufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo{};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        auto ok = vmaCreateBuffer(m_allocator,
                                  &stagingBufferInfo,
                                  &stagingAllocInfo,
                                  &staging.m_buffer,
                                  &staging.m_allocation,
                                  nullptr);
        assert(ok == VK_SUCCESS);

        m_stagingBuffers.insert({size, staging});
    }
    else
    {
        staging = m_stagingBuffers.at(size);
    }

    return staging;
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
