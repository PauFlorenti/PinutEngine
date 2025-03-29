#include "pch.hpp"

#include <imgui/imgui.h>

#include "pinut/assets/material.h"
#include "pinut/components/renderComponent.h"

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

RenderComponent::RenderComponent(std::shared_ptr<Material> InMaterial)
: material(InMaterial),
  difuse(InMaterial->m_diffuseTexture),
  normal(InMaterial->m_normalTexture),
  metallicRoughness(InMaterial->m_metallicRoughnessTexture),
  emissive(InMaterial->m_emissiveTexture)
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
