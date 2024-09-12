#include "stdafx.h"

#include "node.h"

#namespace Pinut
{
    Node::Node(std::shared_ptr<Mesh> mesh, std::shared_ptr<Node> parent)
    : m_mesh(mesh),
      m_parent(parent)
    {
    }
}
