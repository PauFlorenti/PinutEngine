#pragma once

namespace Pinut
{
#define MAX_LIGHTS 10
struct Light
{
    glm::vec3 color{};
    f32       intensity = 1.0f;
    glm::vec3 position{};
    f32       radius{1.0f};

    void DrawImGUI();
};
} // namespace Pinut
