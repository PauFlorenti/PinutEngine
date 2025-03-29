#include "pch.hpp"

#include <imgui/imgui.h>

#include "pinut/components/lightComponent.h"

namespace Pinut
{
namespace Component
{
#ifdef _DEBUG
void LightComponent::RenderDebug()
{
    if (ImGui::TreeNode("Light Component"))
    {
        ImGui::Checkbox("Enabled", &m_enabled);
        ImGui::ColorEdit3("Color", &m_color[0]);
        ImGui::DragFloat("Intensity", &m_intensity, 1, 0, FLT_MAX / 2);
        ImGui::DragFloat("Radius", &m_radius, 1, 0, FLT_MAX / 2);
        ImGui::DragFloat("Spot Light Inner Angle", &m_innerCone, 1.0f, 0.0f, m_outerCone);
        if (ImGui::DragFloat("Spot Light Outer Angle", &m_outerCone, 1.0f, 0.0f, 89.9f))
        {
            if (m_outerCone < m_innerCone)
                m_innerCone = m_outerCone;
        }
        ImGui::DragFloat("Angle exponent", &m_cosineExponent, 1.0f, 0.0f, 100.0f, "%.0f");
        ImGui::TreePop();
    }
}
#endif
} // namespace Component
} // namespace Pinut
