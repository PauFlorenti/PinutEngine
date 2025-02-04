#include "stdafx.h"

#include <external/imgui/imgui.h>
// This has to go after imgui.h
#include <external/ImGuizmo/ImGuizmo.h>

#include "src/components/transformComponent.h"
#include "transformComponent.h"

namespace Pinut
{
namespace Component
{
constexpr glm::vec3 FORWARD = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr glm::vec3 UP      = glm::vec3(0.0f, 1.0f, 0.0f);

TransformComponent::TransformComponent(const glm::mat4& transform) : m_transform(transform) {}

TransformComponent::TransformComponent(const glm::vec3& position,
                                       const glm::vec3& rotation,
                                       const glm::vec3& scale)
{
    auto quaternion = glm::quat(rotation);
    ImGuizmo::RecomposeMatrixFromComponents(&position.x,
                                            &quaternion.x,
                                            &scale.x,
                                            &m_transform[0][0]);
}

glm::vec3 TransformComponent::GetPosition() const { return m_transform[3]; }

glm::mat4 TransformComponent::GetTransform() const { return m_transform; }

glm::quat TransformComponent::GetRotation() const { return glm::toQuat(m_transform); }

glm::vec3 TransformComponent::GetScale() const
{
    return {glm::length(glm::vec3(m_transform[0])),
            glm::length(glm::vec3(m_transform[1])),
            glm::length(glm::vec3(m_transform[2]))};
}

glm::vec3 TransformComponent::GetForward() const { return GetRotation() * FORWARD; }

void TransformComponent::Transform(const glm::mat4& transform) { m_transform *= transform; }

void TransformComponent::Translate(const glm::vec3& position)
{
    m_transform = glm::translate(m_transform, position);
}

void TransformComponent::Rotate(const glm::quat& rotation) { m_transform *= glm::toMat4(rotation); }

void TransformComponent::Scale(const glm::vec3& scale) { glm::scale(m_transform, scale); }

void TransformComponent::SetTransform(const glm::mat4& transform) { m_transform = transform; }

void TransformComponent::SetPosition(const glm::vec3& position)
{
    const auto rotation       = glm::toMat4(GetRotation());
    const auto scale          = glm::scale(glm::mat4(1), GetScale());
    const auto positionMatrix = glm::translate(glm::mat4(1), position);
    m_transform               = positionMatrix * rotation * scale;
}

void TransformComponent::SetRotation(const glm::quat& quaterion)
{
    const auto rotation = glm::toMat4(quaterion);
    const auto scale    = glm::scale(glm::mat4(1), GetScale());
    const auto position = glm::translate(glm::mat4(1), GetPosition());
    m_transform         = position * rotation * scale;
}

void TransformComponent::SetScale(const glm::vec3& scale)
{
    const auto rotation    = glm::toMat4(GetRotation());
    const auto scaleMatrix = glm::scale(glm::mat4(1), scale);
    const auto position    = glm::translate(glm::mat4(1), GetPosition());
    m_transform            = position * rotation * scaleMatrix;
}

#ifdef _DEBUG
void TransformComponent::RenderDebug(const ShaderCameraData& camera)
{
    if (ImGui::TreeNode("Transform Component"))
    {
        static ImGuizmo::OPERATION currentGuizmoOperation(ImGuizmo::TRANSLATE);
        static ImGuizmo::MODE      currentGuizmoMode(ImGuizmo::WORLD);

        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(&m_transform[0][0],
                                              &translation.x,
                                              &rotation.x,
                                              &scale.x);
        ImGui::DragFloat3("Translation", &translation.x, 3);
        ImGui::DragFloat3("Rotation", &rotation.x, 3);
        ImGui::DragFloat3("Scale", &scale.x, 3);

        ImGuizmo::RecomposeMatrixFromComponents(&translation.x,
                                                &rotation.x,
                                                &scale.x,
                                                &m_transform[0][0]);

        if (ImGui::RadioButton("Translate", currentGuizmoOperation == ImGuizmo::TRANSLATE))
            currentGuizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", currentGuizmoOperation == ImGuizmo::ROTATE))
            currentGuizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", currentGuizmoOperation == ImGuizmo::SCALE))
            currentGuizmoOperation = ImGuizmo::SCALE;
        ImGui::SameLine();

        if (currentGuizmoOperation != ImGuizmo::SCALE)
        {
            if (ImGui::RadioButton("Local", currentGuizmoMode == ImGuizmo::LOCAL))
                currentGuizmoMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", currentGuizmoMode == ImGuizmo::WORLD))
                currentGuizmoMode = ImGuizmo::WORLD;
        }

        bool changed = false;
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset"))
        {
            if (currentGuizmoOperation == ImGuizmo::TRANSLATE)
                SetPosition(glm::vec3(0.0f));
            else if (currentGuizmoOperation == ImGuizmo::ROTATE)
                SetRotation(glm::quat(glm::vec3(0.0f)));
            else if (currentGuizmoOperation == ImGuizmo::SCALE)
                SetScale(glm::vec3(1.0f));
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("All"))
        {
            SetPosition(glm::vec3(0.0f));
            SetRotation(glm::quat(glm::vec3(0.0f)));
            SetScale(glm::vec3(1.0f));
            changed = true;
        }

        auto mat = m_transform;

        // Using full screen
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

        ImGuizmo::Manipulate(&camera.view[0][0],
                             &camera.projection[0][0],
                             currentGuizmoOperation,
                             currentGuizmoMode,
                             &mat[0][0]);

        SetTransform(mat);

        ImGui::TreePop();
    }
}
#endif
} // namespace Component
} // namespace Pinut
