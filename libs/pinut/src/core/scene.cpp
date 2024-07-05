#include "stdafx.h"

#include "scene.h"
#include "src/renderer/renderable.h"

namespace Pinut
{
void Scene::LoadScene() {}

void Scene::AddLight(Light l)
{
    assert(m_lightCount < MAX_LIGHTS);
    m_lights[m_lightCount++] = std::move(l);
}

void Scene::AddRenderable(Renderable* r)
{
    assert(r);
    m_renderables.push_back(r);
}
} // namespace Pinut
