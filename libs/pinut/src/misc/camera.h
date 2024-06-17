#pragma once

namespace Pinut
{
    class Camera
    {
    public:
        Camera() = default;

        void LookAt(const glm::vec3 &eye, const glm::vec3 &target);
        void SetProjection(float fov_rad, float width, float height, float near, float far);
        void SetProjection(float fov_rad, float aspectRadio, float near, float far);

        glm::mat4 GetView() const { return view; }
        glm::mat4 GetProjection() const { return projection; }
        glm::vec3 GetPosition() const { return position; }

        const glm::vec3 &GetForward() const { return forward; }
        const glm::vec3 &GetRight() const { return right; }

        void UpdateCamera(float yaw, float pitch, float x, float y, float distance);
        void UpdateCameraWASD(const glm::vec3 &direction);

        float speed{1.0f};

    private:
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection;
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
        /*float fov;
        float near;
        float far;*/

        // float yaw{0.0f};
        // float pitch{0.0f};
        // float roll{0.0f};
    };
}
