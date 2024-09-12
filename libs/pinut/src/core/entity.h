#pragma once

namespace Pinut
{
// Base class for all elements that have to be represented somewhere in the world.
class Entity
{
  public:
    glm::vec3 GetPosition() const;
    glm::quat GetRotation() const;
    glm::vec3 GetScale() const;
    glm::vec3 GetDirection() const;
    glm::mat4 GetTransform() const;

    void Transform(const glm::mat4& transform);
    void Position(const glm::vec3& position); // Adds position to the previous one.
    void Rotation(const glm::quat& rotation); // Adds rotation to the previous one;
    void Scale(const glm::vec3& scale);

    void SetTransform(const glm::mat4& transform);
    void SetPosition(const glm::vec3& position); // Sets position.
    void SetRotation(const glm::quat& quaterion); // Sets rotation.
    void SetScale(const glm::vec3& transform);

    virtual void DrawDebug() = 0;

  protected:
    Entity()          = default;
    virtual ~Entity() = default;

    glm::mat4 m_transform = glm::mat4(1.0f);
};
} // namespace Pinut
