#include "stdafx.h"

#include "light.h"

#include <imgui.h>

void Pinut::Light::DrawImGUI()
{
    if (ImGui::TreeNode("Light"))
    {
        ImGui::DragFloat3("Position", &position[0], 0.1f);
        ImGui::ColorEdit3("Color", &color[0]);
        ImGui::DragFloat("Intensity", &intensity, 1, 0, FLT_MAX / 2);
        ImGui::DragFloat("Radius", &radius, 1, 0, FLT_MAX / 2);
        ImGui::DragFloat("Spot Light Inner Angle", &innerCone, 1.0f, -1.0f, outerCone);
        ImGui::DragFloat("Spot Light Outer Angle", &outerCone, 1.0f, innerCone, 89.9f);
        ImGui::DragFloat("Angle exponent", &cosineExponent, 1.0f, 0.0f, 100.0f, "%.0f");

        auto euler = glm::degrees(glm::eulerAngles(rotation));
        const auto eulerAux = euler;
        if (ImGui::DragFloat3("Rotation", &euler[0], 1.0f, -360.0f, 360.0f, "%0.f"))
        {
            const auto difference = euler - eulerAux;
            rotation *= glm::quat(glm::radians(difference));
        }

        ImGui::TreePop();
    }
}
