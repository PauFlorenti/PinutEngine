#pragma once

#include "render_device/vulkan/vulkanBuffer.h"

namespace RED
{
namespace vulkan
{
class StagingBufferAllocator
{
  public:
    StagingBufferAllocator() = delete;
    explicit StagingBufferAllocator(VulkanBuffer buffer, VkDeviceSize totalSize);
    ~StagingBufferAllocator();

    VkDeviceSize GetFreeSize() const;
    VkDeviceSize GetTotalSize() const;

    std::pair<VkDeviceSize, VulkanBuffer> Allocate(VkDeviceSize size);
    void         Free(VkDeviceSize offset, VkDeviceSize size);

  private:
    VkDeviceSize                                     m_totalSize{0};
    VkDeviceSize                                     m_freeSize{0};
    VulkanBuffer                                     m_buffer;
    std::list<std::pair<VkDeviceSize, VkDeviceSize>> m_freeSpace;
};
} // namespace vulkan
} // namespace RED
