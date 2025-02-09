#pragma once

#include "render_device/buffer.h"

namespace RED
{
class Device;
}
namespace Pinut
{
class Mesh;
namespace Component
{
struct MeshComponent;
} // namespace Component
entt::entity CreateMeshData(std::shared_ptr<RED::Device> device,
                            entt::registry&              registry,
                            Mesh&                        meshComponent);

struct MeshData
{
    ~MeshData();

    RED::GPUBuffer m_vertexBuffer;
    RED::GPUBuffer m_indexBuffer;
};
} // namespace Pinut
