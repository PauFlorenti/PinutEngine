#include "stdafx.h"

#include "src/core/renderable.h"
#include "src/core/scene.h"

namespace Pinut
{
void Scene::LoadScene() {}

void Scene::SetDirectionalLight(DirectionalLight l) { m_directionalLight = std::move(l); }

void Scene::AddLight(std::shared_ptr<Light> l)
{
    assert(m_lightCount < MAX_LIGHTS);
    m_lights[m_lightCount++] = std::move(l);
}

void Scene::AddRenderable(std::shared_ptr<Renderable> r)
{
    assert(r);
    m_renderables.push_back(r);
}

void Scene::Clear()
{
    for (auto& l : m_lights)
        l.reset();

    m_lights.fill(nullptr);
    m_lightCount = 0;
    m_renderables.clear();
    m_opaqueRenderables.clear();
    m_transparentRenderables.clear();
}
} // namespace Pinut
