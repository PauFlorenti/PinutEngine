#pragma once

namespace Pinut
{
class PipelineStage
{
  public:
    virtual void BuildPipeline(VkDevice device) = 0;
    virtual void Destroy(VkDevice device)       = 0;
    void         BindPipeline(VkCommandBuffer cmd);

  public:
    VkPipeline       m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
};
} // namespace Pinut
