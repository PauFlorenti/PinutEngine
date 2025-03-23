#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace Pinut
{
struct RawVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 uv;
    glm::vec3 normal;

    inline bool operator==(const RawVertex& other) const
    {
        return position == other.position && normal == other.normal && color == other.color &&
               uv == other.uv;
    }
    inline bool operator!=(const RawVertex& other) const { return !(*this == other); }
};

struct RawPrimitive
{
    u32         firstIndex{0};
    u32         firstVertex{0};
    u32         indexCount{0};
    u32         vertexCount{0};
    std::string materialName{};
};

struct RawMeshData
{
    std::string               name;
    std::string               materialName;
    std::vector<RawVertex>    vertices;
    std::vector<u16>          indices;
    std::vector<RawPrimitive> primitives;
};

struct RawMaterialData
{
    std::string name;
    glm::vec3   diffuse{};
    glm::vec3   specular{};
    glm::vec3   emissive{};

    std::string diffuseTexture;
    std::string specularTexture;
    std::string normalTexture;
    std::string metallicRoughnessTexture;
    std::string emissiveTexture;
};

struct RawData
{
    std::vector<RawMeshData>     meshData;
    std::vector<std::string>     textureData;
    std::vector<RawMaterialData> materialData;
};
} // namespace Pinut

namespace std
{
using namespace Pinut;

template <>
struct hash<RawVertex>
{
    size_t operator()(RawVertex const& vertex) const
    {
        return ((((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >>
                  1) ^
                 (hash<glm::vec3>()(vertex.color) << 1)) >>
                1) ^
               (hash<glm::vec2>()(vertex.uv) << 1);
    }
};
} // namespace std
