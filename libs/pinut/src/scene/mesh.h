#pragma once

#include "src/core/buffer.h"

namespace Pinut
{
    class Device;
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 color;
        glm::vec2 uv;

        inline bool operator==(const Vertex &other) { return position == other.position &&
                                                             normal == other.normal &&
                                                             color == other.color &&
                                                             uv == other.uv; }
        inline bool operator!=(const Vertex &other) { return !(*this == other); }
    };

    class Mesh
    {
    public:
        static Mesh *CreateMesh(std::vector<Vertex> inVertices, std::vector<uint16_t> inIndices);

        inline bool operator==(const Mesh &other)
        {
            return std::equal(vertices.begin(), vertices.end(), other.vertices.begin()) &&
                   std::equal(indices.begin(), indices.end(), other.indices.begin());
        }
        inline bool operator!=(const Mesh &other) { return !(*this == other); }

        const std::vector<Vertex> &GetVertices() const { return vertices; }
        const std::vector<uint16_t> &GetIndices() const { return indices; }
        const uint32_t GetVerticesSize() const { return static_cast<uint32_t>(vertices.size()); }
        const uint32_t GetIndicesSize() const { return static_cast<uint32_t>(indices.size()); }
        const Buffer &GetVertexBuffer() const { return vertexBuffer; }
        const Buffer &GetIndexBuffer() const { return indexBuffer; }

        void Upload(Device *inDevice);
        void Destroy();

    private:
        Device *device{nullptr};
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        Buffer vertexBuffer;
        Buffer indexBuffer;
    };
}
