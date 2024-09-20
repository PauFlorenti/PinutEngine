#include "stdafx.h"

#include "node.h"
#include "src/assets/mesh.h"

namespace Pinut
{
void CollectAllNodes(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& allNodes)
{
    allNodes.push_back(std::const_pointer_cast<Node>(node));
    for (const auto& child : node->GetChildren())
    {
        CollectAllNodes(child, allNodes);
    }
}

std::vector<std::shared_ptr<Node>> Node::GetAllNodes(std::shared_ptr<Node> node)
{
    std::vector<std::shared_ptr<Node>> allNodes;
    CollectAllNodes(node, allNodes);
    return allNodes;
}

Node::Node() = default;

Node::Node(std::shared_ptr<Mesh> mesh, std::shared_ptr<Node> parent)
: m_mesh(mesh),
  m_parent(parent)
{
}

glm::mat4 Node::GetGlobalMatrix(bool fast)
{
    if (m_parent)
        m_globalMatrix = (fast ? m_parent->m_matrix : m_parent->GetGlobalMatrix() * m_matrix);
    else
        m_globalMatrix = m_matrix;

    return m_globalMatrix;
}

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() { return m_children; }

void Node::Draw(VkCommandBuffer cmd)
{
    assert(m_mesh);
    // TODO: Do material logic here.
    m_mesh->Draw(cmd);
}
} // namespace Pinut
