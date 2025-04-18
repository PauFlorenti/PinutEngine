#include "pch.hpp"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"

#include "pinut/assets/mesh.h"
#include "pinut/renderer/meshData.h"

namespace Pinut
{
void CreateMeshData(std::shared_ptr<RED::Device> device, entt::registry& registry, Mesh& mesh)
{
    if (auto data = registry.try_get<MeshData>(mesh.m_handle); data)
        return;

    const auto handle = registry.create();
    mesh.m_handle     = handle;
    auto& data        = registry.emplace<MeshData>(handle);

    auto& vertices      = mesh.m_vertices;
    data.m_vertexBuffer = device->CreateBuffer(
      {vertices.size() * sizeof(Vertex), sizeof(Vertex), RED::BufferUsage::VERTEX},
      vertices.data());

    if (!mesh.m_indices.empty())
    {
        auto& indices = mesh.m_indices;
        data.m_indexBuffer =
          device->CreateBuffer({indices.size() * sizeof(u16), sizeof(u16), RED::BufferUsage::INDEX},
                               indices.data());
    }
}

MeshData::~MeshData()
{
    m_vertexBuffer.Destroy();
    m_indexBuffer.Destroy();
}
} // namespace Pinut
