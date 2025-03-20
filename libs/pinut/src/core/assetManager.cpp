#include "stdafx.h"

#include <external/tinygltf/tiny_gltf.h>
#include <external/tinyobjloader/tiny_obj_loader.h>

#include "src/assets/asset.h"
#include "src/assets/material.h"
#include "src/assets/mesh.h"
#include "src/assets/texture.h"
#include "src/core/assetManager.h"
#include "src/core/renderable.h"

namespace Pinut
{
const std::filesystem::path AssetManager::m_assetsPath =
  std::filesystem::current_path().parent_path() / std::filesystem::path("assets");

AssetManager::AssetManager() = default;
AssetManager::~AssetManager()
{
    for (auto& asset : m_assets)
        asset.second->Destroy();

    m_assets.clear();
}

bool AssetManager::FindFile(const std::filesystem::path& filepath,
                            std::filesystem::path&       outAbsolutePath)
{
    assert(!filepath.empty());

    if (!filepath.has_extension())
    {
        printf("[ERROR]: File provided has no extension.");
        return false;
    }

    for (const auto& rootPath : std::filesystem::recursive_directory_iterator(m_assetsPath))
    {
        if (rootPath.path().filename() == filepath.filename())
        {
            outAbsolutePath = rootPath.path();
            return true;
        }
    }

    return false;
}

void AssetManager::ImportAsset(const std::filesystem::path& InFilepath)
{
    std::filesystem::path absolutePath;

    if (!FindFile(InFilepath, absolutePath))
    {
        return;
    }

    const auto extension = absolutePath.extension();

    if (extension == ".png" || extension == ".jpg")
    {
        return;
    }
    else if (extension == ".obj")
    {
        auto objAsset = m_objLoader.ParseObj(absolutePath);

        auto mesh = static_cast<Mesh*>(objAsset.get());

        if (!mesh)
        {
            return;
        }

        for (auto prim : mesh->m_primitives)
        {
            // TODO Load textures
            auto material = prim.m_material;

            if (!material)
            {
                continue;
            }

            m_assets.insert({material->GetUUID(), std::move(material)});
        }

        const auto uuid = objAsset->GetUUID();
        m_assets.insert({uuid, std::move(objAsset)});
    }
    else
    {
        printf("[ERROR]: Unknown extension file.");
    }
}
} // namespace Pinut
