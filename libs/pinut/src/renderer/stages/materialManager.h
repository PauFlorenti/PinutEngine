#pragma once

#include "src/assets/material.h"
#include "src/renderer/buffer.h"
#include "src/renderer/descriptorSetManager.h"

namespace Pinut
{
class Device;
class Material;

u32 constexpr MAX_MATERIALS = 1000;

class MaterialManager
{
    friend Material;

  public:
    ~MaterialManager() {};
    void                      Init(Device* device);
    void                      Shutdown();
    std::shared_ptr<Material> CreateMaterial(const std::string&    name,
                                             VkDescriptorSetLayout descriptorSetLayout,
                                             MaterialData          materialData);

  private:
    Device*                                          m_device{nullptr};
    std::map<std::string, std::shared_ptr<Material>> m_materials;

    GPUBuffer m_materialBuffer;
    u32       m_materialCount{0};

    DescriptorSetManager m_descriptorSetManager;
    u32                  m_unusedId = 0;
};
} // namespace Pinut
