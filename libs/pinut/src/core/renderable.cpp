#include "stdafx.h"

#include <imgui.h>
// Needs to go after imgui
#include <ImGuizmo.h>

#include "renderable.h"
#include "src/assets/mesh.h"
#include "src/core/camera.h"
#include "src/core/node.h"
#include "src/renderer/buffer.h"

namespace Pinut
{
namespace
{
void DrawGuizmo(std::shared_ptr<Node> node, Camera* camera)
{
    assert(node);
    assert(camera);

    static ImGuizmo::OPERATION currentGuizmoOperation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE      currentGuizmoMode(ImGuizmo::WORLD);

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
            node->SetPosition(glm::vec3(0.0f));
        else if (currentGuizmoOperation == ImGuizmo::ROTATE)
            node->SetRotation(glm::quat(glm::vec3(0.0f)));
        else if (currentGuizmoOperation == ImGuizmo::SCALE)
            node->SetScale(glm::vec3(1.0f));
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("All"))
    {
        node->SetPosition(glm::vec3(0.0f));
        node->SetRotation(glm::quat(glm::vec3(0.0f)));
        node->SetScale(glm::vec3(1.0f));
        changed = true;
    }

    auto mat = node->GetTransform();

    // Using full screen
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    ImGuizmo::Manipulate(&camera->View()[0][0],
                         &camera->Projection()[0][0],
                         currentGuizmoOperation,
                         currentGuizmoMode,
                         &mat[0][0]);

    node->SetTransform(mat);
}
} // namespace
Renderable::Renderable(std::shared_ptr<Node> root, const std::string& name)
: m_root(root),
  m_name(name)
{
    assert(m_root);
    m_allNodes = std::move(Node::GetAllNodes(m_root));
}

void Renderable::Draw(VkCommandBuffer cmd, VkPipelineLayout layout)
{
    for (const auto& node : m_allNodes)
    {
        node->Draw(cmd, layout);
    }
}

void Renderable::DrawImGui(Camera* camera)
{
    // TODO: Draw node imgui
    if (ImGui::TreeNode(m_name.c_str()))
    {
        auto      matrix = m_root->GetTransform();
        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(&matrix[0][0], &translation.x, &rotation.x, &scale.x);
        ImGui::DragFloat3("Translation", &translation.x, 3);
        ImGui::DragFloat3("Rotation", &rotation.x, 3);
        ImGui::DragFloat3("Scale", &scale.x, 3);

        ImGuizmo::RecomposeMatrixFromComponents(&translation.x,
                                                &rotation.x,
                                                &scale.x,
                                                &matrix[0][0]);

        m_root->SetTransform(std::move(matrix));

        DrawGuizmo(m_root, camera);

        ImGui::TreePop();
    }
}

void Renderable::Destroy() {}

bool Renderable::IsValid() const { return m_root != nullptr; }

void Renderable::SetTransform(const glm::mat4& transform)
{
    assert(m_root);
    m_root->SetTransform(transform);
}

const std::vector<std::shared_ptr<Node>>& Renderable::GetAllNodes() { return m_allNodes; }

} // namespace Pinut
