#pragma once

#include "src/renderer/stages/pipelineStage.h"

namespace Pinut
{
class WireframeStage : public PipelineStage
{
  public:
    void BuildPipeline(VkDevice device) override;
    void Destroy(VkDevice device) override;

    VkDescriptorSetLayout m_perFrameDescriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_perObjectDescriptorSetLayout{VK_NULL_HANDLE};
};
} // namespace Pinut
