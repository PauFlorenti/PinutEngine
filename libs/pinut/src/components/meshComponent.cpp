#include "pch.hpp"

#include <imgui/imgui.h>

#include "pinut/components/meshComponent.h"

namespace Pinut
{
namespace Component
{
#ifdef _DEBUG
void MeshComponent::RenderDebug()
{
    if (ImGui::TreeNode("Mesh Component"))
    {
        // TODO Show path ...
        ImGui::Text("Mesh");
        ImGui::TreePop();
    }
}
#endif
} // namespace Component
} // namespace Pinut
