#include "stdafx.h"

#include <external/imgui/imgui.h>

#include "src/components/skyComponent.h"
#include "src/loaders/objLoader.h"
#include "src/renderer/primitives.h"

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
