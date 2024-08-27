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
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    f32       cosine{0.f}; // If greater than 0, then this is a spotlight.
    f32       cosineExponent{0.f};

    void DrawImGUI();
};

struct DirectionalLight
{
    glm::quat rotation{glm::pi<float>() / 4,
                       0.0f,
                       0.0f,
                       glm::pi<float>() / 4}; // No rotation whatsoever by default.
    f32       intensity = 1.0f;
    glm::vec3 color{};
};
} // namespace Pinut
