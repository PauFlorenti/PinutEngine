#pragma once

#include <glm/gtx/quaternion.hpp>

namespace Pinut
{
class Mesh;
class Material;
class GPUBuffer;
class Renderable final
{
  public:
    Renderable();

    const glm::mat4 Model() const;
    glm::vec3&      Position() { return m_position; }
    glm::vec3&      Scale() { return m_scale; }
    glm::quat&      Rotation() { return m_rotation; }

    void SetMaterial(std::shared_ptr<Material> m) { m_material = m; }
    void SetMesh(std::shared_ptr<Mesh> m) { m_mesh = m; }
    void SetModel(glm::mat4 m);
    void SetInstanceIndex(u32 index) { m_instanceIndex = index; }

    void Draw(VkCommandBuffer cmd) const;
    void Destroy();
    bool IsValid() const;

  private:
    std::shared_ptr<Mesh>     m_mesh{nullptr};
    std::shared_ptr<Material> m_material{nullptr};
    u32                       m_instanceIndex{0}; // Used to map the model in the StorageBuffer
    glm::vec3                 m_position{};
    glm::quat                 m_rotation{};
    glm::vec3                 m_scale{};
};
} // namespace Pinut
