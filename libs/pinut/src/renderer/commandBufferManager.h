#pragma once

#include <vulkan/vulkan.h>

namespace Pinut
{
class Device;
class CommandBufferManager
{
  public:
    void OnCreate(Device* device, u32 numberBackBuffers, u32 numberCommandsToAllocate = 8);
    void OnDestroy();
    void OnBeginFrame();
    VkCommandBuffer GetNewCommandBuffer();

  private:
    struct CommandBufferPerFrame
    {
        VkCommandPool    m_commandPool{VK_NULL_HANDLE};
        VkCommandBuffer* m_commandBuffers{nullptr};
        u32              m_numberCommandBuffersUsed{0};
    };

    CommandBufferPerFrame* m_commandBuffersArray{nullptr};
    CommandBufferPerFrame* m_currentCommandBuffers{nullptr};

    VkDevice m_device{VK_NULL_HANDLE};
    u32      m_frameIndex{0};
    u32      m_numberBackBuffers{0};
    u32      m_numberCommandsPerBackBuffer{0};
};
} // namespace Pinut
