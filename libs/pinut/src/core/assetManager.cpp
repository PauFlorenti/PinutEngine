//#include "stdafx.h"
//
//#include <external/tinygltf/tiny_gltf.h>
//#include <external/tinyobjloader/tiny_obj_loader.h>
//
//#include "src/assets/asset.h"
//#include "src/assets/material.h"
//#include "src/assets/mesh.h"
//#include "src/assets/texture.h"
//#include "src/core/assetManager.h"
//#include "src/core/renderable.h"
//#include "src/renderer/stages/materialManager.h"
//
//namespace Pinut
//{
//
//const std::filesystem::path AssetManager::m_assetsPath =
//  std::filesystem::current_path().parent_path() / std::filesystem::path("assets");
//
//bool AssetManager::FindFile(const std::filesystem::path& filepath,
//                            std::filesystem::path&       outAbsolutePath)
//{
//    assert(!filepath.empty());
//
//    if (!filepath.has_extension())
//    {
//        printf("[ERROR]: File provided has no extension.");
//        return false;
//    }
//
//    for (const auto& rootPath : std::filesystem::recursive_directory_iterator(m_assetsPath))
//    {
//        if (rootPath.path().filename() == filepath.filename())
//        {
//            outAbsolutePath = rootPath.path();
//            return true;
//        }
//    }
//
//    return false;
//}
//
//void AssetManager::Init(Device* device)
//{
//    assert(device);
//    m_device = device;
//
//    m_materialManager.Init();
//    m_objLoader.Init(m_device);
//}
//
//void AssetManager::Shutdown()
//{
//    for (auto& asset : m_assets)
//        asset.second->Destroy();
//
//    m_assets.clear();
//
//    m_materialManager.Shutdown();
//}
//
//std::shared_ptr<Asset> AssetManager::LoadAsset(std::filesystem::path filename,
//                                               const std::string&    name)
//{
//    std::filesystem::path absolutePath;
//    if (!FindFile(filename, absolutePath))
//        return nullptr;
//
//    const auto extension = absolutePath.extension();
//
//    if (extension == ".png" || extension == ".jpg")
//    {
//        //if (auto t = Texture::CreateFromFile(absolutePath.string(), m_device))
//        //{
//        //    RegisterAsset(name, t);
//        //    return t;
//        //}
//        return nullptr;
//    }
//    else if (extension == ".obj")
//    {
//        return m_objLoader.LoadMeshFromFile(absolutePath, *this);
//    }
//    else
//    {
//        printf("[ERROR]: Unknown extension file.");
//    }
//
//    return nullptr;
//}
//
//void AssetManager::RegisterAsset(const std::string& name, std::shared_ptr<Asset> asset)
//{
//    assert(asset);
//    assert(!name.empty());
//    if (auto it = m_assets.find(name); it != m_assets.end())
//    {
//        if (it->second)
//        {
//            printf("Asset trying to be loaded already found in AssetManager!\n");
//            return;
//        }
//    }
//
//    m_assets.insert({name, asset});
//}
//
//void AssetManager::ReleaseAsset(const std::string& name)
//{
//    assert(!name.empty());
//    const auto it = m_assets.find(name);
//    if (it == m_assets.end())
//    {
//        printf("[WARN]: Trying to release asset not registered in the AssetManager.");
//        return;
//    }
//
//    it->second.reset();
//}
//
//std::shared_ptr<Material> AssetManager::CreateMaterial(const std::string&    name,
//                                                       VkDescriptorSetLayout layout,
//                                                       MaterialData          data)
//{
//    auto mat = m_materialManager.CreateMaterial(name, layout, std::move(data));
//    RegisterAsset(name, mat);
//    return mat;
//}
//} // namespace Pinut
