#pragma once

namespace Pinut
{
class Camera
{
  public:
    Camera() = default;

    void LookAt(const glm::vec3& eye, const glm::vec3& target);
    void SetProjection(float fov_rad, float width, float height, float near, float far);
    void SetProjection(float fov_rad, float aspectRadio, float near, float far);

    glm::mat4 View() const { return m_view; }
    glm::mat4 Projection() const { return m_projection; }
    glm::vec3 Position() const { return m_position; }

    const glm::vec3& Forward() const { return m_forward; }
    const glm::vec3& Right() const { return m_right; }

    void UpdateCamera(float yaw, float pitch, float x, float y, float distance);
    void UpdateCameraWASD(const glm::vec3& direction);

    float speed{1.0f};

  private:
    glm::mat4 m_view = glm::mat4(1.0f);
    glm::mat4 m_projection;
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_forward  = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 m_right    = glm::vec3(1.0f, 0.0f, 0.0f);
    /*float fov;
        float near;
        float far;*/

    // float yaw{0.0f};
    // float pitch{0.0f};
    // float roll{0.0f};
};
} // namespace Pinut
