#pragma once

#include <vulkan/vulkan.h>

namespace Pinut
{
class DescriptorSetManager
{
  public:
    void OnCreate(VkDevice                          device,
                  u32                               numberBackBuffers,
                  u32                               initialSets,
                  std::vector<VkDescriptorPoolSize> sizes);
    void OnDestroy();

    VkDescriptorSet Allocate(VkDescriptorSetLayout) const;
    void            Clear();

  private:
    VkDevice          m_device{VK_NULL_HANDLE};
    VkDescriptorPool* m_descriptorPool{nullptr};
    u32               m_frameIndex{0};
    u32               m_numberBackBuffers{0};
};
} // namespace Pinut
