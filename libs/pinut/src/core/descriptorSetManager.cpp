// Some comment

#include "stdafx.h"

#include "descriptorSetManager.h"

#include "src/core/device.h"

namespace Pinut
{
    void DescriptorSetManager::OnCreate(Device *inDevice, const uint32_t initialSets, std::vector<VkDescriptorPoolSize> sizes)
    {
        device = inDevice;
        descriptorPool = CreatePool(initialSets, std::move(sizes));
    }

    void DescriptorSetManager::OnDestroy()
    {
        vkDestroyDescriptorPool(device->GetDevice(), descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }

    VkDescriptorSet DescriptorSetManager::Allocate(VkDescriptorSetLayout layout) const
    {
        VkDescriptorSetAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout};

        VkDescriptorSet set;
        auto ok = vkAllocateDescriptorSets(device->GetDevice(), &info, &set);
        assert(ok == VK_SUCCESS);

        return set;
    }

    void DescriptorSetManager::Clear()
    {
        vkResetDescriptorPool(device->GetDevice(), descriptorPool, 0);
    }

    VkDescriptorPool DescriptorSetManager::CreatePool(const uint32_t setCount, std::vector<VkDescriptorPoolSize> poolSizes)
    {
        VkDescriptorPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = setCount,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
        };

        VkDescriptorPool pool;
        vkCreateDescriptorPool(device->GetDevice(), &info, nullptr, &pool);
        return pool;
    }
}
