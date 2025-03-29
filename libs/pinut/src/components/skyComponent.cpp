#include "pch.hpp"

#include <imgui/imgui.h>

#include "pinut/components/skyComponent.h"
#include "pinut/loaders/objLoader.h"
#include "pinut/renderer/primitives.h"

namespace Pinut
{
namespace Component
{
SkyComponent::SkyComponent(Texture texture, const Mesh& mesh)
: m_texture(std::move(texture)),
  m_mesh(mesh)
{
}

void SkyComponent::SetTexture(Texture texture)
{
    m_texture = std::move(texture);
    m_dirty   = true;
}

const Texture& SkyComponent::GetTexture() const { return m_texture; }

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
} // namespace Component
} // namespace Pinut
