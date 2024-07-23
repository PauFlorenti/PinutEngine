#pragma once

namespace Pinut
{
class GPUBuffer;
class Texture;
struct MaterialResources
{
    std::unordered_map<std::string, GPUBuffer*>  m_buffers;
    std::unordered_map<std::string, VkImageView> m_imageViews;
    std::unordered_map<std::string, VkSampler>   m_samplers;
};

struct MaterialData
{
    u32 color;

    Texture* diffuse{nullptr};
    Texture* normal{nullptr};
    Texture* metallicRoughness{nullptr};
};

struct Material
{
    virtual void BuildPipeline(VkDevice device) = 0;
    // virtual void CreateMaterialInstance(VkDevice device, MaterialResources* resources);
    virtual void Destroy(VkDevice) = 0;
    void         BindPipeline(VkCommandBuffer cmd);

    VkPipeline       m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    std::string      m_name = "";
};

struct MaterialInstance
{
    MaterialInstance(Material* material, VkDescriptorSet set)
    : m_material(material),
      m_descriptorSet(set)
    {
        assert(m_material);
    }

    const Material* GetMaterial() const { return m_material; }
    void            Bind(VkCommandBuffer cmd);
    void            BindPipeline(VkCommandBuffer cmd) { m_material->BindPipeline(cmd); }

  private:
    Material*       m_material{nullptr};
    VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};
};
} // namespace Pinut
