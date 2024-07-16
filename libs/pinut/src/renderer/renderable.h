#pragma once

namespace Pinut
{
class Mesh;
class Material;
class GPUBuffer;
class Renderable final
{
  public:
    Renderable();

    const glm::mat4& Model() const { return m_model; }
    void             SetMesh(std::shared_ptr<Mesh> m) { m_mesh = m; }
    void             SetMaterial(std::shared_ptr<Material> m) { m_material = m; }
    void             SetModel(glm::mat4 m) { m_model = m; }
    void             SetInstanceIndex(u32 index) { m_instanceIndex = index; }

    void Draw(VkCommandBuffer cmd) const;
    void Destroy();
    bool IsValid() const;

  private:
    std::shared_ptr<Mesh>     m_mesh{nullptr};
    std::shared_ptr<Material> m_material{nullptr};
    glm::mat4                 m_model{};
    u32                       m_instanceIndex{0}; // Used to map the model in the StorageBuffer
};
} // namespace Pinut
