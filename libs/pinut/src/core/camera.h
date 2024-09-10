#pragma once

namespace Pinut
{
class Camera
{
  public:
    Camera() = default;

    void LookAt(const glm::vec3& eye, const glm::vec3& target);
    void SetProjection(f32 fov_rad, f32 width, f32 height, f32 near, f32 far);
    void SetProjection(f32 fov_rad, f32 aspectRatio, f32 near, f32 far);
    void SetOrthographic(f32 left, f32 right, f32 bottom, f32 top);

    glm::mat4 View() const { return m_view; }
    glm::mat4 Projection() const { return m_projection; }
    glm::vec3 Position() const { return m_position; }

    const glm::vec3& Forward() const { return m_forward; }
    const glm::vec3& Right() const { return m_right; }

    bool IsOrthographic() const { return m_isOrthographic; }

    void UpdateRotation(const f32 deltaTime, const f32 x, const f32 y);
    void UpdateCamera(f32 yaw, f32 pitch, f32 x, f32 y, f32 distance);
    void UpdateCameraWASD(const glm::vec3& direction);

    void DrawDebug();

    f32 speed{5.0f};

  private:
    bool m_isOrthographic{false};

    glm::mat4 m_view = glm::mat4(1.0f);
    glm::mat4 m_projection;
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_forward  = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 m_right    = glm::vec3(1.0f, 0.0f, 0.0f);

    f32 m_sensibility = 5.0f;
    f32 m_near{0.01f};
    f32 m_far{1000.0f};

    // Perspective
    f32 m_fov = glm::pi<float>() / 4.0f;
    f32 m_aspectRatio{1.0f};

    // Orthographic
    f32 m_orthoRight{10.0f};
    f32 m_orthoLeft{-10.0f};
    f32 m_orthoBottom{-10.0f};
    f32 m_orthoTop{10.0f};
};
} // namespace Pinut
