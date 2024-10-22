#include "pch.hpp"

#define VMA_IMPLEMENTATION
#define VMA_DEBUG_LOG_FORMAT
#include <external/VulkanMemoryAllocator/include/vk_mem_alloc.h>

#include "render_device/bufferDescriptor.h"
#include "render_device/drawCall.h"
#include "render_device/renderPipeline.h"
#include "src/vulkan/utils.h"
#include "src/vulkan/vulkanDevice.h"
#include "src/vulkan/vulkanPipeline.h"

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

    m_rendererContext = vulkanCallbacks->context;
    m_beginFrame_fn   = vulkanCallbacks->BeginFrame_fn;
    m_endFrame_fn     = vulkanCallbacks->EndFrame_fn;
    m_present_fn      = vulkanCallbacks->Present_fn;

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

    std::vector<VkDescriptorPoolSize> poolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
    };
    m_descriptorSetManager.OnCreate(m_device, MAX_FRAMES_IN_FLIGHT, 2, std::move(poolSizes));
}

void VulkanDevice::OnDestroy()
{
    vkDeviceWaitIdle(m_device);

    for (auto& pipeline : m_pipelines)
        pipeline.second.Destroy(m_device);

    m_pipelines.clear();

    for (auto& buffer : m_buffers)
        vmaDestroyBuffer(m_allocator, buffer.second.m_buffer, buffer.second.m_allocation);

    m_buffers.clear();

    m_descriptorSetManager.OnDestroy();

    for (auto& uniformBuffers : m_globalUniformBuffers)
        vmaDestroyBuffer(m_allocator, uniformBuffers.m_buffer, uniformBuffers.m_allocation);

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
        }
    }

    vkDestroyCommandPool(m_device, m_immediateCommandPool, nullptr);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroyFence(m_device, m_frameCompletedFences[i], nullptr);
        vkDestroySemaphore(m_device, m_imagesAvailableSemaphores[i], nullptr);
    }

    vmaDestroyAllocator(m_allocator);

    if (m_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
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

    BeginCommandRecording(QueueType::GRAPHICS);
}

void VulkanDevice::EndFrame()
{
    EndCommandRecording(true, true);

    if (m_lastCommandBuffer && m_lastCommandBuffer->queueType != QueueType::COUNT)
    {
        m_endFrame_fn(m_rendererContext, m_lastCommandBuffer->signalSemaphore);
    }

    m_currentCommandBuffer          = nullptr;
    m_lastCommandBuffer             = nullptr;
    m_currentRenderPipeline         = nullptr;
    m_currentRenderPipelineInternal = nullptr;
}

void VulkanDevice::Present() { m_present_fn(); }

void VulkanDevice::EnableRendering(const VkRect2D&                               renderArea,
                                   const std::vector<VkRenderingAttachmentInfo>& attachments)
{
    BeginCommandRecording(QueueType::GRAPHICS);

    VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
    info.colorAttachmentCount = static_cast<u32>(attachments.size());
    info.pColorAttachments    = attachments.data();
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

void VulkanDevice::DestroyBuffer(BufferResource resource)
{
    if (auto buffer = m_buffers.find(resource); buffer != m_buffers.end())
    {
        auto b = buffer->second;
        vmaDestroyBuffer(m_allocator, b.m_buffer, b.m_allocation);

        m_buffers.erase(buffer);
    }
}

void VulkanDevice::WaitIdle() const { vkDeviceWaitIdle(m_device); }

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

VulkanBuffer VulkanDevice::GetVulkanBuffer(BufferResource bufferResource)
{
    auto buffer = m_buffers.find(bufferResource);
    if (buffer != m_buffers.end())
        return buffer->second;

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

    // Bind vertex buffers
    const auto& buffer = m_buffers.find(drawCall.vertexBuffer.GetID());
    assert(buffer != m_buffers.end());

    VkDeviceSize offset{0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer->second.m_buffer, &offset);

    // Uniforms
    if (!drawCall.uniforms.empty())
    {
        const std::array<Shader, 2> shaders{m_currentRenderPipeline->vertexShader,
                                            m_currentRenderPipeline->fragmentShader};

        std::array<std::vector<VkDescriptorBufferInfo>, MAX_DESCRIPTOR_SETS> descriptorSetInfos;
        for (const auto& uniform : drawCall.uniforms)
        {
            if (const auto& bufferId = uniform.buffer.GetID();
                bufferId.id != GPU_RESOURCE_INVALID && bufferId.type == ResourceType::BUFFER)
            {
                const auto             buffer = GetVulkanBuffer(bufferId);
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = buffer.m_buffer;
                bufferInfo.offset = 0;
                bufferInfo.range  = buffer.m_descriptor.size;

                descriptorSetInfos.at(uniform.set).emplace_back(std::move(bufferInfo));
            }
        }

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(MAX_DESCRIPTOR_SETS * MAX_UNIFORM_SLOTS);
        std::vector<VkDescriptorSet> sets;
        sets.reserve(MAX_DESCRIPTOR_SETS);
        u32 descriptorSetIndex{0};
        for (const auto& descriptorSetLayout : pipeline->GetDescriptorSetLayouts())
        {
            if (descriptorSetLayout.descriptorSetLayout == VK_NULL_HANDLE)
                continue;

            auto set = m_descriptorSetManager.Allocate(descriptorSetLayout.descriptorSetLayout);
            sets.emplace_back(set);

            const auto& bindings = descriptorSetInfos.at(descriptorSetIndex);

            u32 bindingIndex{0};
            for (const auto& binding : bindings)
            {
                VkWriteDescriptorSet write{
                  .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                  .dstSet          = set,
                  .dstBinding      = bindingIndex,
                  .descriptorCount = 1,
                  .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                  .pBufferInfo     = &binding,
                };

                writes.emplace_back(write);

                ++bindingIndex;
            }
            ++descriptorSetIndex;
        }

        vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline->GetPipelineLayout(),
                                0,
                                static_cast<u32>(sets.size()),
                                sets.data(),
                                0,
                                nullptr);

        vkUpdateDescriptorSets(m_device,
                               static_cast<u32>(writes.size()),
                               writes.data(),
                               0,
                               nullptr);
    }

    vkCmdDraw(cmd, drawCall.vertexCount, 1, 0, 0);
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

void VulkanDevice::TransitionImageLayout(VkImage                 image,
                                         VkAccessFlags           srcAccessFlags,
                                         VkAccessFlags           dstAccessFlags,
                                         VkImageLayout           currentLayout,
                                         VkImageLayout           targetLayout,
                                         VkPipelineStageFlags    srcStageFlags,
                                         VkPipelineStageFlags    dstStageFlags,
                                         VkImageSubresourceRange subresourceRange)
{
    auto cmd = BeginImmediateCommandBuffer(m_immediateCommandPool);

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

    FlushImmediateCommandBuffer(cmd, m_queues[static_cast<u32>(QueueType::GRAPHICS)].queue);
}
} // namespace vulkan
} // namespace RED
