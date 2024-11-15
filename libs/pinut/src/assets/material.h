#pragma once

#include "src/assets/asset.h"

namespace Pinut
{
class Device;
class Texture;
struct MaterialData
{
    u32 diffuse;
    u32 specularExponent;

    std::shared_ptr<Texture> diffuseTexture{nullptr};
    std::shared_ptr<Texture> normalTexture{nullptr};
    std::shared_ptr<Texture> metallicRoughnessTexture{nullptr};
    std::shared_ptr<Texture> emissiveTexture{nullptr};
    std::shared_ptr<Texture> ambientOcclusionTexture{nullptr};
};

struct GPUMaterialData
{
    u32 diffuse          = 0x00000000;
    u32 specularExponent = 1;
    u32 dummy1; // TODO Offset in my gpu is 16. Make it generic?
    u32 dummy2;
};

class Material : public Asset
{
  public:
    Material(const std::string& name, MaterialData materialData);
    ~Material();

    void Destroy() override;

    const std::string& GetName() const;
    VkDescriptorSet    GetDescriptorSet() const;

    void SetMaterialData(MaterialData materialData);
    void UpdateDescriptorSet(VkDevice device);
    void Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;

  private:
    std::string  m_name;
    MaterialData m_materialData;

    VkDescriptorSet                         m_descriptorSet{VK_NULL_HANDLE};
    std::array<std::shared_ptr<Texture>, 4> m_textures;
};
} // namespace Pinut
