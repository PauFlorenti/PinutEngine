#pragma once

#include "src/renderer/buffer.h"
#include "src/renderer/descriptorSetManager.h"
#include "src/renderer/materials/opaqueMaterial.h"
#include "src/renderer/materials/transparentMaterial.h"

namespace Pinut
{
class Device;
struct MaterialInstance;
struct MaterialResources;

u32 constexpr MAX_MATERIALS = 1000;

class MaterialManager
{
  public:
    void                              Init(Device* device);
    void                              Shutdown();
    std::shared_ptr<MaterialInstance> CreateMaterialInstance(const std::string& name,
                                                             MaterialType       type,
                                                             MaterialData       materialData);

  private:
    VkDevice                                                 m_device;
    std::map<std::string, std::shared_ptr<MaterialInstance>> m_materials;

    GPUBuffer m_materialBuffer;
    u32       m_materialCount{0};

    DescriptorSetManager m_descriptorSetManager;
    OpaqueMaterial       m_opaqueMaterial;
    TransparentMaterial  m_transparentMaterial;
};
} // namespace Pinut
