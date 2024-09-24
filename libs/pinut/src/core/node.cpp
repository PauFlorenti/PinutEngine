#include "stdafx.h"

#include "node.h"
#include "src/assets/material.h"
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
        m_globalTransform =
          (fast ? m_parent->GetTransform() : m_parent->GetGlobalMatrix() * m_transform);
    else
        m_globalTransform = m_transform;

    return m_globalTransform;
}

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() { return m_children; }

void Node::Draw(VkCommandBuffer cmd, VkPipelineLayout layout)
{
    if (m_mesh)
        m_mesh->Draw(cmd, layout, m_instanceIndex);
}

void Node::DrawDebug(Camera* camera) { Entity::DrawDebug(std::move(camera)); }
} // namespace Pinut
