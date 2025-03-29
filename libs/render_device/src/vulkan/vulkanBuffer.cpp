#include "pch.hpp"

#include "vulkan/vulkanBuffer.h"

namespace RED
{
namespace vulkan
{
VkBufferUsageFlags GetVulkanBufferUsage(BufferUsage usage)
{
    VkBufferUsageFlags flags = 0;

    if (usage & BufferUsage::VERTEX)
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (usage & BufferUsage::INDEX)
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (usage & BufferUsage::UNIFORM)
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (usage & BufferUsage::STORAGE)
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (usage == BufferUsage::NONE)
    {
        flags = 0; // No flags set
    }

    return flags;
}
} // namespace vulkan
} // namespace RED
