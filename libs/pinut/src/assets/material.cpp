#include "stdafx.h"

#include "src/assets/material.h"
#include "src/assets/texture.h"
#include "src/renderer/device.h"
#include "src/renderer/utils.h"

namespace Pinut
{
Material::Material(Device*            device,
                   const std::string& name,
                   MaterialData       materialData,
                   VkDescriptorSet    descriptorSet)
: m_name(name),
  m_materialData(std::move(materialData)),
  m_descriptorSet(descriptorSet)
{
    assert(device);
    m_buffer.Create(device, sizeof(GPUMaterialData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    auto data              = (GPUMaterialData*)m_buffer.AllocationInfo().pMappedData;
    data->diffuse          = m_materialData.diffuse;
    data->specularExponent = m_materialData.specularExponent;
}

Material::~Material() = default;

void Material::Destroy() { m_buffer.Destroy(); }

const std::string& Material::GetName() const { return m_name; }
VkDescriptorSet    Material::GetDescriptorSet() const { return m_descriptorSet; }

void Material::SetMaterialData(MaterialData materialData)
{
    m_materialData = std::move(materialData);
}

void Material::UpdateDescriptorSet(VkDevice device)
{
    auto perObjectBufferInfo =
      vkinit::DescriptorBufferInfo(m_buffer.m_buffer, 0, sizeof(GPUMaterialData));

    VkDescriptorImageInfo textureInfo[5] = {
      vkinit::DescriptorImageInfo(m_materialData.diffuseTexture->ImageView(),
                                  m_materialData.diffuseTexture->Sampler(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
      vkinit::DescriptorImageInfo(m_materialData.normalTexture->ImageView(),
                                  m_materialData.normalTexture->Sampler(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
      vkinit::DescriptorImageInfo(m_materialData.metallicRoughnessTexture->ImageView(),
                                  m_materialData.metallicRoughnessTexture->Sampler(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
      vkinit::DescriptorImageInfo(m_materialData.emissiveTexture->ImageView(),
                                  m_materialData.emissiveTexture->Sampler(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
      vkinit::DescriptorImageInfo(m_materialData.ambientOcclusionTexture->ImageView(),
                                  m_materialData.ambientOcclusionTexture->Sampler(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
    };

    VkWriteDescriptorSet writes[2] = {
      vkinit::WriteDescriptorSet(m_descriptorSet,
                                 0,
                                 1,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 &perObjectBufferInfo),
      vkinit::WriteDescriptorSet(m_descriptorSet,
                                 1,
                                 5,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 nullptr,
                                 textureInfo),
    };

    vkUpdateDescriptorSets(device, 2, writes, 0, nullptr);
}

void Material::Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const
{
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            1,
                            1,
                            &m_descriptorSet,
                            0,
                            nullptr);
}
} // namespace Pinut
