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
    void RenderDebug() override
    {
        if (ImGui::TreeNode("Mesh Component"))
        {
            // TODO Show path ...
            ImGui::Text("Mesh");
            ImGui::TreePop();
        }
    }
#endif

    Mesh mesh;
};
} // namespace Component
} // namespace Pinut
