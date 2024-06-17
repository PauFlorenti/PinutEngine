#pragma once

namespace Pinut
{
    class Device;
    class DescriptorSetManager
    {
    public:
        void OnCreate(Device *inDevice, const uint32_t initialSets, std::vector<VkDescriptorPoolSize> sizes);
        void OnDestroy();

        VkDescriptorSet Allocate(VkDescriptorSetLayout layout) const;
        void Clear();

    private:
        VkDescriptorPool CreatePool(const uint32_t setCount, std::vector<VkDescriptorPoolSize> poolSizes);

        Device *device{nullptr};
        VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
    };
}
