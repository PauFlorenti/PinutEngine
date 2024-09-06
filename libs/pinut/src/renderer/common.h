#pragma once

namespace Pinut
{
struct Light;

constexpr int MAX_ENTITIES = 1000;
struct PerFrameData
{
    glm::mat4 view{};
    glm::mat4 projection{};
    glm::vec3 cameraPosition{};
};

struct LightData
{
    glm::vec3 color{};
    f32       intensity = 1.0f;
    glm::vec3 position{};
    f32       radius{1.0f};
    glm::vec3 direction{};
    f32       innerCone{0.f}; // If greater than 0, then this is a spotlight.
    f32       outerCone{0.f}; // If greater than 0, then this is a spotlight.
    f32       cosineExponent{0.f};
    f32       dummy1;
    f32       dummy2;
};

struct DirectionalLightData
{
    glm::vec3 direction{};
    f32       intensity = 1.0f;
    glm::vec3 color{};
    f32       dummy1;
};

struct SceneLightData
{
    u32                  lightsCount{0};
    u32                  dummy0;
    u32                  dummy1;
    u32                  dummy2;
    LightData            lights[MAX_LIGHTS];
    DirectionalLightData directionalLight;
};
} // namespace Pinut
