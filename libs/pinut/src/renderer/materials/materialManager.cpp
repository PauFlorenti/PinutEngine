#include "stdafx.h"

#include "material.h"
#include "materialManager.h"
#include "src/renderer/device.h"

namespace Pinut
{
void MaterialManager::Init(Device* device)
{
    assert(device);
    m_device = device->GetDevice();

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10},
    };
    m_descriptorSetManager.OnCreate(m_device, 3, 10, std::move(descriptorPoolSizes));

    m_materialBuffer.Create(device,
                            sizeof(u32) * MAX_MATERIALS,
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    m_opaqueMaterial.BuildPipeline(m_device);
    m_transparentMaterial.BuildPipeline(m_device);
}

void MaterialManager::Shutdown()
{
    m_materialBuffer.Destroy();
    m_descriptorSetManager.OnDestroy();
    m_opaqueMaterial.Destroy(m_device);
    m_transparentMaterial.Destroy(m_device);
}

std::shared_ptr<MaterialInstance> MaterialManager::GetMaterialInstance(const std::string& name,
                                                                       MaterialType       type,
                                                                       MaterialData materialData)
{
    if (auto it = m_materials.find(name); it != m_materials.end())
    {
        printf("[WARN]: Trying to create materials that already exists with this name %s",
               name.c_str());

        auto mi = std::make_shared<MaterialInstance>(*it->second);
        return mi;
    }

    switch (type)
    {
        case MaterialType::OPAQUE:
        {
            auto mi           = m_opaqueMaterial.CreateMaterialInstance(m_device,
                                                              std::move(materialData),
                                                              m_materialBuffer,
                                                              m_materialCount,
                                                              m_descriptorSetManager,
                                                              ++m_unusedId);
            m_materials[name] = mi;
            m_materialCount++;
            return std::make_shared<MaterialInstance>(*mi);
            break;
        }
        case MaterialType::TRANSPARENT:
        {
            auto mi           = m_transparentMaterial.CreateMaterialInstance(m_device,
                                                                   std::move(materialData),
                                                                   m_materialBuffer,
                                                                   m_materialCount,
                                                                   m_descriptorSetManager,
                                                                   ++m_unusedId);
            m_materials[name] = mi;
            m_materialCount++;
            return std::make_shared<MaterialInstance>(*mi);
            break;
        }
        case MaterialType::COUNT:
        default:
            printf("[ERROR]: Trying to create material with undefined material type.");
            break;
    }

    return nullptr;
}
} // namespace Pinut
