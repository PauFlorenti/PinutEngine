#include "stdafx.h"

#include "external/tinyobjloader/tiny_obj_loader.h"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"
#include "src/assets/mesh.h"
#include "src/core/assetManager.h"
#include "src/core/node.h"
#include "src/core/renderable.h"
#include "src/loaders/objLoader.h"

namespace Pinut
{
OBJLoader::OBJLoader() = default;

std::shared_ptr<Renderable> OBJLoader::LoadRenderableFromFile(const std::filesystem::path& filepath,
                                                              AssetManager& assetManager)
{
    auto mesh = LoadMeshFromFile(filepath, assetManager);
    return std::make_shared<Renderable>(std::make_shared<Node>(std::move(mesh)),
                                        filepath.filename().string());
}

std::shared_ptr<Mesh> OBJLoader::LoadMeshFromFile(const std::filesystem::path& filepath,
                                                  AssetManager&                assetManager)
{
    tinyobj::attrib_t                attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;

    const auto absoluteFilename = std::filesystem::absolute(filepath).string();
    const auto absolutePath     = std::filesystem::absolute(filepath.parent_path()).string();

    std::string warn;
    std::string err;
    bool        ret = tinyobj::LoadObj(&attrib,
                                &shapes,
                                &materials,
                                &warn,
                                &err,
                                absoluteFilename.c_str(),
                                absolutePath.c_str(),
                                true);

    if (!warn.empty())
    {
        printf("[WARN]: %s\n", warn.c_str());
    }

    if (!err.empty())
    {
        printf("[ERROR]: %s\n", err.c_str());
        return nullptr;
    }

    if (!ret)
    {
        printf("Failed to load/parse .obj.\n");
        return nullptr;
    }

    assert(!attrib.GetVertices().empty());
    assert(attrib.GetVertices().size() % 3 == 0);

    auto mesh = std::make_shared<Mesh>();

    std::unordered_map<Vertex, u16> uniqueVertices{};
    for (const auto& shape : shapes)
    {
        uint32_t firstIndex  = static_cast<uint32_t>(mesh->m_indices.size());
        uint32_t firstVertex = static_cast<uint32_t>(mesh->m_vertices.size());
        uint32_t indexCount  = 0;
        uint32_t vertexCount = 0;

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
                uniqueVertices[vertex] = static_cast<uint32_t>(uniqueVertices.size());
                mesh->m_vertices.push_back(vertex);
                ++vertexCount;
            }

            mesh->m_indices.push_back(static_cast<u16>(uniqueVertices[vertex]));
            ++indexCount;
        }

        Primitive prim;
        prim.m_firstIndex  = firstIndex;
        prim.m_firstVertex = firstVertex;
        prim.m_indexCount  = indexCount;
        prim.m_vertexCount = vertexCount;

        mesh->m_primitives.push_back(prim);
    }

    assetManager.RegisterAsset(filepath.filename().string(), mesh);
    return mesh;
}
} // namespace Pinut
