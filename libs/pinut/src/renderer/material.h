#pragma once

namespace Pinut
{
class Texture;
class Material
{
  public:
    void Destroy();

    void SetAlbedo(Texture* t);
    void SetNormal(Texture* t);
    void SetMetallicRoughness(Texture* t);

    Texture* GetAlbedo() { return m_albedo; }
    Texture* GetNormal() { return m_normal; }
    Texture* GetMetallicRoughness() { return m_metalicRoughness; }

  private:
    Texture* m_albedo{nullptr};
    Texture* m_normal{nullptr};
    Texture* m_metalicRoughness{nullptr};
};
} // namespace Pinut
