#include "pch.hpp"

#include <imgui/imgui.h>

#include "pinut/components/skyComponent.h"
#include "pinut/loaders/objLoader.h"
#include "pinut/renderer/primitives.h"

namespace Pinut
{
namespace Component
{
SkyComponent::SkyComponent(TexturePtr InTexture, MeshPtr InMesh)
: m_texture(std::move(InTexture)),
  m_mesh(InMesh)
{
}

#ifdef _DEBUG
void SkyComponent::RenderDebug()
{
    if (ImGui::TreeNode("Sky Component"))
    {
        ImGui::Text("Sky component");
        ImGui::TreePop();
    }
}
#endif

void SkyComponent::SetTexture(TexturePtr texture)
{
    m_texture = std::move(texture);
    m_dirty   = true;
}

SkyComponent::TexturePtr SkyComponent::GetTexture() const { return m_texture; }

void SkyComponent::SetMesh(MeshPtr InMesh) { m_mesh = InMesh; }

SkyComponent::MeshPtr SkyComponent::GetMesh() const { return m_mesh; }
} // namespace Component
} // namespace Pinut
