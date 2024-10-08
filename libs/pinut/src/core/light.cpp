#include "stdafx.h"

#include "light.h"

#include <imgui.h>

constexpr glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);

namespace Pinut
{
void DirectionalLight::DrawDebug()
{
    if (ImGui::TreeNode("DirectionalLight"))
    {
        m_renderDebug = true;

        ImGui::Checkbox("Enabled", &m_enabled);
        ImGui::DragFloat3("Position", &m_transform[3][0], 0.1f);
        ImGui::ColorEdit3("Color", &m_color[0]);
        ImGui::DragFloat("Intensity", &m_intensity, 1, 0, FLT_MAX / 2);

        auto rotation = glm::toQuat(m_transform);
        auto euler    = glm::degrees(glm::eulerAngles(rotation));
        if (ImGui::DragFloat3("Rotation", &euler[0], 1.0f, -360.0f, 360.0f, "%0.f"))
            SetRotation(glm::quat(glm::radians(euler)));

        ImGui::TreePop();
    }
    else
        m_renderDebug = false;
}

void PointLight::DrawDebug()
{
    if (ImGui::TreeNode("Point Light"))
    {
        m_renderDebug = true;

        ImGui::Checkbox("Enabled", &m_enabled);
        ImGui::DragFloat3("Position", &m_transform[3][0], 0.1f);
        ImGui::ColorEdit3("Color", &m_color[0]);
        ImGui::DragFloat("Intensity", &m_intensity, 1, 0, FLT_MAX / 2);
        ImGui::DragFloat("Radius", &m_radius, 1, 0, FLT_MAX / 2);
        ImGui::TreePop();
    }
    else
        m_renderDebug = false;
}

void SpotLight::DrawDebug()
{
    if (ImGui::TreeNode("Spot Light"))
    {
        m_renderDebug = true;

        ImGui::Checkbox("Enabled", &m_enabled);
        ImGui::DragFloat3("Position", &m_transform[3][0], 0.1f);
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

        auto rotation = glm::toQuat(m_transform);
        auto euler    = glm::degrees(glm::eulerAngles(rotation));
        if (ImGui::DragFloat4("Rotation", &rotation[0], 0.01f, -1.0f, 1.0f))
        {
            SetRotation(glm::normalize(rotation));
        }

        ImGui::TreePop();
    }
    else
        m_renderDebug = false;
}

} // namespace Pinut
