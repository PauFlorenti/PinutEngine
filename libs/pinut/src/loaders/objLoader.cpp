#include "stdafx.h"

#include "external/tinyobjloader/tiny_obj_loader.h"

#include "src/assets/material.h"
#include "src/assets/mesh.h"
#include "src/loaders/objLoader.h"

namespace Pinut
{
OBJLoader::OBJLoader()  = default;
OBJLoader::~OBJLoader() = default;

std::shared_ptr<Mesh> OBJLoader::ParseObj(const std::filesystem::path& InFilepath)
{
    tinyobj::attrib_t                attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;

    const auto absoluteFilename = std::filesystem::absolute(InFilepath).string();
    const auto absolutePath     = std::filesystem::absolute(InFilepath.parent_path()).string();

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

    auto ConvertToVec3 = [](const float Input[3]) -> glm::vec3
    {
        return glm::vec3(Input[0], Input[1], Input[2]);
    };

    std::vector<std::shared_ptr<Material>> coreMaterials;
    coreMaterials.reserve(materials.size());

    for (const auto material : materials)
    {
        auto m = std::make_shared<Material>(absoluteFilename + ".mat." + material.name);

        m->m_diffuse  = ConvertToVec3(material.diffuse);
        m->m_emissive = ConvertToVec3(material.emission);
        m->m_specular = ConvertToVec3(material.specular);

        m->m_diffuseTexture  = material.diffuse_texname;
        m->m_normalTexture   = material.normal_texname;
        m->m_emissiveTexture = material.emissive_texname;
        m->m_specularTexture = material.specular_texname;

        coreMaterials.emplace_back(std::move(m));
    }

    return ParseMesh(absoluteFilename, attrib, shapes, std::move(coreMaterials));
}

std::shared_ptr<Mesh> OBJLoader::ParseMesh(const std::string&                     InFilename,
                                           const tinyobj::attrib_t&               InAttrib,
                                           const std::vector<tinyobj::shape_t>&   InShapes,
                                           std::vector<std::shared_ptr<Material>> InMaterials)
{
    const auto                      filename = InFilename + ".mesh";
    auto                            mesh     = std::make_shared<Mesh>(filename);
    std::unordered_map<Vertex, u16> uniqueVertices{};

    for (const auto& shape : InShapes)
    {
        uint32_t firstIndex  = static_cast<uint32_t>(mesh->m_indices.size());
        uint32_t firstVertex = static_cast<uint32_t>(mesh->m_vertices.size());
        uint32_t indexCount  = 0;
        uint32_t vertexCount = 0;

        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.position = {InAttrib.vertices[3 * index.vertex_index + 0],
                               InAttrib.vertices[3 * index.vertex_index + 1],
                               InAttrib.vertices[3 * index.vertex_index + 2]};

            if (!InAttrib.normals.empty())
            {
                vertex.normal = {InAttrib.normals[3 * index.normal_index + 0],
                                 InAttrib.normals[3 * index.normal_index + 1],
                                 InAttrib.normals[3 * index.normal_index + 2]};
            }

            vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};

            if (InAttrib.texcoords.size() > 0)
            {
                vertex.uv = {InAttrib.texcoords[2 * index.texcoord_index + 0],
                             1.0f - InAttrib.texcoords[2 * index.texcoord_index + 1]};
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

        for (const auto material : InMaterials)
        {
            auto wholeName    = material->GetName();
            auto materialName = wholeName.substr(wholeName.find(".mat.") + 5);
            prim.m_material   = shape.name == materialName ? std::move(material) : nullptr;
        }

        mesh->m_primitives.push_back(prim);
    }

    return mesh;
}
} // namespace Pinut
