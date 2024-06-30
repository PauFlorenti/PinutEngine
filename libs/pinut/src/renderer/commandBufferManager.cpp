#include "stdafx.h"

#include "commandBufferManager.h"
#include "device.h"
#include "utils.h"

namespace Pinut
{
void CommandBufferManager::OnCreate(Device* device,
                                    u32     numberBackBuffers,
                                    u32     numberCommandsToAllocate)
{
    assert(device);
    m_device        = device->GetDevice();
    auto queueIndex = device->GetGraphicsQueueIndex();

    m_numberBackBuffers           = std::move(numberBackBuffers);
    m_numberCommandsPerBackBuffer = std::move(numberCommandsToAllocate);

    m_commandBuffersArray = new CommandBufferPerFrame[m_numberBackBuffers];

    for (u32 i = 0; i < m_numberBackBuffers; ++i)
    {
        auto cmdsPerFrame                        = &m_commandBuffersArray[i];
        cmdsPerFrame->m_numberCommandBuffersUsed = 0;

        auto info = vkinit::CommandPoolCreateInfo(queueIndex,
                                                  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                                                    VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

        auto ok = vkCreateCommandPool(m_device, &info, nullptr, &cmdsPerFrame->m_commandPool);
        assert(ok == VK_SUCCESS);

        cmdsPerFrame->m_commandBuffers = new VkCommandBuffer[m_numberCommandsPerBackBuffer];

        auto allocateInfo = vkinit::CommandBufferAllocateInfo(cmdsPerFrame->m_commandPool,
                                                              VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                              m_numberCommandsPerBackBuffer);
        ok = vkAllocateCommandBuffers(m_device, &allocateInfo, cmdsPerFrame->m_commandBuffers);
        assert(ok == VK_SUCCESS);
    }
}

void CommandBufferManager::OnDestroy()
{
    for (u32 i = 0; i < m_numberBackBuffers; ++i)
    {
        auto cmds = m_commandBuffersArray[i];
        vkFreeCommandBuffers(m_device,
                             cmds.m_commandPool,
                             cmds.m_numberCommandBuffersUsed,
                             cmds.m_commandBuffers);
        vkDestroyCommandPool(m_device, cmds.m_commandPool, nullptr);
    }

    delete[] m_commandBuffersArray;
}

void CommandBufferManager::OnBeginFrame()
{
    m_currentCommandBuffers = &m_commandBuffersArray[m_frameIndex % m_numberBackBuffers];
    m_currentCommandBuffers->m_numberCommandBuffersUsed = 0;
    ++m_frameIndex;
}

VkCommandBuffer CommandBufferManager::GetNewCommandBuffer()
{
    auto cmd = m_currentCommandBuffers
                 ->m_commandBuffers[m_currentCommandBuffers->m_numberCommandBuffersUsed++];
    assert(m_currentCommandBuffers->m_numberCommandBuffersUsed < m_numberCommandsPerBackBuffer);
    return cmd;
}

} // namespace Pinut
