#include "pch.hpp"

#include "src/descriptorSetManager.h"
#include "src/init_utils.h"

namespace vulkan
{
void DescriptorSetManager::OnCreate(VkDevice                          device,
                                    u32                               numberBackBuffers,
                                    u32                               initialSets,
                                    std::vector<VkDescriptorPoolSize> sizes)
{
    assert(device);
    m_device = device;

    m_numberBackBuffers = std::move(numberBackBuffers);

    m_descriptorPool        = new VkDescriptorPool[numberBackBuffers];
    auto descriptorPoolInfo = vkinit::DescriptorPoolCreateInfo(initialSets, sizes);

    for (u32 i = 0; i < m_numberBackBuffers; ++i)
    {
        vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &m_descriptorPool[i]);
    }
}

void DescriptorSetManager::OnDestroy()
{
    for (u32 i = 0; i < m_numberBackBuffers; ++i)
    {
        vkDestroyDescriptorPool(m_device, m_descriptorPool[i], nullptr);
    }
}

VkDescriptorSet DescriptorSetManager::Allocate(VkDescriptorSetLayout layout) const
{
    auto info = vkinit::DescriptorSetAllocateInfo(m_descriptorPool[m_frameIndex], 1, layout);

    VkDescriptorSet set;
    auto            ok = vkAllocateDescriptorSets(m_device, &info, &set);
    assert(ok == VK_SUCCESS);
    return set;
}

void DescriptorSetManager::Clear()
{
    vkResetDescriptorPool(m_device, m_descriptorPool[m_frameIndex], 0);
    m_frameIndex = m_frameIndex++ % m_numberBackBuffers;
}
} // namespace vulkan
