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
#ifdef _DEBUG
    void RenderDebug() override;
#endif

    explicit SkyComponent(Texture texture, const Mesh& mesh);

    void           SetTexture(Texture texture);
    const Texture& GetTexture() const;

    Mesh      m_mesh;
    glm::vec4 m_color{glm::vec4(1.0f)};
    bool      m_dirty{true};

  private:
    Texture m_texture;
};
} // namespace Component
} // namespace Pinut
