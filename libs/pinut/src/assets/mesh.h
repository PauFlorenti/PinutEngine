#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "src/assets/asset.h"
#include "src/renderer/buffer.h"

namespace Pinut
{
class AssetManager;
class Device;
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
    struct DrawCall
    {
        u32                               m_vertexCount;
        u32                               m_indexCount;
        u64                               m_vertexOffset;
        u64                               m_indexOffset;
        std::shared_ptr<MaterialInstance> m_material;
        std::shared_ptr<GPUBuffer>        m_vertexBuffer;
        std::shared_ptr<GPUBuffer>        m_indexBuffer;
        std::shared_ptr<Renderable>       m_owner;

        void Draw(VkCommandBuffer cmd) const;
    };

    static std::shared_ptr<Mesh> Create(Device*             device,
                                        std::vector<Vertex> vertices,
                                        std::vector<u16>    indices);

    Mesh()  = default;
    ~Mesh() = default;
    void Destroy() override;

    const std::vector<std::shared_ptr<MaterialInstance>> Materials() const;
    std::vector<DrawCall>&                               DrawCalls() { return m_drawCalls; };

    void SetMaterial(std::shared_ptr<MaterialInstance> material, u32 slot = 0);

    GPUBuffer m_vertexBuffer;
    GPUBuffer m_indexBuffer;

  private:
    std::vector<DrawCall> m_drawCalls;
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
