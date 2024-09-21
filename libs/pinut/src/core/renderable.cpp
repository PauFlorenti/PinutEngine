#include "stdafx.h"

#include <imgui.h>

#include "renderable.h"
#include "src/assets/mesh.h"
#include "src/core/node.h"
#include "src/renderer/buffer.h"

namespace Pinut
{
Renderable::Renderable(std::shared_ptr<Node> root) : m_root(root)
{
    assert(m_root);
    m_allNodes = std::move(Node::GetAllNodes(m_root));
}

void Renderable::Draw(VkCommandBuffer cmd, VkPipelineLayout layout)
{
    for (const auto& node : m_allNodes)
    {
        node->BindMaterial(cmd, layout);
        node->Draw(cmd);
    }
}

void Renderable::DrawImGui()
{
    // TODO: Draw node imgui
    // if (ImGui::TreeNode(m_name.c_str()))
    // {
    //     auto       euler    = glm::degrees(glm::eulerAngles(m_rotation));
    //     const auto eulerAux = euler;

    //     ImGui::DragFloat3("Position", &m_position.x, 0.1f);
    //     ImGui::DragFloat3("Scale", &m_scale.x, 0.1f); // Constrain scale min to 0 ??
    //     if (ImGui::DragFloat3("Rotation", &euler[0], 1.f, -360.0f, 360.0f, "%.0f"))
    //     {
    //         const auto difference = eulerAux - euler;
    //         const auto quaternion = glm::quat(glm::radians(difference));
    //         m_rotation *= quaternion;
    //     }

    //     ImGui::TreePop();
    // }
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
