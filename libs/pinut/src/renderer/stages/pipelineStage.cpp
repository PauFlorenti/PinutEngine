#include "stdafx.h"

#include "src/renderer/stages/pipelineStage.h"

namespace Pinut
{
void PipelineStage::BindPipeline(VkCommandBuffer cmd)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}
} // namespace Pinut
