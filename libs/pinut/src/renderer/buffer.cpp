#include "stdafx.h"

#include "buffer.h"
#include "device.h"

namespace Pinut
{
void GPUBuffer::Create(Device*            device,
                       size_t             size,
                       VkBufferUsageFlags bufferUsageFlags,
                       VmaMemoryUsage     memoryUsage)
{
    assert(device);
    m_device = device;

    VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size  = size,
      .usage = bufferUsageFlags,
    };

    VmaAllocationCreateInfo allocationCreateInfo{
      .flags =
        VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      .usage = memoryUsage,
    };

    auto ok = vmaCreateBuffer(device->GetAllocator(),
                              &bufferInfo,
                              &allocationCreateInfo,
                              &m_buffer,
                              &m_allocation,
                              &m_allocationInfo);
    assert(ok == VK_SUCCESS);
}

void GPUBuffer::Destroy()
{
    vmaDestroyBuffer(m_device->GetAllocator(), m_buffer, m_allocation);
    m_buffer         = VK_NULL_HANDLE;
    m_device         = nullptr;
    m_allocation     = nullptr;
    m_allocationInfo = {};
}

VmaAllocation            GPUBuffer::Allocation() const { return m_allocation; }
const VmaAllocationInfo& GPUBuffer::AllocationInfo() const { return m_allocationInfo; }

} // namespace Pinut
