// Some comment
#include "stdafx.h"

#include "buffer.h"

#include "device.h"

namespace Pinut
{
    void Buffer::Create(Device *inDevice, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage)
    {
        device = inDevice;

        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = bufferUsage,
        };

        VmaAllocationCreateInfo allocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = memoryUsage};

        auto ok = vmaCreateBuffer(device->GetAllocator(), &bufferInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo);
        assert(ok == VK_SUCCESS);
    }

    void Buffer::Destroy()
    {
        vmaDestroyBuffer(device->GetAllocator(), buffer, allocation);
    }
}
