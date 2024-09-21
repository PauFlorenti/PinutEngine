#pragma once

#include <glm/gtx/quaternion.hpp>

namespace Pinut
{
class Node;
class GPUBuffer;
struct MaterialInstance;
class Renderable final
{
  public:
    explicit Renderable(std::shared_ptr<Node> root);

    void Draw(VkCommandBuffer cmd, VkPipelineLayout layout);
    void DrawImGui();
    void Destroy();
    bool IsValid() const;

    void SetTransform(const glm::mat4& transform);

    const std::vector<std::shared_ptr<Node>>& GetAllNodes();

  private:
    std::string                        m_name;
    std::shared_ptr<Node>              m_root{nullptr};
    std::vector<std::shared_ptr<Node>> m_allNodes;
};
} // namespace Pinut
