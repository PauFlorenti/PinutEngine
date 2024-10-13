// #pragma once

// namespace vulkan
// {
// class Device;
// class GPUBuffer
// {
//   public:
//     void Create(Device*            device,
//                 size_t             size,
//                 VkBufferUsageFlags bufferUsageFlags,
//                 VmaMemoryUsage     memoryUsage = VMA_MEMORY_USAGE_AUTO);
//     void Destroy();

//     VmaAllocation            Allocation() const;
//     const VmaAllocationInfo& AllocationInfo() const;

//     VkBuffer m_buffer{VK_NULL_HANDLE};

//   private:
//     Device*           m_device{nullptr};
//     VmaAllocation     m_allocation{nullptr};
//     VmaAllocationInfo m_allocationInfo{};
// };
// } // namespace vulkan
