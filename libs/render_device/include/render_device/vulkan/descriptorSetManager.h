#pragma once

bool operator==(const VkDescriptorBufferInfo& lhs, const VkDescriptorBufferInfo& rhs);
bool operator!=(const VkDescriptorBufferInfo& lhs, const VkDescriptorBufferInfo& rhs);

namespace RED::vulkan
{
struct UniformDescriptorInfo;
}
namespace std
{
template <>
struct hash<RED::vulkan::UniformDescriptorInfo>
{
    size_t operator()(const RED::vulkan::UniformDescriptorInfo& info) const noexcept;
};
} // namespace std

namespace RED
{
namespace vulkan
{
class VulkanPipeline;
struct UniformDescriptorInfo
{
    struct UniformResource
    {
        std::variant<std::vector<VkDescriptorBufferInfo>, std::vector<VkDescriptorImageInfo>>
                         descriptorInfo;
        i32              binding{-1};
        VkDescriptorType type;
    };

    std::vector<UniformResource> resources;

    bool operator==(const UniformDescriptorInfo&) const;
};

struct DescriptorSet
{
    VkDescriptorSet set{VK_NULL_HANDLE};
    u32             liveFrames{0};
};

using UniformDescriptorSetInfos =
  std::array<UniformDescriptorInfo, 2>; // TODO Should I use MAX_DESCRIPTOR_SETS ??

class DescriptorSetManager
{
  public:
    void Update();
    void OnDestroy(VkDevice device);

    std::vector<VkDescriptorSet> GetDescriptorSet(
      VkDevice                         device,
      const UniformDescriptorSetInfos& uniformDescriptorSetInfo,
      const VulkanPipeline*            pipeline);
    void Clear();

  private:
    VkDescriptorSet Allocate(VkDevice device, const VulkanPipeline* pipeline, u32 set);

    std::unordered_map<const VulkanPipeline*, VkDescriptorPool> m_pools;
    std::unordered_map<UniformDescriptorInfo, DescriptorSet>    m_sets;
};
} // namespace vulkan
} // namespace RED

namespace std
{
using namespace RED::vulkan;

template <>
struct hash<VkDescriptorBufferInfo>
{
    size_t operator()(const VkDescriptorBufferInfo& bufferInfo) const;
};
} // namespace std
