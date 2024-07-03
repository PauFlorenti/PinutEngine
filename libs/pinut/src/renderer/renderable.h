#pragma once

namespace Pinut
{
class Mesh;
class Material;
class Renderable final
{
  public:
    void SetMesh(Mesh* m) { m_mesh = m; }
    void SetMaterial(Material* m) { m_material = m; }
    void Draw(VkCommandBuffer cmd) const;
    void Destroy();
    bool IsValid() const;

  private:
    Mesh*     m_mesh{nullptr};
    Material* m_material{nullptr};
};
} // namespace Pinut
