#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "src/assets/asset.h"
#include "src/renderer/buffer.h"

namespace Pinut
{
class AssetManager;
class Device;
class Material;
class Renderable;
struct MaterialInstance;
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 uv;

    inline bool operator==(const Vertex& other) const
    {
        return position == other.position && normal == other.normal && color == other.color &&
               uv == other.uv;
    }
    inline bool operator!=(const Vertex& other) const { return !(*this == other); }
};

class Mesh final : public Asset
{
  public:
    static std::shared_ptr<Mesh> Create(Device*             device,
                                        std::vector<Vertex> vertices,
                                        std::vector<u16>    indices);

    Mesh()  = default;
    ~Mesh() = default;

    void Destroy() override;
    void Draw(VkCommandBuffer cmd);

    std::vector<Vertex>       m_vertices;
    std::vector<u16>          m_indices;
    GPUBuffer                 m_vertexBuffer;
    GPUBuffer                 m_indexBuffer;
    std::shared_ptr<Material> m_material{nullptr};
};
} // namespace Pinut

namespace std
{
using namespace Pinut;
template <>
struct hash<Vertex>
{
    size_t operator()(Vertex const& vertex) const
    {
        return ((((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >>
                  1) ^
                 (hash<glm::vec3>()(vertex.color) << 1)) >>
                1) ^
               (hash<glm::vec2>()(vertex.uv) << 1);
    }
};
} // namespace std
