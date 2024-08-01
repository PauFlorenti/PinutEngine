#include "stdafx.h"

#include <filesystem>

#include "tinyobjloader/tiny_obj_loader.h"

#include "src/assets/asset.h"
#include "src/assets/mesh.h"
#include "src/core/assetManager.h"
#include "src/renderer/device.h"
#include "src/renderer/materials/material.h"
#include "src/renderer/materials/materialManager.h"

namespace Pinut
{
void AssetManager::Init(Device* device, std::shared_ptr<MaterialManager> materialManager)
{
    assert(device);
    assert(materialManager);
    m_device          = device;
    m_materialManager = materialManager;
}

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

    assert(!attrib.GetVertices().empty());
    assert(attrib.GetVertices().size() % 3 == 0);

    u64 indexCount = 0;
    for (const auto& s : shapes)
    {
        indexCount += s.mesh.indices.size();
    }
    const u64 vertexCount      = attrib.GetVertices().size() / 3;
    const u64 vertexBufferSize = sizeof(Vertex) * vertexCount;
    const u64 indexBufferSize  = sizeof(u16) * indexCount;

    auto mesh = std::make_shared<Mesh>();
    mesh->m_vertexBuffer.Create(m_device,
                                vertexBufferSize,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    mesh->m_indexBuffer.Create(m_device,
                               indexBufferSize,
                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                               VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    GPUBuffer stagingBuffer;
    stagingBuffer.Create(m_device,
                         vertexBufferSize + indexBufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

    u64       vertexOffset = 0;
    u64       indexOffset  = indexBufferSize;
    const u8* stagingData  = static_cast<u8*>(stagingBuffer.AllocationInfo().pMappedData);

    for (const auto& shape : shapes)
    {
        std::unordered_map<Vertex, u32> uniqueVertices{};
        std::vector<Vertex>             vertices;
        std::vector<u16>                indices;
        Mesh::DrawCall                  dc;

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

        const auto vertexData     = stagingData + vertexOffset;
        const auto indexData      = stagingData + indexOffset;
        const u64  vertexDataSize = vertices.size() * sizeof(Vertex);
        const u64  indexDataSize  = indices.size() * sizeof(u16);
        memcpy((void*)vertexData, vertices.data(), vertexDataSize);
        memcpy((void*)indexData, indices.data(), indexDataSize);

        dc.m_vertexBuffer = std::make_shared<GPUBuffer>(mesh->m_vertexBuffer);
        dc.m_vertexCount  = static_cast<u32>(vertices.size());
        dc.m_vertexOffset = vertexOffset;

        dc.m_indexBuffer = std::make_shared<GPUBuffer>(mesh->m_indexBuffer);
        dc.m_indexCount  = static_cast<u32>(indices.size());
        dc.m_indexOffset = indexOffset;

        vertexOffset = vertexDataSize;
        indexOffset  = indexDataSize;

        const auto it = std::find_if(materials.begin(),
                                     materials.end(),
                                     [&shape](tinyobj::material_t m)
                                     {
                                         return m.name == shape.name;
                                     });

        if (it == materials.end())
            continue;

        MaterialData materialData;
        // materialData.color = glm::vec3(it->diffuse[0], it->diffuse[1], it->diffuse[2]);

        auto mi       = m_materialManager->CreateMaterialInstance(it->name,
                                                            Pinut::MaterialType::OPAQUE,
                                                            std::move(materialData));
        dc.m_material = std::move(mi);
    }

    RegisterAsset(name, std::move(mesh));
}
} // namespace Pinut
