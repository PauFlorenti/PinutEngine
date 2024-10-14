#pragma once

#include <external/VulkanMemoryAllocator/include/vk_mem_alloc.h>

typedef struct VmaAllocation_T* VmaAllocation;
namespace RED
{
namespace vulkan
{
struct VulkanBuffer
{
    BufferResource m_id;
    VmaAllocation  m_allocation;
    VkBuffer       m_buffer;
};
} // namespace vulkan
} // namespace RED
