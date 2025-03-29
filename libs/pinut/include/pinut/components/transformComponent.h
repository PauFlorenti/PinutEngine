#pragma once

#include "pinut/components/baseComponent.h"

namespace Pinut
{
namespace Component
{
struct TransformComponent : public BaseComponent
{
    TransformComponent(const glm::mat4& transform);
    TransformComponent(const glm::vec3& inPosition = glm::vec3(0.0f),
                       const glm::vec3& inRotation = glm::vec3(0.0f),
                       const glm::vec3& inScale    = glm::vec3(1.0f));

    glm::mat4 GetTransform() const;
    glm::vec3 GetPosition() const;
    glm::quat GetRotation() const;
    glm::vec3 GetScale() const;
    glm::vec3 GetForward() const;

    void Transform(const glm::mat4& transform);
    void Translate(const glm::vec3& position);
    void Rotate(const glm::quat& rotation);
    void Scale(const glm::vec3& scale);

    void SetTransform(const glm::mat4& transform);
    void SetPosition(const glm::vec3& position);
    void SetRotation(const glm::quat& quaterion);
    void SetScale(const glm::vec3& scale);

#ifdef _DEBUG
    void RenderDebug(const ShaderCameraData& camera);
#endif

    glm::mat4 m_transform;

  private:
    entt::entity              m_parent{entt::null};
    std::vector<entt::entity> m_childs;
};
} // namespace Component
} // namespace Pinut
