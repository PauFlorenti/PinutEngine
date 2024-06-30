#pragma once

#include <vulkan/vulkan.h>

namespace Pinut
{
class Device;
class ForwardPipeline
{
  public:
    void Init(Device* device);
    void Render();
    void Shutdown();

    VkPipeline       Pipeline() const;
    VkPipelineLayout PipelineLayout() const;

    // TODO Temporal
    VkDescriptorSetLayout PerFrameDescriptorSetLayout() const { return m_perFrameDescriptorSetLayout; }
    VkDescriptorSetLayout PerObjectDescriptorSetLayout() const { return m_perObjectDescriptorSetLayout; }
    // TODO End temporal

  private:
    Device*          m_device{nullptr};
    VkPipeline       m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};

    VkDescriptorSetLayout m_perFrameDescriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_perObjectDescriptorSetLayout{VK_NULL_HANDLE};
};
} // namespace Pinut
