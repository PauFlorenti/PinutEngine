#pragma once

#include "pinut/assets/mesh.h"
#include "pinut/assets/texture.h"
#include "pinut/components/baseComponent.h"

namespace Pinut
{
namespace Component
{
struct SkyComponent : public BaseComponent
{
    using TexturePtr = std::shared_ptr<Texture>;
    using MeshPtr    = std::shared_ptr<Mesh>;

    explicit SkyComponent(TexturePtr InTexture, MeshPtr mesh);

// BaseComponent
#ifdef _DEBUG
    void RenderDebug() override;
#endif

    void       SetTexture(TexturePtr InTexture);
    TexturePtr GetTexture() const;

    void    SetMesh(MeshPtr InMesh);
    MeshPtr GetMesh() const;

    glm::vec4 m_color{glm::vec4(1.0f)};
    bool      m_dirty{true};

  private:
    MeshPtr    m_mesh;
    TexturePtr m_texture;
};
} // namespace Component
} // namespace Pinut
