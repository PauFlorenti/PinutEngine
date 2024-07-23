#include "stdafx.h"

#include "scene.h"
#include "src/renderer/materials/material.h"
#include "src/renderer/renderable.h"

namespace Pinut
{
void Scene::LoadScene() {}

void Scene::AddLight(Light l)
{
    assert(m_lightCount < MAX_LIGHTS);
    m_lights[m_lightCount++] = std::move(l);
}

void Scene::AddRenderable(std::shared_ptr<Renderable> r)
{
    assert(r);

    switch (r->Material()->Type())
    {
        case MaterialType::OPAQUE:
            m_opaqueRenderables.push_back(r);
            break;
        case MaterialType::TRANSPARENT:
            m_transparentRenderables.push_back(r);
            break;
    }
}

void Scene::Clear()
{
    m_lightCount = 0;
    memset(m_lights.data(), 0, sizeof(Light) * m_lights.size());
    m_opaqueRenderables.clear();
    m_transparentRenderables.clear();
}
} // namespace Pinut
