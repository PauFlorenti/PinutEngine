#include "stdafx.h"

#include "material.h"

namespace Pinut
{
void Material::BindPipeline(VkCommandBuffer cmd)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

void MaterialInstance::Bind(VkCommandBuffer cmd)
{
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_material->m_pipelineLayout,
                            1,
                            1,
                            &m_descriptorSet,
                            0,
                            nullptr);
}
} // namespace Pinut
