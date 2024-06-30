#pragma once

#include <glm/glm.hpp>

namespace Pinut
{
struct PerFrameData
{
    glm::mat4 view{};
    glm::mat4 projection{};
};
} // namespace Pinut
