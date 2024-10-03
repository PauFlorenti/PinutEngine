//#pragma once
//
//#include "src/assets/material.h"
//
//namespace Pinut
//{
//class Device;
//class Material;
//
//u32 constexpr MAX_MATERIALS = 1000;
//
//class MaterialManager
//{
//    friend Material;
//
//  public:
//    ~MaterialManager() {};
//    void                      Init();
//    void                      Shutdown();
//    std::shared_ptr<Material> CreateMaterial(const std::string&    name,
//                                             VkDescriptorSetLayout descriptorSetLayout,
//                                             MaterialData          materialData);
//
//  private:
//    std::map<std::string, std::shared_ptr<Material>> m_materials;
//
//    u32 m_materialCount{0};
//    u32 m_unusedId = 0;
//};
//} // namespace Pinut
