#pragma once

#include <array>

#include "src/core/light.h"

namespace Pinut
{
class Renderable;
class Scene
{
  public:
    void LoadScene();
    void AddRenderable(std::shared_ptr<Renderable> r);
    void AddLight(Light l);
    void Clear();

    const std::vector<std::shared_ptr<Renderable>>& Renderables() const { return m_renderables; }
    const std::vector<std::shared_ptr<Renderable>>& OpaqueRenderables() const
    {
        return m_opaqueRenderables;
    }
    const std::vector<std::shared_ptr<Renderable>>& TransparentRenderables() const
    {
        return m_transparentRenderables;
    }
    const u32                      LightsCount() const { return m_lightCount; }
    std::array<Light, MAX_LIGHTS>& Lights() { return m_lights; }

  private:
    std::vector<std::shared_ptr<Renderable>> m_opaqueRenderables;
    std::vector<std::shared_ptr<Renderable>> m_transparentRenderables;
    std::vector<std::shared_ptr<Renderable>> m_renderables;
    std::array<Light, MAX_LIGHTS>            m_lights;
    u32                                      m_lightCount{0};
};
} // namespace Pinut
