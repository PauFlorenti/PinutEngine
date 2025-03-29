#include "pch.hpp"

#include "vulkan/descriptorSetManager.h"
#include "vulkan/descriptorSetManager.h"
#include "vulkan/utils.h"
#include "vulkan/vulkanPipeline.h"

bool operator==(const VkDescriptorBufferInfo& lhs, const VkDescriptorBufferInfo& rhs)
{
    return lhs.buffer == rhs.buffer && lhs.offset == rhs.offset && lhs.range == rhs.range;
}

bool operator!=(const VkDescriptorBufferInfo& lhs, const VkDescriptorBufferInfo& rhs)
{
    return !(lhs == rhs);
}

bool operator==(const VkDescriptorImageInfo& lhs, const VkDescriptorImageInfo& rhs)
{
    return lhs.imageLayout == rhs.imageLayout && lhs.imageView == rhs.imageView &&
           lhs.sampler == rhs.sampler;
}

bool operator!=(const VkDescriptorImageInfo& lhs, const VkDescriptorImageInfo& rhs)
{
    return !(lhs == rhs);
}

namespace std
{
using namespace RED::vulkan;

size_t hash<VkDescriptorBufferInfo>::operator()(const VkDescriptorBufferInfo& bufferInfo) const
{
    size_t h1 = std::hash<VkBuffer>{}(bufferInfo.buffer);
    size_t h2 = std::hash<VkDeviceSize>{}(bufferInfo.offset);
    size_t h3 = std::hash<VkDeviceSize>{}(bufferInfo.range);

    return h1 ^ (h2 << 1) ^ (h3 << 2);
}

inline size_t std::hash<RED::vulkan::UniformDescriptorInfo>::operator()(
  const RED::vulkan::UniformDescriptorInfo& info) const noexcept
{
    return size_t();
}

} // namespace std

namespace RED
{
namespace vulkan
{
bool UniformDescriptorInfo::operator==(const UniformDescriptorInfo& other) const
{
    if (this->resources.size() != other.resources.size())
        return false;

    for (u32 i = 0; i < resources.size(); ++i)
    {
        const auto& resource      = resources.at(i);
        const auto& otherResource = other.resources.at(i);

        if (resource.binding != otherResource.binding ||
            resource.descriptorInfo != otherResource.descriptorInfo ||
            resource.type != otherResource.type)
            return false;
    }

    return true;
}
void DescriptorSetManager::Update()
{
    for (auto it = m_sets.begin(); it != m_sets.end();)
    {
        if (it->second.liveFrames > 600)
        {
            m_sets.erase(it++);
        }
        else
        {
            ++it->second.liveFrames;
        }

        ++it;
    }
}

void DescriptorSetManager::OnDestroy(VkDevice device)
{
    for (auto& pool : m_pools)
    {
        vkDestroyDescriptorPool(device, pool.second, nullptr);
    }

    m_pools.clear();
    m_sets.clear();
}

std::vector<VkDescriptorSet> DescriptorSetManager::GetDescriptorSet(
  VkDevice                         device,
  const UniformDescriptorSetInfos& uniformDescriptorSetInfo,
  const VulkanPipeline*            pipeline)
{
    u32                          setIndex             = 0;
    const auto&                  descriptorSetLayouts = pipeline->GetDescriptorSetLayouts();
    std::vector<VkDescriptorSet> sets;
    sets.reserve(2); // Should I use MAX_DESCRIPTOR_SETS ??

    for (const auto& layout : descriptorSetLayouts)
    {
        auto& layoutInfo = descriptorSetLayouts.at(setIndex);

        if (layoutInfo.bindings.empty())
        {
            ++setIndex;
            continue;
        }

        const auto it = m_sets.find(uniformDescriptorSetInfo.at(setIndex));
        if (it != m_sets.end())
        {
            it->second.liveFrames = 0;
            sets.emplace_back(it->second.set);
            ++setIndex;
            continue;
        }

        sets.emplace_back(Allocate(device, pipeline, setIndex));
        m_sets.insert({uniformDescriptorSetInfo.at(setIndex), {sets.back(), 0}});

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(MAX_UNIFORM_SLOTS);
        for (const auto& resource : uniformDescriptorSetInfo.at(setIndex).resources)
        {
            VkWriteDescriptorSet write{};
            write.sType          = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.pNext          = nullptr;
            write.dstSet         = sets.back();
            write.descriptorType = resource.type;
            write.dstBinding     = resource.binding;

            if (resource.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                if (const auto& bufferInfo =
                      std::get_if<std::vector<VkDescriptorBufferInfo>>(&resource.descriptorInfo))
                {
                    write.descriptorCount = static_cast<u32>(bufferInfo->size());
                    write.pBufferInfo     = bufferInfo->data();
                }
            }
            else if (resource.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
                if (const auto& imageInfo =
                      std::get_if<std::vector<VkDescriptorImageInfo>>(&resource.descriptorInfo))
                {
                    write.descriptorCount = static_cast<u32>(imageInfo->size());
                    write.pImageInfo      = imageInfo->data();
                }
            }

            writes.emplace_back(write);
        }

        if (!writes.empty())
        {
            vkUpdateDescriptorSets(device,
                                   static_cast<u32>(writes.size()),
                                   writes.data(),
                                   0,
                                   nullptr);
        }

        ++setIndex;
    }

    return sets;
}

VkDescriptorSet DescriptorSetManager::Allocate(VkDevice              device,
                                               const VulkanPipeline* pipeline,
                                               u32                   setIndex)
{
    if (auto it = m_pools.find(pipeline); it == m_pools.end())
    {
        const auto&                poolSizes = pipeline->GetDescriptorPoolSizes();
        VkDescriptorPoolCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        info.maxSets       = 1000;
        info.poolSizeCount = static_cast<u32>(poolSizes.size());
        info.pPoolSizes    = poolSizes.data();

        VkDescriptorPool pool;
        auto             ok = vkCreateDescriptorPool(device, &info, nullptr, &pool);
        assert(ok == VK_SUCCESS);

        m_pools.insert({pipeline, pool});
    }

    const auto& pool = m_pools.find(pipeline);
    assert(pool != m_pools.end());

    const auto& layouts = pipeline->GetDescriptorSetLayouts();
    auto        info =
      vkinit::DescriptorSetAllocateInfo(pool->second, 1, layouts.at(setIndex).descriptorSetLayout);

    VkDescriptorSet set;
    auto            ok = vkAllocateDescriptorSets(device, &info, &set);
    assert(ok == VK_SUCCESS);

    return set;
}

void DescriptorSetManager::Clear()
{
    // vkResetDescriptorPool(m_device, m_descriptorPool[m_frameIndex], 0);
    // m_frameIndex = m_frameIndex++ % m_numberBackBuffers;
}
} // namespace vulkan
} // namespace RED
