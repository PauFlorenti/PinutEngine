#include "pch.hpp"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"

#include "pinut/assets/mesh.h"
#include "pinut/renderer/meshData.h"

namespace Pinut
{
void CreateMeshData(std::shared_ptr<RED::Device> InDevice,
                    entt::registry&              InRegistry,
                    std::shared_ptr<Mesh>        InMesh)
{
    if (auto data = InRegistry.try_get<MeshData>(InMesh->m_handle); data)
        return;

    const auto handle = InRegistry.create();
    InMesh->m_handle  = handle;
    auto& data        = InRegistry.emplace<MeshData>(handle);

    auto& vertices      = InMesh->m_vertices;
    data.m_vertexBuffer = InDevice->CreateBuffer(
      {vertices.size() * sizeof(Vertex), sizeof(Vertex), RED::BufferUsage::VERTEX},
      vertices.data());

    if (!InMesh->m_indices.empty())
    {
        auto& indices      = InMesh->m_indices;
        data.m_indexBuffer = InDevice->CreateBuffer(
          {indices.size() * sizeof(u16), sizeof(u16), RED::BufferUsage::INDEX},
          indices.data());
    }
}

MeshData::~MeshData()
{
    m_vertexBuffer.Destroy();
    m_indexBuffer.Destroy();
}
} // namespace Pinut
