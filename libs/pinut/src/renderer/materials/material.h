#pragma once

namespace Pinut
{
class GPUBuffer;
class Texture;
enum class MaterialType
{
    OPAQUE,
    TRANSPARENT,
    COUNT
};
struct MaterialData
{
    u32 color;

    std::shared_ptr<Texture> diffuse{nullptr};
    std::shared_ptr<Texture> normal{nullptr};
    std::shared_ptr<Texture> metallicRoughness{nullptr};
};

struct GPUMaterialData
{
    glm::vec3 padding;
    u32 color;
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
    MaterialInstance(Material* material, VkDescriptorSet set, MaterialType type)
    : m_material(material),
      m_descriptorSet(set),
      m_type(type)
    {
        assert(m_material);
        assert(m_type != MaterialType::COUNT);
    }

    const Material* GetMaterial() const { return m_material; }
    void            Bind(VkCommandBuffer cmd);
    void            BindPipeline(VkCommandBuffer cmd) { m_material->BindPipeline(cmd); }
    MaterialType    Type() const { return m_type; }

  private:
    Material*       m_material{nullptr};
    VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};
    MaterialType    m_type;
};
} // namespace Pinut
