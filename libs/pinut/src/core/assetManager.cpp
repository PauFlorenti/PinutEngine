#include "stdafx.h"

#include <external/tinygltf/tiny_gltf.h>
#include <external/tinyobjloader/tiny_obj_loader.h>

#include "src/assets/asset.h"
#include "src/assets/material.h"
#include "src/assets/mesh.h"
#include "src/assets/texture.h"
#include "src/core/assetManager.h"
#include "src/loaders/rawAssetData.h"

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
        ProcessTexture(absolutePath);
    }
    else if (extension == ".obj")
    {
        auto rawAsset = m_objLoader.ParseObjFile(absolutePath);

        ProcessRawData(std::move(rawAsset));
    }
    else
    {
        printf("[ERROR]: Unknown extension file.");
    }
}

void AssetManager::ProcessRawData(RawData InRawData)
{
    for (const auto rawTexture : InRawData.textureData)
    {
        ProcessTexture(rawTexture);
    }

    for (const auto rawMaterial : InRawData.materialData)
    {
        if (const auto materialUuid = std::hash<std::string>{}(rawMaterial.name);
            m_assets.find(materialUuid) != m_assets.end())
        {
            continue;
        }

        auto material = std::make_shared<Material>(rawMaterial.name);

        material->m_diffuse  = rawMaterial.diffuse;
        material->m_emissive = rawMaterial.emissive;
        material->m_specular = rawMaterial.specular;

        m_assets.insert({material->GetUUID(), material});
    }

    for (const auto rawMesh : InRawData.meshData)
    {
        std::vector<Primitive> primitives(rawMesh.primitives.size());
        std::vector<Vertex>    vertices(rawMesh.vertices.size());

        std::transform(
          rawMesh.primitives.begin(),
          rawMesh.primitives.end(),
          primitives.begin(),
          [](const RawPrimitive& InPrimitive)
          {
              //    if (auto m =
              //          m_assets.find(std::hash<std::string>{}(InPrimitive.materialName));
              //        m != m_assets.end())
              //    {
              //    }

              return Primitive{InPrimitive.firstIndex,
                               InPrimitive.firstVertex,
                               InPrimitive.indexCount,
                               InPrimitive.vertexCount};
          });

        std::transform(
          rawMesh.vertices.begin(),
          rawMesh.vertices.end(),
          vertices.begin(),
          [](const RawVertex& InVertex)
          {
              return Vertex{InVertex.position, InVertex.color, InVertex.uv, InVertex.normal};
          });

        auto mesh = std::make_shared<Mesh>(std::move(vertices),
                                           rawMesh.indices,
                                           std::move(primitives),
                                           rawMesh.name);

        m_assets.insert({mesh->GetUUID(), mesh});
    }
}

void AssetManager::ProcessTexture(const std::filesystem::path& InFilename)
{
    auto t = std::make_shared<Texture>(InFilename);
    m_assets.insert({t->GetUUID(), std::move(t)});
}
} // namespace Pinut
