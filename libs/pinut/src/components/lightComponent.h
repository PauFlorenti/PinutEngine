#pragma once

namespace Pinut
{
namespace Component
{
struct LightComponent
{
    bool      m_enabled{true};
    bool      m_castShadows{false};
    f32       m_intensity{1.0f};
    glm::vec3 m_color{glm::vec3(1.0f)};
    f32       m_radius{1.0f};
    f32       m_innerCone{0.0f};
    f32       m_outerCone{0.0f};
    f32       m_cosineExponent{0.0f};
};
} // namespace Component
} // namespace Pinut
