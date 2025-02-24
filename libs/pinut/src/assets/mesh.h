#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "render_device/buffer.h"

#include "src/assets/asset.h"
#include "src/renderer/meshData.h"

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
    glm::vec4 color;
    glm::vec2 uv;
    glm::vec3 normal;

    inline bool operator==(const Vertex& other) const
    {
        return position == other.position && normal == other.normal && color == other.color &&
               uv == other.uv;
    }
    inline bool operator!=(const Vertex& other) const { return !(*this == other); }
};

struct Primitive
{
    u32                       m_firstIndex{0};
    u32                       m_firstVertex{0};
    u32                       m_indexCount{0};
    u32                       m_vertexCount{0};
    std::shared_ptr<Material> m_material{nullptr}; // TODO
};

class Mesh final : public Asset
{
    friend void Pinut::CreateMeshData(std::shared_ptr<RED::Device> device,
                                      entt::registry&              registry,
                                      Mesh&                        mesh);
    friend class Renderer;

  public:
    Mesh();
    Mesh(std::vector<Vertex> vertices, std::vector<u16> indices, std::vector<Primitive> primitives);
    ~Mesh();

    Mesh(const Mesh&)            = default;
    Mesh& operator=(const Mesh&) = default;
    Mesh(Mesh&&)                 = default;
    Mesh& operator=(Mesh&&)      = default;

    bool operator==(const Mesh& other) const;
    bool operator!=(const Mesh& other) const;

    void Destroy() override;
    void Clear();

    std::vector<Vertex>    m_vertices;
    std::vector<u16>       m_indices;
    std::vector<Primitive> m_primitives;

  private:
    entt::entity m_handle{entt::null};
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
