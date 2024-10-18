#include "stdafx.h"

#include <imgui.h>
// Needs to go after imgui
#include <ImGuizmo.h>

#include "renderable.h"
#include "src/assets/mesh.h"
#include "src/core/camera.h"
#include "src/core/node.h"

namespace Pinut
{
Renderable::Renderable(std::shared_ptr<Node> root, const std::string& name)
: m_root(root),
  m_name(name)
{
    assert(m_root);
    m_allNodes = std::move(Node::GetAllNodes(m_root));
}

void Renderable::DrawImGui(Camera* camera)
{
    // TODO: Draw node imgui
    if (ImGui::TreeNode(m_name.c_str()))
    {
        m_root->DrawDebug(std::move(camera));
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
