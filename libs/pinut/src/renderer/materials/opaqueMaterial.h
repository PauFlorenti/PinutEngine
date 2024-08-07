#pragma once

#include "src/renderer/materials/material.h"

namespace Pinut
{
class Texture;
struct OpaqueMaterial : public Material
{
    void                              BuildPipeline(VkDevice device) override;
    void                              Destroy(VkDevice device) override;
    std::shared_ptr<MaterialInstance> CreateMaterialInstance(
      VkDevice              device,
      MaterialData          materialData,
      const GPUBuffer&      buffer,
      u32                   offsetCount,
      DescriptorSetManager& descriptorSetManager,
      const u32             id) override;

    VkDescriptorSetLayout m_perFrameDescriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_perObjectDescriptorSetLayout{VK_NULL_HANDLE};
};
} // namespace Pinut
