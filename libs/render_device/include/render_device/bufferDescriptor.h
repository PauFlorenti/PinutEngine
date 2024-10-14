#pragma once

namespace RED
{
struct BufferDescriptor
{
    u64                size{0};
    u64                elementSize{0};
    VkBufferUsageFlags usage;
};
} // namespace RED
