#pragma once

#include "vk_mem_alloc.h"

namespace Pinut
{
    class Device;
    struct Buffer
    {
        void Create(Device* inDevice, size_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
        void Destroy();

        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;

    private:
        Device* device{nullptr};
    };
}
