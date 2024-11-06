#pragma once

#include "src/core/entity.h"

namespace Pinut
{
class Camera;
class Material;
class Mesh;
class Node : public Entity
{
  public:
    static std::vector<std::shared_ptr<Node>> GetAllNodes(std::shared_ptr<Node> node);

    Node();
    explicit Node(std::shared_ptr<Mesh> mesh, std::shared_ptr<Node> parent = nullptr);

    std::shared_ptr<Node> GetParent() const { return m_parent; }
    void                  SetParent(std::shared_ptr<Node> parent) { m_parent = parent; }
    bool                  HasParent() { return m_parent != nullptr; }
    void                  AddChild(std::shared_ptr<Node> node) { m_children.push_back(node); };

    glm::mat4             GetGlobalMatrix(bool fast = true);
    u32                   GetInstanceIndex() const { return m_instanceIndex; }
    std::shared_ptr<Mesh> GetMesh() const { return m_mesh; }

    void SetMesh(std::shared_ptr<Mesh> mesh) { m_mesh = mesh; }
    void SetInstanceIndex(u32 i) { m_instanceIndex = std::move(i); }

    const std::vector<std::shared_ptr<Node>>& GetChildren();

    void DrawDebug(Camera* camera) override;

    entt::entity m_renderId;

  private:
    std::shared_ptr<Mesh>              m_mesh{nullptr};
    std::shared_ptr<Node>              m_parent{nullptr};
    std::vector<std::shared_ptr<Node>> m_children;

    glm::mat4 m_globalTransform;

    u32 m_instanceIndex{0};
};
} // namespace Pinut
