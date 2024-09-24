#include "stdafx.h"

#include "light.h"

#include <imgui.h>

constexpr glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);

namespace Pinut
{

void Light::DrawDebug(Camera* camera) { Entity::DrawDebug(camera); }

void DirectionalLight::DrawDebug(Camera* camera)
{
    if (ImGui::TreeNode("DirectionalLight"))
    {
        m_renderDebug = true;

        Entity::DrawDebug(camera);
        ImGui::Checkbox("Enabled", &m_enabled);
        ImGui::ColorEdit3("Color", &m_color[0]);
        ImGui::DragFloat("Intensity", &m_intensity, 1, 0, FLT_MAX / 2);

        ImGui::TreePop();
    }
    else
        m_renderDebug = false;
}

void PointLight::DrawDebug(Camera* camera)
{
    if (ImGui::TreeNode("Point Light"))
    {
        m_renderDebug = true;

        Entity::DrawDebug(camera);

        ImGui::Checkbox("Enabled", &m_enabled);
        ImGui::ColorEdit3("Color", &m_color[0]);
        ImGui::DragFloat("Intensity", &m_intensity, 1, 0, FLT_MAX / 2);
        ImGui::DragFloat("Radius", &m_radius, 1, 0, FLT_MAX / 2);
        ImGui::TreePop();
    }
    else
        m_renderDebug = false;
}

void SpotLight::DrawDebug(Camera* camera)
{
    if (ImGui::TreeNode("Spot Light"))
    {
        m_renderDebug = true;

        Entity::DrawDebug(camera);

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
    else
        m_renderDebug = false;
}

} // namespace Pinut
