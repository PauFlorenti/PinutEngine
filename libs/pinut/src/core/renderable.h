#pragma once

#include <glm/gtx/quaternion.hpp>

namespace Pinut
{
class Camera;
class Node;
class GPUBuffer;
struct MaterialInstance;
class Renderable final
{
 public:
   explicit Renderable(std::shared_ptr<Node> root, const std::string& name = "");

   void DrawImGui(Camera* camera);
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
