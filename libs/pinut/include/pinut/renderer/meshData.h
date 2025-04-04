#pragma once

#include "render_device/buffer.h"

namespace RED
{
class Device;
}
namespace Pinut
{
class Mesh;
void CreateMeshData(std::shared_ptr<RED::Device> InDevice,
                    entt::registry&              InRegistry,
                    std::shared_ptr<Mesh>        InMesh);

struct MeshData
{
    ~MeshData();

    RED::GPUBuffer m_vertexBuffer;
    RED::GPUBuffer m_indexBuffer;
};
} // namespace Pinut
