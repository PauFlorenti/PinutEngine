// Some comments
#include "stdafx.h"

#include "commandBufferRing.h"

#include "device.h"

namespace Pinut
{
    void CommandBufferRing::OnCreate(Device *device, const uint32_t &nBackBuffers, const uint32_t &nCommandsAllocated)
    {
        deviceHandle = device;
        numberBackBuffers = nBackBuffers;
        numberCommandsAllocatedPerBackBuffer = nCommandsAllocated;

        commandBuffersArray = new CommandBuffersPerFrame[nBackBuffers]();

        for (uint32_t i = 0; i < numberBackBuffers; i++)
        {
            CommandBuffersPerFrame* commandBufferPerFrame = &commandBuffersArray[i];

            VkCommandPoolCreateInfo commandPoolInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = device->GetGraphicsQueueFamilyIndex(),
            };

            auto ok = vkCreateCommandPool(deviceHandle->GetDevice(), &commandPoolInfo, nullptr, &commandBufferPerFrame->commandPool);
            assert(ok == VK_SUCCESS);

            commandBufferPerFrame->commandBuffers = new VkCommandBuffer[numberCommandsAllocatedPerBackBuffer]();

            VkCommandBufferAllocateInfo commandBufferInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandBufferPerFrame->commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = numberCommandsAllocatedPerBackBuffer,
            };

            ok = vkAllocateCommandBuffers(deviceHandle->GetDevice(), &commandBufferInfo, commandBufferPerFrame->commandBuffers);
            assert(ok == VK_SUCCESS);
            commandBufferPerFrame->numberCommandBuffersUsed = 0;
        }

        frameIndex = 0;
        currentFrameCommandBuffers = &commandBuffersArray[frameIndex % numberBackBuffers];
        frameIndex++;
        currentFrameCommandBuffers->numberCommandBuffersUsed = 0;
    }

    void CommandBufferRing::OnDestroy()
    {
        for (uint32_t i = 0; i < numberBackBuffers; i++)
        {
            vkFreeCommandBuffers(deviceHandle->GetDevice(), commandBuffersArray[i].commandPool, numberCommandsAllocatedPerBackBuffer, commandBuffersArray[i].commandBuffers);
            vkDestroyCommandPool(deviceHandle->GetDevice(), commandBuffersArray[i].commandPool, nullptr);
        }

        delete[] commandBuffersArray;
    }

    VkCommandBuffer CommandBufferRing::GetNewCommandBuffer()
    {
        auto commandBuffer = currentFrameCommandBuffers->commandBuffers[currentFrameCommandBuffers->numberCommandBuffersUsed++];

        assert(currentFrameCommandBuffers->numberCommandBuffersUsed < numberCommandsAllocatedPerBackBuffer);

        return commandBuffer;
    }

    void CommandBufferRing::OnBeginFrame()
    {
        currentFrameCommandBuffers = &commandBuffersArray[frameIndex % numberBackBuffers];
        currentFrameCommandBuffers->numberCommandBuffersUsed = 0;
        frameIndex++;
    }
}
