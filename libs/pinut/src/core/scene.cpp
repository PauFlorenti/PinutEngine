#include "stdafx.h"

#include "scene.h"
#include "src/renderer/materials/material.h"
#include "src/renderer/renderable.h"

namespace Pinut
{
void Scene::LoadScene() {}

void Scene::SetDirectionalLight(DirectionalLight l) { m_directionalLight = std::move(l); }

void Scene::SetDirectionalLight(glm::vec3 direction, glm::vec3 color, f32 intensity)
{
    m_directionalLight = {direction, intensity, color};
}

void Scene::AddLight(Light l)
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
    m_lightCount = 0;
    memset(m_lights.data(), 0, sizeof(Light) * m_lights.size());
    m_renderables.clear();
    m_opaqueRenderables.clear();
    m_transparentRenderables.clear();
}
} // namespace Pinut
