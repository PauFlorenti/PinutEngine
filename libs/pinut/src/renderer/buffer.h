#pragma once

#include "vk_mem_alloc.h"

namespace Pinut
{
class Device;
class GPUBuffer
{
  public:
    void Create(Device*            device,
                size_t             size,
                VkBufferUsageFlags bufferUsageFlags,
                VmaMemoryUsage     memoryUsage);
    void Destroy();

    VkBuffer                 Buffer() const;
    VmaAllocation            Allocation() const;
    const VmaAllocationInfo& AllocationInfo() const;

  private:
    Device*           m_device{nullptr};
    VkBuffer          m_buffer{VK_NULL_HANDLE};
    VmaAllocation     m_allocation{nullptr};
    VmaAllocationInfo m_allocationInfo{};
};
} // namespace Pinut
