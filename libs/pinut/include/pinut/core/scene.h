#pragma once

namespace Pinut
{
  constexpr i32 MAX_ENTITIES = 1000;
class Scene
{
  public:
    Scene();
    ~Scene();
    entt::entity    CreateEntity();
    entt::registry& Registry() { return m_sceneRegistry; }
    void            LoadScene();
    void            ClearScene();

  private:
    entt::registry            m_sceneRegistry;
    std::vector<entt::entity> m_entities;
};
} // namespace Pinut
