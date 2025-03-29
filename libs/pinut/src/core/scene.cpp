#include "pch.hpp"

#include "pinut/components/transformComponent.h"
#include "pinut/core/scene.h"

namespace Pinut
{
Scene::Scene() { m_entities.reserve(MAX_ENTITIES); }

entt::entity Scene::CreateEntity()
{
    assert(m_entities.size() <= MAX_ENTITIES);

    auto entity = m_sceneRegistry.create();

    m_sceneRegistry.emplace<Component::TransformComponent>(entity);

    m_entities.push_back(entity);
    return entity;
}

void Scene::LoadScene() {}

void Scene::ClearScene() { m_sceneRegistry.clear(); }
} // namespace Pinut
