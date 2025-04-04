#include "pch.hpp"

#include <imgui/imgui.h>

#include "pinut/assets/material.h"
#include "pinut/components/renderComponent.h"

namespace Pinut
{
namespace Component
{
RenderComponent::RenderComponent(MaterialPtr InMaterial) : material(InMaterial) {}

#ifdef _DEBUG
void RenderComponent::RenderDebug()
{
    if (ImGui::TreeNode("Render Component"))
    {
        ImGui::Text("Render Component");
        ImGui::TreePop();
    }
}
#endif
} // namespace Component
} // namespace Pinut
