#pragma once

#include <array>

#include "src/assets/mesh.h"
#include "src/core/light.h"

namespace Pinut
{
class Renderable;
class Scene
{
  public:
    void LoadScene();
    void SetDirectionalLight(DirectionalLight l);
    void SetDirectionalLight(glm::vec3 direction, glm::vec3 color, f32 intensity);
    void AddRenderable(std::shared_ptr<Renderable> r);
    void AddLight(Light l);
    void Clear();

    const std::vector<std::shared_ptr<Renderable>>& Renderables() const { return m_renderables; }
    const std::vector<Mesh::DrawCall>& OpaqueRenderables() const { return m_opaqueRenderables; }
    const std::vector<Mesh::DrawCall>& TransparentRenderables() const
    {
        return m_transparentRenderables;
    }

    DirectionalLight&              GetDirectionalLight() { return m_directionalLight; }
    const u32                      LightsCount() const { return m_lightCount; }
    std::array<Light, MAX_LIGHTS>& Lights() { return m_lights; }

  private:
    std::vector<Mesh::DrawCall>              m_opaqueRenderables;
    std::vector<Mesh::DrawCall>              m_transparentRenderables;
    std::vector<std::shared_ptr<Renderable>> m_renderables;
    DirectionalLight                         m_directionalLight;
    std::array<Light, MAX_LIGHTS>            m_lights;
    u32                                      m_lightCount{0};
};
} // namespace Pinut
