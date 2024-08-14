#pragma once

namespace Pinut
{

struct DirectionalLight;
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
    u32   lightsCount{0};
    u32   dummy0;
    u32   dummy1;
    u32   dummy2;
    Light lights[MAX_LIGHTS];
    DirectionalLight directionalLight;
};
} // namespace Pinut
