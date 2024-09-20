#pragma once

namespace Pinut
{
class DescriptorSetManager;
class GPUBuffer;
class Texture;
struct MaterialInstance;
//enum class MaterialType
//{
//    OPAQUE,
//    TRANSPARENT,
//    COUNT
//};
struct MaterialData
{
    u32 diffuse;
    u32 specularExponent;

    std::shared_ptr<Texture> diffuseTexture{nullptr};
    std::shared_ptr<Texture> normal{nullptr};
    std::shared_ptr<Texture> metallicRoughness{nullptr};
};

struct GPUMaterialData
{
    u32 diffuse          = 0x00000000;
    u32 specularExponent = 1;
    u32 dummy1; // TODO Offset in my gpu is 16. Make it generic?
    u32 dummy2;
};

struct Material
{
    virtual void                              BuildPipeline(VkDevice device) = 0;
    virtual void                              Destroy(VkDevice)              = 0;
    virtual std::shared_ptr<MaterialInstance> CreateMaterialInstance(
      VkDevice              device,
      MaterialData          materialData,
      const GPUBuffer&      buffer,
      u32                   offsetCount,
      DescriptorSetManager& descriptorSetManager,
      const u32             id) = 0;
    void BindPipeline(VkCommandBuffer cmd);

    VkPipeline       m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    std::string      m_name = "";
};

struct MaterialInstance
{
    MaterialInstance(Material* material, VkDescriptorSet set, /*MaterialType type,*/ const u32 id)
    : m_material(material),
      m_descriptorSet(set),
      //m_type(type),
      m_id(id)
    {
        assert(m_material);
        //assert(m_type != MaterialType::COUNT);
    }

    //bool operator==(const MaterialInstance& other)
    //{
    //    return m_id == other.m_id && m_type == other.m_type;
    //}
    //bool operator!=(const MaterialInstance& other) { return !(*this == other); }

    const Material* GetMaterial() const { return m_material; }
    void            Bind(VkCommandBuffer cmd);
    void            BindPipeline(VkCommandBuffer cmd) { m_material->BindPipeline(cmd); }
    //MaterialType    Type() const { return m_type; }

  private:
    Material*       m_material{nullptr};
    VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};
    //MaterialType    m_type;
    u32             m_id = 0xFFFFFFFF;
};
} // namespace Pinut
