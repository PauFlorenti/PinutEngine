#pragma once

#include "vulkan/vulkan.h"

namespace Pinut
{
    class Device;

    // This class manages the number of commandBuffers allocated per each frame
    // It has fixed commandBuffers per each frame.
    class CommandBufferRing
    {
    public:
        void OnCreate(Device *device, const uint32_t &nBackBuffers, const uint32_t &nCommandsAllocated);
        void OnDestroy();
        void OnBeginFrame(); // Resets the current command buffers allocated for this frame
        VkCommandBuffer GetNewCommandBuffer();
        VkCommandPool GetPool() { return currentFrameCommandBuffers->commandPool; }

    private:
        Device *deviceHandle{nullptr};

        uint32_t frameIndex{0};
        uint32_t numberBackBuffers;
        uint32_t numberCommandsAllocatedPerBackBuffer;

        struct CommandBuffersPerFrame
        {
            VkCommandPool commandPool;
            VkCommandBuffer *commandBuffers;
            uint32_t numberCommandBuffersUsed{0};
        } *commandBuffersArray, *currentFrameCommandBuffers;
    };
}
