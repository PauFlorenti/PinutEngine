#pragma once

#include "pinut/assets/mesh.h"
#include "pinut/components/baseComponent.h"

namespace Pinut
{
class Renderer;
namespace Component
{
struct MeshComponent : public BaseComponent
{
    using MeshPtr = std::shared_ptr<Mesh>;
    MeshComponent(MeshPtr inMesh) : m_mesh(inMesh) {}

    friend Renderer;

    // BaseComponent
#ifdef _DEBUG
    void RenderDebug() override;
#endif

    MeshPtr m_mesh;
};
} // namespace Component
} // namespace Pinut
