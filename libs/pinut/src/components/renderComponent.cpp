#include "stdafx.h"

#include <external/imgui/imgui.h>

#include "src/components/renderComponent.h"

namespace Pinut
{
namespace Component
{
RenderComponent::RenderComponent(Texture inDifuse,
                                 Texture inNormal,
                                 Texture inMetallicRoughness,
                                 Texture inEmissive)
: difuse(inDifuse),
  normal(inNormal),
  metallicRoughness(inMetallicRoughness),
  emissive(inEmissive)
{
}

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
