#include "stdafx.h"

#include "light.h"

#include <imgui.h>

void Pinut::Light::DrawImGUI()
{
    if (ImGui::TreeNode("Light"))
    {
        ImGui::DragFloat3("Position", &position[0]);
        ImGui::ColorEdit3("Color", &color[0]);
        ImGui::DragFloat("Intensity", &intensity, 1, 0, FLT_MAX / 2);
        ImGui::DragFloat("Radius", &radius, 1, 0, FLT_MAX / 2);

        ImGui::TreePop();
    }
}
