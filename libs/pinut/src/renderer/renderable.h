#pragma once

#include <glm/gtx/quaternion.hpp>

namespace Pinut
{
class Mesh;
class GPUBuffer;
struct MaterialInstance;
class Renderable final
{
  public:
    explicit Renderable(std::string name);

    const glm::mat4       Model() const;
    glm::vec3&            Position() { return m_position; }
    glm::vec3&            Scale() { return m_scale; }
    glm::quat&            Rotation() { return m_rotation; }
    std::shared_ptr<Mesh> GetMesh() { return m_mesh; }
    const u32             InstanceIndex() const { return m_instanceIndex; }

    // void SetMaterial(std::shared_ptr<MaterialInstance> material);
    void SetMesh(std::shared_ptr<Mesh> m) { m_mesh = m; }
    void SetModel(glm::mat4 m);
    void SetInstanceIndex(u32 index) { m_instanceIndex = index; }

    void DrawImGui();
    void Destroy();
    bool IsValid() const;

  private:
    std::string           m_name;
    std::shared_ptr<Mesh> m_mesh{nullptr};
    u32                   m_instanceIndex{0}; // Used to map the model in the StorageBuffer
    glm::vec3             m_position{};
    glm::quat             m_rotation{};
    glm::vec3             m_scale{};
};
} // namespace Pinut
