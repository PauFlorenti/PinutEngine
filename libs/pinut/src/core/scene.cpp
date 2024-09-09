#include "stdafx.h"

#include "scene.h"
#include "src/renderer/materials/material.h"
#include "src/renderer/renderable.h"

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

    for (auto& dc : r->GetMesh()->DrawCalls())
    {
        dc.m_owner = r;
        switch (dc.m_material->Type())
        {
            case MaterialType::OPAQUE:
                m_opaqueRenderables.push_back(dc);
                break;
            case MaterialType::TRANSPARENT:
                m_transparentRenderables.push_back(dc);
                break;
            default:
                break;
        }
    }
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
