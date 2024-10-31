#pragma once

#include "render_device/buffer.h"

namespace Pinut
{
struct MeshData
{
    ~MeshData();

    RED::GPUBuffer m_vertexBuffer;
    RED::GPUBuffer m_indexBuffer;
};
} // namespace Pinut
