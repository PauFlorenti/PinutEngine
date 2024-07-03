#pragma once

namespace Pinut
{
class Mesh;
class Renderable final
{
  public:
    void SetMesh(Mesh* m) { m_mesh = m; }
    void Draw(VkCommandBuffer cmd) const;
    void Destroy();
    bool IsValid() const;

  private:
    Mesh* m_mesh{nullptr};
};
} // namespace Pinut
