#pragma once

#include "pinut/assets/mesh.h"
#include "pinut/components/baseComponent.h"

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
