#include "stdafx.h"

#include "camera.h"

namespace Pinut
{
void Camera::LookAt(const glm::vec3& eye, const glm::vec3& target)
{
    static const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    m_position = eye;
    m_view     = glm::lookAt(eye, target, glm::vec3(0.0f, 1.0f, 0.0f));
    m_forward  = glm::normalize(target - eye);
    m_right    = glm::normalize(glm::cross(up, m_forward));
}

void Camera::SetProjection(float fov_rad, float width, float height, float near, float far)
{
    SetProjection(fov_rad, width / height, near, far);
}

void Camera::SetProjection(float fov_rad, float aspectRadio, float near, float far)
{
    m_projection = glm::perspective(fov_rad, aspectRadio, near, far);
}

void Camera::UpdateCamera(float yaw, float pitch, float x, float y, float distance) {}

void Camera::UpdateCameraWASD(const glm::vec3& direction)
{
    m_position += direction * speed;
    LookAt(m_position, m_forward);
}
} // namespace Pinut
