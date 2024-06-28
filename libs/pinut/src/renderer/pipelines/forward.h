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

  private:
    Device*          m_device{nullptr};
    VkPipeline       m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
};
} // namespace Pinut
