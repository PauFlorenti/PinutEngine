#include "stdafx.h"

#include "materialManager.h"
#include "src/renderer/device.h"

namespace Pinut
{
void MaterialManager::Init(Device* device)
{
    assert(device);
    m_device = device;

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10},
    };
    m_descriptorSetManager.OnCreate(m_device->GetDevice(), 3, 100, std::move(descriptorPoolSizes));

    m_materialBuffer.Create(device,
                            sizeof(u32) * MAX_MATERIALS,
                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void MaterialManager::Shutdown()
{
    m_materialBuffer.Destroy();
    m_descriptorSetManager.OnDestroy();
}

std::shared_ptr<Material> MaterialManager::CreateMaterial(const std::string&    name,
                                                          VkDescriptorSetLayout descriptorSetLayout,
                                                          MaterialData          materialData)
{
    if (const auto it = m_materials.find(name); it != m_materials.end())
    {
        printf("[WARN]: Material with name '%s' already exists.", name.c_str());
        return it->second;
    }

    auto set = m_descriptorSetManager.Allocate(descriptorSetLayout);
    auto mat = std::make_shared<Material>(m_device, name, std::move(materialData), std::move(set));
    mat->UpdateDescriptorSet(m_device->GetDevice());

    m_materials[name] = mat;

    return mat;
}
} // namespace Pinut
