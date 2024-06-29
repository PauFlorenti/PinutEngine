#pragma once

#include <glm/glm.hpp>

#include "src/renderer/buffer.h"

namespace Pinut
{
class Device;
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 uv;

    inline bool operator==(const Vertex& other)
    {
        return position == other.position && normal == other.normal && color == other.color &&
               uv == other.uv;
    }
    inline bool operator!=(const Vertex& other) { return !(*this == other); }
};

class Mesh
{
  public:
    static Mesh* Create(Device* device, std::vector<Vertex> vertices, std::vector<u16> indices);
    void         Destroy();

    const u32& GetVertexCount() const;
    const u32& GetIndexCount() const;

    GPUBuffer m_vertexBuffer;
    GPUBuffer m_indexBuffer;

  private:
    u32 m_vertexCount;
    u32 m_indexCount;
};
} // namespace Pinut
