#pragma once

#include "src/core/light.h"

namespace Pinut
{
class Renderable;
class Scene
{
  public:
    void LoadScene();
    void AddRenderable(Renderable* r);
    void AddLight(Light l);

    const std::vector<Renderable*>& Renderables() const { return m_renderables; }
    const Light*                    Lights() const { return m_lights; }
    const u32                       LightsCount() const { return m_lightCount; }

  private:
    std::vector<Renderable*> m_renderables;
    Light                    m_lights[MAX_LIGHTS];
    u32                      m_lightCount{0};
};
} // namespace Pinut
