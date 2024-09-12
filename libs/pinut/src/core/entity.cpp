#include "stdafx.h"

#include "entity.h"

namespace Pinut
{
constexpr glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 Entity::GetPosition() const { return m_transform[3]; }
glm::quat Entity::GetRotation() const { return glm::toQuat(m_transform); }
glm::vec3 Entity::GetScale() const
{
    glm::vec3 scale;
    scale.x = glm::length(glm::vec3(m_transform[0])); // Length of the first column
    scale.y = glm::length(glm::vec3(m_transform[1])); // Length of the second column
    scale.z = glm::length(glm::vec3(m_transform[2])); // Length of the third column
    return scale;
}
glm::vec3 Entity::GetDirection() const { return GetRotation() * forward; }
glm::mat4 Entity::GetTransform() const { return m_transform; }

void Entity::Transform(const glm::mat4& transform) { m_transform *= transform; }
void Entity::Position(const glm::vec3& position)
{
    m_transform = glm::translate(m_transform, position);
}
void Entity::Rotation(const glm::quat& rotation) { m_transform *= glm::toMat4(rotation); }
void Entity::Scale(const glm::vec3& scale) { glm::scale(m_transform, scale); }

void Entity::SetTransform(const glm::mat4& transform) { m_transform = transform; }

void Entity::SetPosition(const glm::vec3& position)
{
    const auto rotation       = glm::toMat4(GetRotation());
    const auto scale          = glm::scale(glm::mat4(1), GetScale());
    const auto positionMatrix = glm::translate(glm::mat4(1), position);
    m_transform               = positionMatrix * rotation * scale;
}

void Entity::SetRotation(const glm::quat& quaterion)
{
    const auto rotation = glm::toMat4(quaterion);
    const auto scale    = glm::scale(glm::mat4(1), GetScale());
    const auto position = glm::translate(glm::mat4(1), GetPosition());
    m_transform         = position * rotation * scale;
}

void Entity::SetScale(const glm::vec3& scale)
{
    const auto rotation    = glm::toMat4(GetRotation());
    const auto scaleMatrix = glm::scale(glm::mat4(1), scale);
    const auto position    = glm::translate(glm::mat4(1), GetPosition());
    m_transform            = position * rotation * scaleMatrix;
}

} // namespace Pinut
