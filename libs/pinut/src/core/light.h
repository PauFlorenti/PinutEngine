#pragma once

namespace Pinut
{
#define MAX_LIGHTS 10

class Texture;
class Light
{
  public:
    Light()          = default;
    virtual ~Light() = default;

    glm::vec3 GetPosition() const;
    glm::quat GetRotation() const;
    glm::vec3 GetDirection() const;
    glm::mat4 GetTransform() const;

    void Position(glm::vec3 position); // Adds position to the previous one.
    void Rotation(glm::quat rotation); // Adds rotation to the previous one;

    void SetPosition(glm::vec3 position); // Sets position.
    void SetRotation(glm::quat quaterion); // Sets rotation.
    void SetTransform(glm::mat4 transform);

    virtual void DrawDebug() = 0;

    bool      m_enabled{true};
    bool      m_castShadows{false};
    f32       m_intensity{1.0f};
    glm::vec3 m_color{glm::vec3(1.0f)};

  protected:
    glm::mat4                m_transform{glm::mat4(1.0f)};
    std::shared_ptr<Texture> m_shadowMap{nullptr};
};

class DirectionalLight final : public Light
{
  public:
    void DrawDebug() override;
};

class PointLight : public Light
{
  public:
    void DrawDebug() override;

    f32 m_radius{1.0f};
};

class SpotLight final : public PointLight
{
  public:
    void DrawDebug() override;

    f32 m_innerCone{15.0f};
    f32 m_outerCone{30.0f};
    f32 m_cosineExponent{10.0f};
};
} // namespace Pinut
