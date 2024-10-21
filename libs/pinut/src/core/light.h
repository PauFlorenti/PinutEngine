#pragma once

#include "entity.h"

namespace Pinut
{
class Camera;
class Texture;
class Light : public Entity
{
  public:
    virtual void DrawDebug(Camera* camera);

    bool      m_enabled{true};
    bool      m_castShadows{false};
    f32       m_intensity{1.0f};
    glm::vec3 m_color{glm::vec3(1.0f)};

#ifdef _DEBUG
    bool m_renderDebug{false};
#endif

  protected:
    Light()          = default;
    virtual ~Light() = default;

    std::shared_ptr<Texture> m_shadowMap{nullptr};
};

class DirectionalLight final : public Light
{
  public:
    ~DirectionalLight() = default;
    void DrawDebug(Camera* camera) override;
};

class PointLight : public Light
{
  public:
    ~PointLight() = default;
    void DrawDebug(Camera* camera) override;

    f32 m_radius{1.0f};
};

class SpotLight final : public PointLight
{
  public:
    ~SpotLight() = default;
    void DrawDebug(Camera* camera) override;

    f32 m_innerCone{15.0f};
    f32 m_outerCone{30.0f};
    f32 m_cosineExponent{10.0f};
};
} // namespace Pinut
