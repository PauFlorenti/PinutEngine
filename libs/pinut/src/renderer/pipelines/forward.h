#pragma once

#include <vulkan/vulkan.h>

#include "src/renderer/buffer.h"
#include "src/renderer/descriptorSetManager.h"

namespace Pinut
{
class Device;
struct PerFrameData;
class ForwardPipeline
{
  public:
    void Init(Device* device);
    void Shutdown();
    void BindPipeline(VkCommandBuffer cmd);
    void UpdatePerFrameData(VkCommandBuffer cmd, PerFrameData data);
    void Render(VkCommandBuffer cmd);

    VkPipeline Pipeline() const;

  private:
    Device*              m_device{nullptr};
    VkPipeline           m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout     m_pipelineLayout{VK_NULL_HANDLE};
    DescriptorSetManager m_descriptorSetManager;

    VkDescriptorSetLayout m_perFrameDescriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_perObjectDescriptorSetLayout{VK_NULL_HANDLE};

    GPUBuffer m_perFrameBuffer;
    GPUBuffer m_perObjectBuffer;
};
} // namespace Pinut
