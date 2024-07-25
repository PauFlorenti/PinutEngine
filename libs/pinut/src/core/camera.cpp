#include "stdafx.h"

#include <imgui.h>

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

void Camera::UpdateRotation(const f32 deltaTime, const f32 x, const f32 y)
{
    auto       yaw   = glm::degrees(atan2f(m_forward.x, m_forward.z));
    const auto modul = sqrtf(m_forward.x * m_forward.x + m_forward.z * m_forward.z);
    auto       pitch = glm::degrees(atan2f(-m_forward.y, modul));

    yaw += x * deltaTime * m_sensibility;
    pitch -= y * deltaTime * m_sensibility;

    if (pitch > 89.9f)
    {
        pitch = 89.9f;
    }
    if (pitch < -89.9f)
    {
        pitch = -89.9f;
    }

    const auto newForward =
      glm::normalize(glm::vec3(sinf(glm::radians(yaw)) * cosf(glm::radians(-pitch)),
                               sinf(glm::radians(-pitch)),
                               cosf(glm::radians(yaw)) * cosf(glm::radians(-pitch))));
    LookAt(m_position, m_position + newForward);
}

void Camera::UpdateCamera(float yaw, float pitch, float x, float y, float distance) {}

void Camera::UpdateCameraWASD(const glm::vec3& direction)
{
    m_position += direction * speed;
    LookAt(m_position, m_position + m_forward);
}

void Camera::DrawImGUI()
{
    ImGui::DragFloat("Speed", &speed);
    ImGui::DragFloat("Sensibility", &m_sensibility);
}
} // namespace Pinut
