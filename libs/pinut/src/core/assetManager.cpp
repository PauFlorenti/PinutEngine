#include "stdafx.h"

#include <filesystem>

#include "tinyobjloader/tiny_obj_loader.h"

#include "src/assets/asset.h"
#include "src/assets/mesh.h"
#include "src/core/assetManager.h"
#include "src/renderer/device.h"

namespace Pinut
{
AssetManager* AssetManager::m_managerInstance = nullptr;
AssetManager* AssetManager::Get()
{
    if (!m_managerInstance)
        m_managerInstance = new AssetManager();

    return m_managerInstance;
}

void AssetManager::Init(Device* device) { m_device = device; }

void AssetManager::Shutdown()
{
    for (auto& asset : m_assets)
        asset.second->Destroy();

    m_assets.clear();
}

void AssetManager::LoadAsset(const std::string& filename, const std::string& name)
{
    assert(!filename.empty());

    std::filesystem::path p(filename);

    if (p.extension() == ".obj")
    {
        LoadMesh(filename, name);
    }
    else
    {
        printf("[ERROR]: Unknown extension file.");
    }
}

void AssetManager::RegisterAsset(const std::string& name, std::shared_ptr<Asset> asset)
{
    assert(asset);
    assert(!name.empty());
    if (auto it = m_assets.find(name); it != m_assets.end())
    {
        if (it->second)
        {
            printf("Asset trying to be loaded already found in AssetManager!\n");
            return;
        }
    }

    m_assets.insert({name, asset});
}

void AssetManager::ReleaseAsset(const std::string& name)
{
    assert(!name.empty());
    const auto it = m_assets.find(name);
    if (it == m_assets.end())
    {
        printf("[WARN]: Trying to release asset not registered in the AssetManager.");
        return;
    }

    it->second.reset();
}

void AssetManager::LoadMesh(const std::string& filename, const std::string& name)
{
    tinyobj::attrib_t                attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;

    //#ifdef _WIN32
    //    const auto path = filename.substr(filename.find_last_of("\\") + 1);
    //#else
    //#endif
    const auto path = filename.substr(0, filename.find_last_of("/") + 1);

    std::string warn;
    std::string err;
    bool        ret = tinyobj::LoadObj(&attrib,
                                &shapes,
                                &materials,
                                &warn,
                                &err,
                                filename.c_str(),
                                path.c_str(),
                                true);

    if (!warn.empty())
    {
        printf("[WARN]: %s\n", warn.c_str());
    }

    if (!err.empty())
    {
        printf("[ERROR]: %s\n", err.c_str());
    }

    if (!ret)
    {
        printf("Failed to load/parse .obj.\n");
        return;
    }

    std::unordered_map<Vertex, u32> uniqueVertices{};
    std::vector<Vertex>             vertices;
    std::vector<u16>                indices;

    for (const auto& shape : shapes)
    {
        Mesh::DrawCall dc;
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.position = {attrib.vertices[3 * index.vertex_index + 0],
                               attrib.vertices[3 * index.vertex_index + 1],
                               attrib.vertices[3 * index.vertex_index + 2]};

            if (!attrib.normals.empty())
            {
                vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                                 attrib.normals[3 * index.normal_index + 1],
                                 attrib.normals[3 * index.normal_index + 2]};
            }

            vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};

            if (attrib.texcoords.size() > 0)
            {
                vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
                             1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(static_cast<u16>(uniqueVertices[vertex]));
        }
    }

    Mesh::Create(name, std::move(vertices), std::move(indices));

    // Create material with the same name as mesh

    for (const auto& m : materials)
    {
    }
}
} // namespace Pinut
