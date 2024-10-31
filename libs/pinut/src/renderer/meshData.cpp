#include "stdafx.h"

#include "src/renderer/meshData.h"

namespace Pinut
{
MeshData::~MeshData()
{
    m_vertexBuffer.Destroy();
    m_indexBuffer.Destroy();
}
} // namespace Pinut
