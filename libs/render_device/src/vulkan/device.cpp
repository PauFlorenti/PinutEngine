#include "pch.hpp"

#define VMA_IMPLEMENTATION
#define VMA_DEBUG_LOG_FORMAT
#include <external/VulkanMemoryAllocator/include/vk_mem_alloc.h>

#include "src/drawCall.h"
#include "src/renderPipeline.h"
#include "src/vulkan/device.h"
#include "src/vulkan/pipeline.h"

namespace vulkan
{
bool   PipelineKey::operator==(const PipelineKey&) const noexcept = default;

Device::Device(DeviceInfo* deviceInfo, QueueInfo* queues, DeviceCallbacks* callbacks)
{
    assert(deviceInfo);
    assert(queues);

    m_device         = deviceInfo->device;
    m_physicalDevice = deviceInfo->physicalDevice;

    m_rendererContext = callbacks->context;
    m_beginFrame_fn   = callbacks->BeginFrame_fn;
    m_endFrame_fn     = callbacks->EndFrame_fn;

    /*
      Queues
      1 Graphics
      2 Present - used by renderer, not here
      3 Compute
    */
    m_queues[static_cast<u32>(QueueType::GRAPHICS)] = queues[0];
    m_queues[static_cast<u32>(QueueType::COMPUTE)]  = queues[2];

    VmaAllocatorCreateInfo allocator_info{
      .flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = m_physicalDevice,
      .device         = m_device,
      .instance       = deviceInfo->instance,
    };

    vmaCreateAllocator(&allocator_info, &m_allocator);

    m_immediateCommandPool = CreateCommandPool(queues[static_cast<u32>(QueueType::GRAPHICS)].index);

    for (auto& commandBufferSet : m_commandBufferSets)
    {
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            commandBufferSet.commandPools.at(i) =
              CreateCommandPool(queues[static_cast<u32>(QueueType::GRAPHICS)].index);
            commandBufferSet.commandBufferIndex.at(i) = -1;
        }
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        m_frameCompletedFences.at(i)      = VK_NULL_HANDLE;
        m_imagesAvailableSemaphores.at(i) = CreateSemaphore();
    }
}

void Device::OnDestroy()
{
    vkDeviceWaitIdle(m_device);

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

void Device::BeginFrame()
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

void Device::EndFrame()
{
    EndCommandRecording(true, true);

    if (m_lastCommandBuffer && m_lastCommandBuffer->queueType != QueueType::COUNT)
    {
        m_endFrame_fn(m_rendererContext, m_lastCommandBuffer->signalSemaphore);
    }

    m_currentCommandBuffer  = nullptr;
    m_lastCommandBuffer     = nullptr;
    m_currentRenderPipeline = nullptr;
}

void Device::EnableRendering(const VkRect2D&                               renderArea,
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

void Device::DisableRendering()
{
    const auto& cmd = m_currentCommandBuffer->commandBuffer;
    vkCmdEndRendering(cmd);
}

void Device::SetGraphicsState(GraphicsState* state) { m_currentGraphicsState = *state; }

void Device::SetRenderPipeline(RenderPipeline* pipeline) { m_currentRenderPipeline = pipeline; }

void Device::SubmitDrawCalls(const std::vector<DrawCall>& drawCalls)
{
    for (const auto& dc : drawCalls)
    {
        SubmitDrawCall(dc);
    }
}

void Device::WaitIdle() const { vkDeviceWaitIdle(m_device); }

void Device::BeginCommandRecording(QueueType type)
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

void Device::EndCommandRecording(bool waitForImage, bool signalFence)
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

VkFence Device::CreateFence(VkFenceCreateFlags flags) const
{
    VkFence           fence{VK_NULL_HANDLE};
    VkFenceCreateInfo info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, VK_NULL_HANDLE, flags};
    auto              ok = vkCreateFence(m_device, &info, nullptr, &fence);
    assert(ok == VK_SUCCESS);
    return fence;
}

VkSemaphore Device::CreateSemaphore(VkSemaphoreCreateFlags flags) const
{
    VkSemaphore           semaphore{VK_NULL_HANDLE};
    VkSemaphoreCreateInfo info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, VK_NULL_HANDLE, flags};
    vkCreateSemaphore(m_device, &info, nullptr, &semaphore);

    return semaphore;
}

VkCommandPool Device::CreateCommandPool(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags)
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

VkCommandBuffer Device::CreateCommandBuffer(const VkCommandPool& commandPool)
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

Pipeline* Device::GetRenderPipeline(const RenderPipeline* pipeline,
                                    const GraphicsState&  graphicsState)
{
    //auto key = PipelineKey{pipeline, graphicsState};

    //auto it = m_pipelines.find(key);
    //if (it != m_pipelines.end())
    //    return &it->second;

    //Pipeline renderPipeline = Pipeline::Create(m_device, *pipeline, graphicsState);

    //m_pipelines.insert({key, renderPipeline});
    //auto p = m_pipelines.find(key);
    //if (p != m_pipelines.end())
    //    return &p->second;

    return nullptr;
}

void Device::SubmitDrawCall(const DrawCall& drawCall)
{
    auto pipeline = GetRenderPipeline(m_currentRenderPipeline, m_currentGraphicsState);
}

VkCommandBuffer Device::BeginImmediateCommandBuffer(VkCommandPool commandPool)
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

void Device::FlushImmediateCommandBuffer(VkCommandBuffer cmd, VkQueue queue, VkFence fence) const
{
    vkEndCommandBuffer(cmd);

    VkSubmitInfo info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    info.commandBufferCount = 1;
    info.pCommandBuffers    = &cmd;

    auto ok = vkQueueSubmit(queue, 1, &info, fence);
    assert(ok == VK_SUCCESS);
    vkQueueWaitIdle(queue);
}

void Device::TransitionImageLayout(VkImage                 image,
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
