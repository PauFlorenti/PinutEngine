#pragma once

#include "src/assets/mesh.h"
#include "src/components/baseComponent.h"

namespace Pinut
{
namespace Component
{
struct MeshComponent : public BaseComponent
{
    MeshComponent(const Mesh& inMesh) : mesh(inMesh) {}

#ifdef _DEBUG
    void RenderDebug() override;
#endif

    Mesh mesh;
};
} // namespace Component
} // namespace Pinut
