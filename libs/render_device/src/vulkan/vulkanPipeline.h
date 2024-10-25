#pragma once

#include "render_device/shader.h"

namespace RED
{
struct RenderPipeline;
struct GraphicsState;
namespace vulkan
{
class VulkanPipeline
{
    struct DescriptorSetLayout
    {
        VkDescriptorSetLayout                     descriptorSetLayout{VK_NULL_HANDLE};
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

    using DescriptorSetLayouts = std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS>;

  public:
    VulkanPipeline();
    ~VulkanPipeline();
    VulkanPipeline(const VulkanPipeline&);
    // VulkanPipeline(VulkanPipeline&&)                 = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(VulkanPipeline&&)      = delete;

    static VulkanPipeline Create(VkDevice              device,
                                 const RenderPipeline& pipeline,
                                 const GraphicsState&  graphicsState);

    bool operator==(const VulkanPipeline&) const noexcept = default;

    void Destroy(VkDevice device);

    VkPipeline                               GetPipeline() const;
    VkPipelineLayout                         GetPipelineLayout() const;
    DescriptorSetLayouts                     GetDescriptorSetLayouts() const;
    const std::vector<VkDescriptorPoolSize>& GetDescriptorPoolSizes() const;

  protected:
    static VkShaderModule CreateShaderModule(VkDevice device, const char* filename);
    static void           AddDescriptorSetLayoutBindings(
                const Shader&                                                               shader,
                std::array<std::vector<VkDescriptorSetLayoutBinding>, MAX_DESCRIPTOR_SETS>& bindings);
    static DescriptorSetLayouts CreateDescriptorSetLayouts(VkDevice              device,
                                                           const RenderPipeline& pipeline);

  private:
    VkPipeline                        m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout                  m_pipelineLayout{VK_NULL_HANDLE};
    DescriptorSetLayouts              m_descriptorSetLayouts;
    std::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
};
} // namespace vulkan
} // namespace RED
