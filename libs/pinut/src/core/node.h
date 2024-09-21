#pragma once

#include "src/core/entity.h"

namespace Pinut
{
class Material;
class Mesh;
class Node : public Entity
{
  public:
    static std::vector<std::shared_ptr<Node>> GetAllNodes(std::shared_ptr<Node> node);

    Node();
    explicit Node(std::shared_ptr<Mesh> mesh, std::shared_ptr<Node> parent = nullptr);

    std::shared_ptr<Node> GetParent() const { return m_parent; }
    bool                  HasParent() { return m_parent != nullptr; }
    void                  AddChild(std::shared_ptr<Node> node) { m_children.push_back(node); };

    glm::mat4 GetGlobalMatrix(bool fast = true);
    u32       GetInstanceIndex() const { return m_instanceIndex; }

    void SetMatrix(const glm::mat4& matrix) { m_matrix = matrix; }
    void SetMesh(std::shared_ptr<Mesh> mesh) { m_mesh = mesh; }
    void SetMaterial(std::shared_ptr<Material> material) { m_material = material; }
    void SetInstanceIndex(u32 i) { m_instanceIndex = std::move(i); }

    const std::vector<std::shared_ptr<Node>>& GetChildren();

    void BindMaterial(VkCommandBuffer cmd, VkPipelineLayout layout);
    void Draw(VkCommandBuffer cmd);
    void DrawDebug() override {};

  private:
    std::shared_ptr<Mesh>              m_mesh{nullptr};
    std::shared_ptr<Material>          m_material{nullptr};
    std::shared_ptr<Node>              m_parent{nullptr};
    std::vector<std::shared_ptr<Node>> m_children;

    glm::mat4 m_matrix{glm::mat4(1.0f)};
    glm::mat4 m_globalMatrix{glm::mat4(1.0f)};

    u32 m_instanceIndex{0};
};
} // namespace Pinut
