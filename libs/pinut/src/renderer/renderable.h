#pragma once

namespace Pinut
{
class Mesh;
class Material;
class GPUBuffer;
class Renderable final
{
  public:
    Renderable() = default;

    const glm::mat4& Model() const { return m_model; }
    void             SetMesh(Mesh* m) { m_mesh = m; }
    void             SetMaterial(Material* m) { m_material = m; }
    void             SetModel(glm::mat4 m) { m_model = m; }

    void Draw(VkCommandBuffer cmd) const;
    void Destroy();
    bool IsValid() const;

  private:
    Mesh*      m_mesh{nullptr};
    Material*  m_material{nullptr};
    glm::mat4  m_model{};
    GPUBuffer* m_buffer{nullptr};
};
} // namespace Pinut
