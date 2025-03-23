#include "stdafx.h"

#include "external/tinyobjloader/tiny_obj_loader.h"

#include "src/loaders/objLoader.h"
#include "src/loaders/rawAssetData.h"

namespace Pinut
{
OBJLoader::OBJLoader()  = default;
OBJLoader::~OBJLoader() = default;

RawData OBJLoader::ParseObjFile(const std::filesystem::path& InFilepath)
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
        return {};
    }

    if (!ret)
    {
        printf("Failed to load/parse .obj.\n");
        return {};
    }

    assert(!attrib.GetVertices().empty());
    assert(attrib.GetVertices().size() % 3 == 0);

    RawData rawData;
    rawData.materialData.resize(materials.size());

    auto ConvertToVec3 = [](const float Input[3]) -> glm::vec3
    {
        return glm::vec3(Input[0], Input[1], Input[2]);
    };

    auto InsertTexture = [&](const std::string& InTextureName)
    {
        if (InTextureName.empty())
        {
            return;
        }

        if (auto it =
              std::find(rawData.textureData.begin(), rawData.textureData.end(), InTextureName);
            it != rawData.textureData.end())
        {
            return;
        }

        // TODO Find recursivelly in folder ...
        rawData.textureData.emplace_back(absolutePath + "\\" + InTextureName);
    };

    std::transform(materials.begin(),
                   materials.end(),
                   rawData.materialData.begin(),
                   [&](const tinyobj::material_t& InMaterial)
                   {
                       InsertTexture(InMaterial.diffuse_texname);
                       InsertTexture(InMaterial.normal_texname);
                       InsertTexture(InMaterial.emissive_texname);
                       InsertTexture(InMaterial.specular_texname);

                       return RawMaterialData{InMaterial.name,
                                              ConvertToVec3(InMaterial.diffuse),
                                              ConvertToVec3(InMaterial.emission),
                                              ConvertToVec3(InMaterial.specular),
                                              InMaterial.diffuse_texname,
                                              InMaterial.normal_texname,
                                              InMaterial.emissive_texname,
                                              InMaterial.specular_texname};
                   });

    rawData.meshData.resize(1);

    ParseMesh(absoluteFilename, attrib, shapes, rawData);

    return rawData;
}

void OBJLoader::ParseMesh(const std::string&                   InFilename,
                          const tinyobj::attrib_t&             InAttrib,
                          const std::vector<tinyobj::shape_t>& InShapes,
                          RawData&                             OutRawData)
{
    auto& mesh = OutRawData.meshData.at(0); // At the moment assume there is only one mesh.
    mesh.name  = InFilename + ".mesh";
    std::unordered_map<RawVertex, u16> uniqueVertices{};

    for (const auto& shape : InShapes)
    {
        uint32_t firstIndex  = static_cast<uint32_t>(mesh.indices.size());
        uint32_t firstVertex = static_cast<uint32_t>(mesh.vertices.size());
        uint32_t indexCount  = 0;
        uint32_t vertexCount = 0;

        for (const auto& index : shape.mesh.indices)
        {
            RawVertex vertex{};

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
                mesh.vertices.push_back(vertex);
                ++vertexCount;
            }

            mesh.indices.push_back(static_cast<u16>(uniqueVertices[vertex]));
            ++indexCount;
        }

        RawPrimitive prim;
        prim.firstIndex  = firstIndex;
        prim.firstVertex = firstVertex;
        prim.indexCount  = indexCount;
        prim.vertexCount = vertexCount;

        for (const auto material : OutRawData.materialData)
        {
            auto wholeName    = material.name;
            auto materialName = wholeName.substr(wholeName.find(".mat.") + 5);
            prim.materialName = shape.name == materialName ? materialName : "";
        }

        mesh.primitives.push_back(prim);
    }
}
} // namespace Pinut
