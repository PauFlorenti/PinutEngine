#include "stdafx.h"

#include <imgui.h>

#include "renderable.h"
#include "src/assets/mesh.h"
#include "src/renderer/buffer.h"

namespace Pinut
{
Renderable::Renderable(std::string name) : m_name(name) {}

const glm::mat4 Renderable::Model() const
{
    return glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(m_rotation) *
           glm::scale(glm::mat4(1.0f), m_scale);
}

void Renderable::SetModel(glm::mat4 model)
{
    glm::vec3 skew{};
    glm::vec4 perspective{};
    auto      ok = glm::decompose(model, m_scale, m_rotation, m_position, skew, perspective);
    assert(ok == true);
}

void Renderable::DrawImGui()
{
    if (ImGui::TreeNode(m_name.c_str()))
    {
        auto       euler    = glm::degrees(glm::eulerAngles(m_rotation));
        const auto eulerAux = euler;

        ImGui::DragFloat3("Position", &m_position.x, 0.1f);
        ImGui::DragFloat3("Scale", &m_scale.x, 0.1f); // Constrain scale min to 0 ??
        if (ImGui::DragFloat3("Rotation", &euler[0], 1.f, -360.0f, 360.0f, "%.0f"))
        {
            const auto difference = eulerAux - euler;
            const auto quaternion = glm::quat(glm::radians(difference));
            m_rotation *= quaternion;
        }

        ImGui::TreePop();
    }
}

void Renderable::Destroy() {}

bool Renderable::IsValid() const { return m_mesh != nullptr; }

} // namespace Pinut
