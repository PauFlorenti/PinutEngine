#include "stdafx.h"

#include <imgui.h>
// Needs to go after imgui
#include <ImGuizmo.h>

#include "src/core/camera.h"
#include "src/core/entity.h"

namespace Pinut
{
constexpr glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 Entity::GetPosition() const { return m_transform[3]; }
glm::quat Entity::GetRotation() const { return glm::toQuat(m_transform); }
glm::vec3 Entity::GetScale() const
{
    glm::vec3 scale;
    scale.x = glm::length(glm::vec3(m_transform[0])); // Length of the first column
    scale.y = glm::length(glm::vec3(m_transform[1])); // Length of the second column
    scale.z = glm::length(glm::vec3(m_transform[2])); // Length of the third column
    return scale;
}
glm::vec3 Entity::GetDirection() const { return GetRotation() * forward; }
glm::mat4 Entity::GetTransform() const { return m_transform; }

void Entity::Transform(const glm::mat4& transform) { m_transform *= transform; }
void Entity::Position(const glm::vec3& position)
{
    m_transform = glm::translate(m_transform, position);
}
void Entity::Rotation(const glm::quat& rotation) { m_transform *= glm::toMat4(rotation); }
void Entity::Scale(const glm::vec3& scale) { glm::scale(m_transform, scale); }

void Entity::SetTransform(const glm::mat4& transform) { m_transform = transform; }

void Entity::SetPosition(const glm::vec3& position)
{
    const auto rotation       = glm::toMat4(GetRotation());
    const auto scale          = glm::scale(glm::mat4(1), GetScale());
    const auto positionMatrix = glm::translate(glm::mat4(1), position);
    m_transform               = positionMatrix * rotation * scale;
}

void Entity::SetRotation(const glm::quat& quaterion)
{
    const auto rotation = glm::toMat4(quaterion);
    const auto scale    = glm::scale(glm::mat4(1), GetScale());
    const auto position = glm::translate(glm::mat4(1), GetPosition());
    m_transform         = position * rotation * scale;
}

void Entity::SetScale(const glm::vec3& scale)
{
    const auto rotation    = glm::toMat4(GetRotation());
    const auto scaleMatrix = glm::scale(glm::mat4(1), scale);
    const auto position    = glm::translate(glm::mat4(1), GetPosition());
    m_transform            = position * rotation * scaleMatrix;
}

void Entity::DrawDebug(Camera* camera)
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

    SetTransform(std::move(m_transform));

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

    auto mat = GetTransform();

    // Using full screen
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    ImGuizmo::Manipulate(&camera->View()[0][0],
                         &camera->Projection()[0][0],
                         currentGuizmoOperation,
                         currentGuizmoMode,
                         &mat[0][0]);

    SetTransform(mat);
}

} // namespace Pinut
