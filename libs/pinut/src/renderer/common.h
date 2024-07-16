#pragma once

#include <glm/glm.hpp>

namespace Pinut
{

constexpr int MAX_ENTITIES = 1000;
struct PerFrameData
{
    glm::mat4 view{};
    glm::mat4 projection{};
    glm::vec3 cameraPosition{};
};
} // namespace Pinut
