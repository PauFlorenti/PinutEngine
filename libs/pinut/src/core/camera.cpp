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

void Camera::SetProjection(float fov_rad, float aspectRatio, float near, float far)
{
    m_isOrthographic = false;
    m_aspectRatio    = aspectRatio;
    m_projection     = glm::perspective(fov_rad, aspectRatio, near, far);
}

void Camera::SetOrthographic(f32 left, f32 right, f32 bottom, f32 top)
{
    m_isOrthographic = true;
    m_orthoLeft      = left;
    m_orthoRight     = right;
    m_orthoBottom    = bottom;
    m_orthoTop       = top;
    m_projection     = glm::ortho(left, right, bottom, top);
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

void Camera::DrawDebug()
{
    if (ImGui::Checkbox("Orthographic", &m_isOrthographic))
    {
        if (m_isOrthographic)
            SetOrthographic(m_orthoLeft, m_orthoRight, m_orthoBottom, m_orthoTop);
        else
            SetProjection(m_fov, m_aspectRatio, m_near, m_far);
    }

    ImGui::DragFloat3("Position", &m_position[0]);
    ImGui::DragFloat("Speed", &speed);
    ImGui::DragFloat("Sensibility", &m_sensibility);

    if (m_isOrthographic)
    {
        if (ImGui::DragFloat("Right", &m_orthoRight, 1.0f, 0.0f))
            SetOrthographic(m_orthoLeft, m_orthoRight, m_orthoBottom, m_orthoTop);

        if (ImGui::DragFloat("Left", &m_orthoLeft, 1.0f, 0.0f))
            SetOrthographic(m_orthoLeft, m_orthoRight, m_orthoBottom, m_orthoTop);

        if (ImGui::DragFloat("Bottom", &m_orthoBottom, 1.0f, 0.0f))
            SetOrthographic(m_orthoLeft, m_orthoRight, m_orthoBottom, m_orthoTop);

        if (ImGui::DragFloat("Top", &m_orthoTop, 1.0f, 0.0f))
            SetOrthographic(m_orthoLeft, m_orthoRight, m_orthoBottom, m_orthoTop);
    }
    else
    {
        auto fov = glm::degrees(m_fov);
        if (ImGui::DragFloat("Fov", &fov, 1.0f, 0.0f, 90.0f, "%.0f"))
        {
            m_fov = glm::radians(fov);
            SetProjection(m_fov, m_aspectRatio, m_near, m_far);
        }

        if (ImGui::DragFloat("Near", &m_near, 1.0f, 0.0001f, m_far))
        {
            SetProjection(m_fov, m_aspectRatio, m_near, m_far);
        }

        if (ImGui::DragFloat("Far", &m_far, 1.0f, m_near, 100000.0f))
        {
            SetProjection(m_fov, m_aspectRatio, m_near, m_far);
        }
    }
    LookAt(m_position, m_position + m_forward);
}
} // namespace Pinut
