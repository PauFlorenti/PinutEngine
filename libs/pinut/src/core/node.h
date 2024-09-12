#pragma once

#include "src/core/entity.h"

namespace Pinut
{
class Material;
class Mesh;
class Node : public Entity
{
  public:
    explicit Node(std::shared_ptr<Mesh> mesh, std::shared_ptr<Node> parent = nullptr);

    std::shared_ptr<Node> GetParent() const { return m_parent; }
    bool                  HasParent() { return m_parent != nullptr; }
    void                  AddChild(std::shared_ptr<Node> node) { m_children.push_back(node); };

  private:
    std::shared_ptr<Mesh>              m_mesh{nullptr};
    std::shared_ptr<Node>              m_parent{nullptr};
    std::vector<std::shared_ptr<Node>> m_children;
};
} // namespace Pinut
