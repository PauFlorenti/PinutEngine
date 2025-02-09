#pragma once

#include "src/assets/mesh.h"
#include "src/components/baseComponent.h"
#include "src/renderer/meshData.h"

namespace Pinut
{
namespace Component
{
struct MeshComponent : public BaseComponent
{
    MeshComponent(const Mesh& inMesh) : m_mesh(inMesh) {}

#ifdef _DEBUG
    void RenderDebug() override;
#endif

    Mesh m_mesh;
};
} // namespace Component
} // namespace Pinut
