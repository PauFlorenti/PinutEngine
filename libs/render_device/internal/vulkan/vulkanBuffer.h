#pragma once

#include "render_device/bufferDescriptor.h"

namespace RED
{
namespace vulkan
{
struct VulkanBuffer
{
    BufferDescriptor m_descriptor;
    VmaAllocation    m_allocation;
    VkBuffer         m_buffer;
};

VkBufferUsageFlags GetVulkanBufferUsage(BufferUsage usage);
} // namespace vulkan
} // namespace RED
