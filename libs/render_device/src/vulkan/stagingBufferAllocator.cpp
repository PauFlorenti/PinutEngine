#include "vulkan/stagingBufferAllocator.h"

namespace RED
{
namespace vulkan
{
StagingBufferAllocator::StagingBufferAllocator(VulkanBuffer buffer, VkDeviceSize size)
: m_buffer(std::move(buffer)),
  m_totalSize(size),
  m_freeSize(size)
{
    m_freeSpace.emplace_back(0, m_totalSize);
}

StagingBufferAllocator::~StagingBufferAllocator()
{
}

VkDeviceSize StagingBufferAllocator::GetTotalSize() const { return m_totalSize; }
VkDeviceSize StagingBufferAllocator::GetFreeSize() const { return m_freeSize; }

std::pair<VkDeviceSize, VulkanBuffer> StagingBufferAllocator::Allocate(VkDeviceSize size)
{
    for (auto it = m_freeSpace.begin(); it != m_freeSpace.end(); ++it)
    {
        if (it->second >= size)
        {
            VkDeviceSize offset = it->first;

            if (it->second > size)
            {
                it->first += size;
                it->second -= size;
            }
            else
            {
                m_freeSpace.erase(it);
            }

            m_freeSize -= size;
            return {offset, m_buffer};
        }
    }

    return {std::numeric_limits<VkDeviceSize>::max(), {}};
}

void StagingBufferAllocator::Free(VkDeviceSize offset, VkDeviceSize size)
{
    auto it = m_freeSpace.begin();
    for (; it != m_freeSpace.end(); ++it)
    {
        if (it->first > offset)
            break;
    }

    if (it != m_freeSpace.begin() && std::prev(it)->first + std::prev(it)->second == offset)
    {
        std::prev(it)->second += size;
        if (it != m_freeSpace.end() && std::prev(it)->first + std::prev(it)->second == it->first)
        {
            std::prev(it)->second += it->second;
            m_freeSpace.erase(it);
        }
    }
    else
    {
        m_freeSpace.insert(it, {offset, size});
    }

    m_freeSize += size;
}
} // namespace vulkan
} // namespace RED
